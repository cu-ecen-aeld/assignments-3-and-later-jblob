#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <syslog.h>

#define BACKLOG 20
#define PORT "9000"

#define DEBUG_OUT
#define FOUT "/var/tmp/aesdsocketdata"

int main(int argc, char *argv[])
{
	openlog(NULL, 0, LOG_USER);

	int sockfd, new_fd;
	int status;

	struct addrinfo hints;
	struct addrinfo *servinfo; // will point to the results of getaddrinfo
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	char s[INET6_ADDRSTRLEN];
	
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	// 1. Get address information
	status = getaddrinfo(NULL, PORT, &hints, &servinfo);
	if (status != 0)
	{
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}
	// servinfo now points to a linked list of 1 or more
	// struct addrinfos

	// 2. Create the socket
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockfd == -1)
	{
		freeaddrinfo(servinfo);
		return -1;
	}

#ifdef DEBUG_OUT
	printf("Socket started\n");
#endif

	int yes = 1;
	status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (status == -1) 
	{
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
		close(sockfd);
		freeaddrinfo(servinfo);
		return -1;
	}
	
	freeaddrinfo(servinfo); // free the linked-list - no longer needed from here

	// connect
	// A server waits (listen/accept). Only a client initiates a connect()
	//connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	
	// 4. Listen for incoming connections
	// listen
	status = listen(sockfd, BACKLOG);
	if (status == -1)
	{
		close(sockfd);
		return -1;
	}
	
	printf("Server: waiting for connections on port %s...\n", PORT);
	
	while(1)
	{
		// 5. Accept a connection
		// accept
		addr_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		if (new_fd == -1) 
		{
			close(sockfd);
			return -1;
		}
	
		// send/recv
		// 6. Get the printable IP address
		void *addr;
		if (their_addr.ss_family == AF_INET) // IPv4
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)&their_addr;
			addr = &(ipv4->sin_addr);
		} 
		else // IPv6
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&their_addr;
			addr = &(ipv6->sin6_addr);
		}

		inet_ntop(their_addr.ss_family, addr, s, sizeof s);
#ifdef DEBUG_OUT
		printf("Accepted connection from %s\n", s);
#endif
		syslog(LOG_DEBUG, "Accepted connection from %s", s);
		
		// write received data to file FOUT
		FILE *fout = fopen(FOUT, "a+");
		if (fout == NULL)
		{
#ifdef DEBUG_OUT
			printf("Error creating file %s\n", FOUT);
#endif
			syslog(LOG_ERR, "Error creating file %s", FOUT);
			close(new_fd);
			continue;
		}
		
		// loop while we get data
		char buf[1024]; // Buffer for incoming data
		ssize_t bytes_received;
		int ret;
		
		while ((bytes_received = recv(new_fd, buf, sizeof(buf), 0)) > 0)
		{
#ifdef DEBUG_OUT
			//printf("received %.*s\n", (int)bytes_received, buf);
#endif
			ret = fwrite(buf, 1, bytes_received, fout);
			if (ret != (size_t)bytes_received)
			{
				// writing to file failed
				syslog(LOG_ERR, "writing to file %s failed", FOUT);
				// return(-1);
				break;
			}
			
			// flush to disk if newline is found
			if (memchr(buf, '\n', bytes_received) != NULL)
			{
				// packet is complete
				fflush(fout);
			}

            // Send the full file content back to the client
			rewind(fout); // Go to the start of the file
			char send_buf[1024];
			size_t bytes_read;
                
			while ((bytes_read = fread(send_buf, 1, sizeof(send_buf), fout)) > 0) 
			{
				if (send(new_fd, send_buf, bytes_read, 0) == -1) 
				{
#ifdef DEBUG_OUT
					printf("failed sending file content back to sender");
#endif
					syslog(LOG_ERR, "failed sending file content back to sender");
					break;
				}
			}
                
			// Seek back to the end so the next append happens correctly
			fseek(fout, 0, SEEK_END);
//			fclose(fout);
		}
		
		if (bytes_received == -1)
		{
#ifdef DEBUG_OUT
			printf("error in recv\n");
#endif
		}

		// Cleanup
		close(new_fd);
		syslog(LOG_DEBUG, "Closed connection from %s", s);
	}
	
#ifdef DEBUG_OUT
	printf("server shutdown\n");
#endif
	
	close(sockfd);
	
	
	closelog();
	
	return 0;
}
