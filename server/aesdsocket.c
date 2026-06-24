#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <sys/queue.h>
#include <time.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <syslog.h>

#define BACKLOG 20
#define PORT "9000"

#define IOCTL_PREFIX "AESDCHAR_IOCSEEKTO:"

#define DEBUG_OUT
#define USE_AESD_CHAR_DEVICE 1
#if USE_AESD_CHAR_DEVICE
    #define FOUT "/dev/aesdchar"
#else
    #define FOUT "/var/tmp/aesdsocketdata"
#endif

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

    void *addr;
    char s[INET6_ADDRSTRLEN];

    if (th_arg->their_addr.ss_family == AF_INET) 
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)&th_arg->their_addr;
        addr = &(ipv4->sin_addr);
    } 
    else 
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&th_arg->their_addr;
        addr = &(ipv6->sin6_addr);
    }

    inet_ntop(th_arg->their_addr.ss_family, addr, s, sizeof s);
    syslog(LOG_DEBUG, "<AESDSOCKET>Accepted connection from %s", s);

    pthread_mutex_lock(&file_mutex);

    int fd = open(FOUT, O_RDWR);
    if (fd < 0) 
    {
        pthread_mutex_unlock(&file_mutex);
        close(th_arg->new_fd);
        return NULL;
    }

    /* -------- RECEIVE COMPLETE MESSAGE -------- */
    char buf[1024];
    char full_buf[2048] = {0};
    size_t total_len = 0;
    ssize_t bytes_received;

    while ((bytes_received = recv(th_arg->new_fd, buf, sizeof(buf), 0)) > 0) 
    {

        if (total_len + bytes_received < sizeof(full_buf)) 
        {
            memcpy(full_buf + total_len, buf, bytes_received);
            total_len += bytes_received;
        }

        if (memchr(buf, '\n', bytes_received) != NULL)
            break;
    }

	/* Ensure newline termination */
	if (total_len > 0 && full_buf[total_len - 1] != '\n') 
	{
		full_buf[total_len++] = '\n';
	}


/* -------- PARSE IOCTL OR NORMAL WRITE -------- */
    if (strncmp(full_buf, IOCTL_PREFIX, strlen(IOCTL_PREFIX)) == 0) 
    {
        uint32_t cmd_idx, cmd_offset;

        if (sscanf(full_buf + strlen(IOCTL_PREFIX), "%u,%u", &cmd_idx, &cmd_offset) == 2) 
        {
            struct aesd_seekto seekto;
            seekto.write_cmd = cmd_idx;
            seekto.write_cmd_offset = cmd_offset;

            if (ioctl(fd, AESDCHAR_IOCSEEKTO, &seekto) != 0) 
            {
                syslog(LOG_ERR, "<AESDSOCKET>ioctl failed");
            }
        }
        /* HIER KEIN lseek! Wir wollen genau ab der ioctl-Position lesen. */
    } 
    else 
    {
        /* -------- NORMAL WRITE (robust) -------- */
        size_t written_total = 0;

        while (written_total < total_len) 
        {
            ssize_t written = write(fd, full_buf + written_total, total_len - written_total);
            if (written < 0) break;
            written_total += written;
        }
        
        fsync(fd);

        /* Das funktioniert für BEIDE Varianten (Device und Datei) */
        /* da dein Treiber llseek implementiert hat! */
        lseek(fd, 0, SEEK_SET);
    }

    /* Das globale lseek(fd, 0, SEEK_SET); an dieser Stelle UNBEDINGT ENTFERNEN! */

    /* -------- READ BACK AND SEND -------- */
    char send_buf[1024];
	ssize_t bytes_read;

    while ((bytes_read = read(fd, send_buf, sizeof(send_buf))) > 0) 
    {
        size_t sent_total = 0;

        while (sent_total < bytes_read) 
        {
            ssize_t sent = send(th_arg->new_fd, send_buf + sent_total, bytes_read - sent_total, 0);
            if (sent < 0) 
				break;
            sent_total += sent;
        }
    }

    close(fd);
    pthread_mutex_unlock(&file_mutex);

    /* -------- CLEANUP -------- */
    close(th_arg->new_fd);

    pthread_mutex_lock(&file_mutex);
    th_arg->bThreadCompleted = true;
    pthread_mutex_unlock(&file_mutex);

    return NULL;
}

void* timer_thread(void* arg) 
{
	while (!caught_sig)
	{
		for (int i=0; i<10 && !caught_sig; i++)
		{
			sleep(1);
		}

		if (caught_sig)
			break;
	
		time_t rawtime;
		struct tm *info;
		char buffer[80];

		time(&rawtime);
		info = localtime(&rawtime);
		strftime(buffer, sizeof(buffer), "timestamp: %Y/%m/%d/%H/%M/%S\n", info);

		pthread_mutex_lock(&file_mutex);
		FILE *fout = fopen(FOUT, "a+");
		if (fout == NULL) 
		{
#ifdef DEBUG_OUT
			syslog(LOG_ERR, "<AESDSOCKET><TIMERTHREAD>writing to file %s failed", FOUT);
#endif
			pthread_mutex_unlock(&file_mutex);
			continue; // dont return so that the timerthread gets called again in 10 secs
		}
		fputs(buffer, fout);
		fclose(fout);
	
		pthread_mutex_unlock(&file_mutex);
	}

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
	signal(SIGPIPE, SIG_IGN);

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
	
#if !USE_AESD_CHAR_DEVICE
	pthread_t thread_id_timer;
	int err_timer;
	FILE *f = fopen(FOUT, "w"); 
	if (f != NULL) 
	{
		fclose(f);
	} 
	else 
	{
		syslog(LOG_ERR, "<AESDSOCKET> Could not initialize file %s", FOUT);
	}
#endif

	// 1. Get address information
	status = getaddrinfo(NULL, PORT, &hints, &servinfo);
	if (status != 0)
	{
		// fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		syslog(LOG_ERR, "<AESDSOCKET>error in getaddrinfo %s", gai_strerror(status));
		return -1;
	}
	// servinfo now points to a linked list of 1 or more "struct addrinfos"

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

#if !USE_AESD_CHAR_DEVICE
	bool bTimerStarted = false;
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
		
#if !USE_AESD_CHAR_DEVICE
		if (bTimerStarted == false)
		{
			err_timer = pthread_create(&thread_id_timer, NULL, timer_thread, NULL);
			if (err_timer != 0)
			{
				// TODO: errorhandling
				syslog(LOG_ERR, "<AESDSOCKET>error in pthread_create, retval = %d", err_timer);
			}
			bTimerStarted = true;
		}
#endif		
		datap = SLIST_FIRST(&head);
		while (datap != NULL) 
		{
			// 1. Save the pointer to the NEXT element BEFORE we potentially free 'datap'
			tmp_datap = SLIST_NEXT(datap, entries); 

			if (datap->bThreadCompleted) 
			{
				// 2. Join the thread to prevent zombies
				pthread_join(datap->thread_id, NULL);

				// 3. Remove from list and free memory
				SLIST_REMOVE(&head, datap, thread_data, entries);
				free(datap);
			}
			
			// 4. Move to the next element using our saved pointer
			datap = tmp_datap; 
		}
	}

	syslog(LOG_DEBUG, "<AESDSOCKET>Caught signal, exiting");
	
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

#if !USE_AESD_CHAR_DEVICE
	if (bTimerStarted)
	{
		pthread_join(thread_id_timer, NULL);
	}
#endif
	
	close(sockfd);
	closelog();
	
#if !USE_AESD_CHAR_DEVICE
	status = remove(FOUT);
	if( status != 0 )
	{
		syslog(LOG_ERR, "<AESDSOCKET>error deleting file %s\n", FOUT);
	}
#endif

	return 0;
}
