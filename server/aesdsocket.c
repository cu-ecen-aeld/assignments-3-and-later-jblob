#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <syslog.h>

#define BACKLOG 20
#define PORT "9000"


int main(int argc, char *argv[])
{
	openlog(NULL, 0, LOG_USER);
/*
if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
	fprintf(stderr, "gai error: %s\n", gai_strerror(status));
	exit(1);
}


// ... do everything until you don't need servinfo anymore ....

*/	
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
//	getaddrinfo("www.google.de", PORT, &hints, &res);
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

	// printf("sockfd=%d\n", sockfd); // to avoid compiler error due to -Werror
	
	// syslog(LOG_DEBUG, "Socket started\n");
	
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
	printf("Accepted connection from %s\n", s);

	// Cleanup
	close(new_fd);
	close(sockfd);
	
	
	closelog();
	
	return 0;
}


/*
 b. Opens a stream socket bound to port 9000, failing and returning -1 if any of the socket connection steps fail.
 c. Listens for and accepts a connection
 d. Logs message to the syslog “Accepted connection from xxx” where XXXX is the IP address of the connected client. 
 */
