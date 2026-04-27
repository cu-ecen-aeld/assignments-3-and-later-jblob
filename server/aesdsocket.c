#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <sys/queue.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <syslog.h>

#define BACKLOG 20
#define PORT "9000"

#define DEBUG_OUT
#define FOUT "/var/tmp/aesdsocketdata"

volatile sig_atomic_t caught_sig  = false;

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
struct thread_data
{
	struct sockaddr_storage their_addr;
	int new_fd;
	bool bThreadCompleted;
	pthread_t thread_id;
	SLIST_ENTRY(thread_data) entries; // pointer to the next element
};

static void signal_handler(int signal_number)
{
	if ((signal_number == SIGINT) || (signal_number == SIGTERM))
	{
		caught_sig = true;
	}
}

void *threadfunc(void *arg) 
{
	struct thread_data *th_arg = (struct thread_data *)arg;

	// send/recv
	// 6. Get the printable IP address
	void *addr;
	char s[INET6_ADDRSTRLEN];

	if (th_arg->their_addr.ss_family == AF_INET) // IPv4
	{
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)&th_arg->their_addr;
		addr = &(ipv4->sin_addr);
	} 
	else // IPv6
	{
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&th_arg->their_addr;
		addr = &(ipv6->sin6_addr);
	}

	inet_ntop(th_arg->their_addr.ss_family, addr, s, sizeof s);

	syslog(LOG_DEBUG, "<AESDSOCKET>Accepted connection from %s", s);
		
	// --- START KRITISCHER ABSCHNITT ---
	pthread_mutex_lock(&file_mutex);
	
	// write received data to file FOUT
	FILE *fout = fopen(FOUT, "a+");
	if (fout == NULL)
	{
		syslog(LOG_ERR, "<AESDSOCKET>Error creating file %s", FOUT);
		close(th_arg->new_fd);
//		continue; // TODO: break or exit here ?
		th_arg->bThreadCompleted = true;
		pthread_exit(NULL);
	}
		
	// loop while we get data
	char buf[1024]; // Buffer for incoming data
	ssize_t bytes_received;
	int ret;
		
	while ((bytes_received = recv(th_arg->new_fd, buf, sizeof(buf), 0)) > 0)
	{
		ret = fwrite(buf, 1, bytes_received, fout);
		if (ret != (size_t)bytes_received)
		{
			// writing to file failed
			syslog(LOG_ERR, "<AESDSOCKET>writing to file %s failed", FOUT);
			// return(-1);
			break;
		}
		
		// flush to disk if newline is found
		if (memchr(buf, '\n', bytes_received) != NULL)
		{
			// packet is complete
			fflush(fout);

			// Send the full file content back to the client
			rewind(fout); // Go to the start of the file
			char send_buf[1024];
			size_t bytes_read;
				
			while ((bytes_read = fread(send_buf, 1, sizeof(send_buf), fout)) > 0) 
			{
				if (send(th_arg->new_fd, send_buf, bytes_read, 0) == -1) 
				{
					syslog(LOG_ERR, "<AESDSOCKET>failed sending file content back to sender");
					break;
				}
			}
				
			// Seek back to the end so the next append happens correctly
			fseek(fout, 0, SEEK_END);
		}
	}
		
	if (bytes_received == -1)
	{
#ifdef DEBUG_OUT
		syslog(LOG_ERR, "<AESDSOCKET>error in recv");
#endif
	}

	// Cleanup
	fclose(fout);
	pthread_mutex_unlock(&file_mutex);
	// --- ENDE KRITISCHER ABSCHNITT ---
	
	close(th_arg->new_fd);
	syslog(LOG_DEBUG, "<AESDSOCKET>Closed connection from %s", s);
	
	th_arg->bThreadCompleted = true;
	
	pthread_exit(NULL);
}

void* timer_thread(void* arg) 
{
	if  (caught_sig)
	{
		return NULL;
	}
	
	sleep(10);

	time_t rawtime;
	struct tm *info;
	char buffer[80];

	time(&rawtime);
	info = localtime(&rawtime);
	strftime(buffer, sizeof(buffer), "timestamp:%Y/%m/%d/%H/%M/%S\n", info);

	pthread_mutex_lock(&file_mutex);
	FILE *fout = fopen(FOUT, "a+");
	if (fout == NULL) 
	{
#ifdef DEBUG_OUT
		syslog(LOG_ERR, "<AESDSOCKET><TIMERTHREAD>writing to file %s failed", FOUT);
#endif
		return NULL;
	}
	fputs(buffer, fout);
	fclose(fout);
	
	pthread_mutex_unlock(&file_mutex);

    return NULL;
}

int main(int argc, char *argv[])
{
	bool run_as_daemon = false;
	if (argc > 1)
	{
		if (strcmp(argv[1], "-d") == 0)
		{
			run_as_daemon = true;
		}
	}

	openlog(NULL, 0, LOG_USER);
	
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = signal_handler;

	if (sigaction(SIGINT, &sa, NULL) != 0) 
	{
		syslog(LOG_ERR, "<AESDSOCKET>error in sigaction SIGINT");
		return -1;
	}
	if (sigaction(SIGTERM, &sa, NULL) != 0) 
	{
		syslog(LOG_ERR, "<AESDSOCKET>error in sigaction SIGTERM");
		return -1;
	}

	int sockfd, new_fd;
	int status;
	struct sockaddr_storage their_addr;
	struct thread_data *th_data;

	struct addrinfo hints;
	struct addrinfo *servinfo; // will point to the results of getaddrinfo
	socklen_t addr_size;
	
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;	 // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;	 // fill in my IP for me

	// head of the list
	SLIST_HEAD(slisthead, thread_data);
	struct slisthead head;
	SLIST_INIT(&head); // Initialize the head
	struct thread_data *datap, *tmp_datap; // Declare iterators for SLIST_FOREACH_SAFE
	
	pthread_t thread_id_timer;
	int err_timer;
	err_timer = pthread_create(&thread_id_timer, NULL, timer_thread, NULL);
	if (err_timer != 0)
	{
		// TODO: errorhandling
		syslog(LOG_ERR, "<AESDSOCKET>error in pthread_create, retval = %d", err_timer);
	}

	// 1. Get address information
	status = getaddrinfo(NULL, PORT, &hints, &servinfo);
	if (status != 0)
	{
		// fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		syslog(LOG_ERR, "<AESDSOCKET>error in getaddrinfo %s", gai_strerror(status));
		return -1;
	}
	// servinfo now points to a linked list of 1 or more
	// struct addrinfos

	// 2. Create the socket
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockfd == -1)
	{
		syslog(LOG_ERR, "<AESDSOCKET>error in socket creation");
		freeaddrinfo(servinfo);
		return -1;
	}

#ifdef DEBUG_OUT
	syslog(LOG_INFO, "<AESDSOCKET>Socket started");
#endif

	int yes = 1;
	status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (status == -1) 
	{
		syslog(LOG_ERR, "<AESDSOCKET>error in setsockopt");
		close(sockfd);
		freeaddrinfo(servinfo);
		return -1;
	}
	
	// 3. Bind to the port (Essential for servers!)
	// bind
	// must call bind() (the kernel only auto-binds for clients)
	status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	if (status == -1)
	{
		syslog(LOG_ERR, "<AESDSOCKET>error in bind");
		close(sockfd);
		freeaddrinfo(servinfo);
		return -1;
	}
	
	freeaddrinfo(servinfo); // free the linked-list - no longer needed from here

	// connect
	// A server waits (listen/accept). Only a client initiates a connect()
	//connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);

	if (run_as_daemon)
	{
#ifdef DEBUG_OUT
		printf("Programm now running in background\n");
#endif
		// daemon(noch_im_verzeichnis_bleiben, ausgabe_behalten)
		// 0, 0 bedeutet: wechsle nach "/" und leite stdout/stderr nach /dev/null um
		if (daemon(0, 0) == -1) 
		{
			syslog(LOG_ERR, "<AESDSOCKET>error starting aesdsocket daemon");
			return 1;
		}
	}
	
	// 4. Listen for incoming connections
	// listen
	status = listen(sockfd, BACKLOG);
	if (status == -1)
	{
		syslog(LOG_ERR, "<AESDSOCKET>error in listen");
		close(sockfd);
		return -1;
	}

#ifdef DEBUG_OUT	
	syslog(LOG_INFO, "<AESDSOCKET>Server: waiting for connections on port %s", PORT);
#endif
	
	while(!caught_sig)
	{
		// 5. Accept a connection
		// accept
		addr_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		if (new_fd == -1) 
		{
			if (caught_sig) 
			{
				break; // Exit the while loop normally
			}
			// close(sockfd); // dont close socket here - we afterwards call continue and accept further connections !
			continue;
		}
	
		int err;
		// pthread_t thread_id; // is passed as 1st arg to pthread_create and filled by it
		th_data = malloc(sizeof(struct thread_data));
		if (th_data == NULL)
		{
			syslog(LOG_ERR, "<AESDSOCKET>error in malloc - exiting");
			close(new_fd);
			continue;
		}
		th_data->their_addr = their_addr;
		th_data->new_fd = new_fd;
		th_data->bThreadCompleted = false;
		err = pthread_create(&(th_data->thread_id), NULL, threadfunc, (void*)th_data);
		if (err != 0)
		{
			// TODO: errorhandling
			syslog(LOG_ERR, "<AESDSOCKET>error in pthread_create, retval = %d", err);
		}
		// add the thread to the linked list:
		SLIST_INSERT_HEAD(&head, th_data, entries);
		
		datap = SLIST_FIRST(&head);
		while(datap != NULL)
		{
			tmp_datap = SLIST_NEXT(datap, entries);
			if (datap->bThreadCompleted) 
			{
				pthread_join(datap->thread_id, NULL);
				SLIST_REMOVE(&head, datap, thread_data, entries);
				free(datap);
			}
			datap = tmp_datap;
		}
	}

	syslog(LOG_DEBUG, "<AESDSOCKET>Caught signal, exiting");
	status = remove(FOUT);
	if( status != 0 )
	{
		syslog(LOG_ERR, "<AESDSOCKET>error deleting file %s\n", FOUT);
	}
	
#ifdef DEBUG_OUT
	syslog(LOG_INFO, "<AESDSOCKET>server shutdown");
#endif
	
	// close all remaining threads
	datap = SLIST_FIRST(&head);
	while(datap != NULL)
	{
		tmp_datap = SLIST_NEXT(datap, entries);
		pthread_join(datap->thread_id, NULL);
		SLIST_REMOVE(&head, datap, thread_data, entries);
		free(datap);
		datap = tmp_datap;
	}

	pthread_join(thread_id_timer, NULL);
	
	close(sockfd);
	closelog();
	
	return 0;
}
