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


void check_input(int argc, char * argv[], uint8_t * status)
{
	int is_correct_command = 0;

	if(argc == 1)
	{
		printf("Error using\nTry \"--help\" for help\n");
		exit(EXIT_SUCCESS);
	}
	else if(argc == 2)
	{
		if(strcmp(argv[1], "--help") == 0)
		{
			is_correct_command = 1;
			printf("\"start\" to start sniffer from default interface (eth0)\n\n");
			printf("\"stop\" to stop sniffer\n\n");
			printf("\"show [ip] count\" prints number of packets received from ip address\n\n");
			printf("\"select iface [iface]\"cselect interface for sniffing\n\n");
			printf("\"stat [iface]\" show all collected statistics for particular interface. If [iface] ommited - for all interfaces\n\n");
			exit(EXIT_SUCCESS);
		}
		else if(strcmp(argv[1], "start") == 0)
		{
			*status = STATUS_START;
			is_correct_command = 1;
		}	
		else if(strcmp(argv[1], "stop") == 0)
		{
			*status = STATUS_STOP;
			is_correct_command = 1;
		}
		else if(strcmp(argv[1], "stat") == 0)
		{
			*status = STATUS_STAT_ALL;
			is_correct_command = 1;
		}
	}
	else if(argc == 3)
	{
		if(strcmp(argv[1], "stat") == 0)
		{
			*status = STATUS_STAT_IP;
			is_correct_command = 1;
		}
	}
	else if(argc == 4)
	{
		if((strcmp(argv[1], "show") == 0) & (strcmp(argv[3], "count") == 0))
		{
			*status = STATUS_SELECT;
			is_correct_command = 1;
		}
		else if((strcmp(argv[1], "select") == 0) & (strcmp(argv[2], "iface") == 0))
		{
			*status = STATUS_SHOW;
			is_correct_command = 1;
		}
	}
	
	if(is_correct_command == 0)
	{
		printf("Command incorrect. Try using \"--help\" for help\n");
		exit(EXIT_SUCCESS);
	}
}


void get_daemon_pid(pid_t * daemon_pid)
{
	char text_buffer[buf_len];
	int is_daemon_running;

	is_daemon_running = system("pidof -s daemon_sniffer");
	if(is_daemon_running != 0) // If daemon is not running then start it:
	{
		printf("Starting daemon\n");
		*daemon_pid = fork();
		if((*daemon_pid) == 0)
		{			
			execl("./daemon_sniffer", NULL);
			exit(EXIT_SUCCESS);
		}	
	}
	
	FILE *cmd;	
	cmd = popen("pidof -s daemon_sniffer", "r");
	fgets(text_buffer, buf_len - 1, cmd);
	*daemon_pid = strtoul(text_buffer, NULL, 10);
	pclose(cmd);
	printf("daemon_pid = %d\n", *daemon_pid);
}
