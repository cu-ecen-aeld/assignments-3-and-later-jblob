#include <stdio.h>
#include <stdlib.h>

#include <syslog.h>

int main(int argc, char *argv[])
{
	openlog(NULL, 0, LOG_USER);
	
	syslog(LOG_DEBUG, "Socket started\n");

	closelog();
	
	return 0;
}
