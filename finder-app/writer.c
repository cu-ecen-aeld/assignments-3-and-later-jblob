#include <stdio.h>
#include <stdlib.h>

#include <syslog.h>

int main(int argc, char *argv[])
{
	FILE *fout;
	
	openlog(NULL, 0, LOG_USER);
	
	if (argc != 3)
	{
		// printf("usage: %s <path> <content>\n", argv[0]);
		syslog(LOG_ERR, "usage: %s <path> <content>\n", argv[0]);
		closelog();
		exit(1);
	}
	
	syslog(LOG_DEBUG, "Writing %s to %s\n", argv[2], argv[1]);

	fout = fopen(argv[1], "w");
	if (fout == NULL)
	{
		// printf("error opening file %s\n", argv[1]);
		syslog(LOG_ERR, "error opening file %s\n", argv[1]);
		closelog();
		exit(1);
	}
	
	if (fprintf(fout, "%s", argv[2]) < 0)
	{
		syslog(LOG_ERR, "error writing file %s\n", argv[1]);
		closelog();
		exit(1);
	}
	fclose(fout);

	closelog();
	
	return 0;
}
