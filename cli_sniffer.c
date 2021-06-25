#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <signal.h>
#include <netinet/ip.h>
#include "cli_functions.h"


int main(int argc, char * argv[])
{
	int fifo_pipe_created;
	int pipe_file_descriptor;
	
	int log;
	pid_t daemon_pid;
	uint8_t main_input_status; // This is for bool variables

	char *iface_name = (char*)malloc(sizeof(char) * 10);
	iface_name[0] = 0;

	check_input(argc, argv, &main_input_status);
	printf("Main input status = %d\n", main_input_status);
	get_daemon_pid(&daemon_pid);

	

	log = open(log_path, O_CREAT|O_RDWR|O_TRUNC, 0777);
	if(log < 0)
	{
		printf("Error opening log\n%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Creating FIFO pipe:
	fifo_pipe_created = mkfifo(pipe, 0666);
	if(fifo_pipe_created < 0)
	{
		printf("Error number: %d\n", errno);
		printf("%s\n", strerror(errno));
		if(errno != EEXIST) exit(EXIT_FAILURE); // If pipe exist then continue 
	}

	// Opening pipe for writing:
	pipe_file_descriptor = open(pipe, O_WRONLY, 0666); // Here is blocking opening, 
                                                           // which means the pipe should be opened by daemon before or
                                                           // simultaneously
	if(pipe_file_descriptor < 0)
	{
		printf("Cannot open pipe for writing\nErrno = %d\n", errno);
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	write(pipe_file_descriptor, (char*)main_input_status, 1); // write input status into pipe, so daemon can read it
	switch(main_input_status)
	{
		case STATUS_START:
		case STATUS_STOP:
		case STATUS_SHOW:
		case STATUS_STAT_IP:
		case STATUS_STAT_ALL:
			break; // Actrually do nothing, because daemon will be started itself
		case STATUS_SELECT:
			iface_name = strcat(iface_name, argv[3]);
			write(pipe_file_descriptor, iface_name, 10);
			break;
		default:
			printf("Something actually went wrong...\n");
			break;
	}

	close(log);
	close(pipe_file_descriptor);
	return 0;
}






