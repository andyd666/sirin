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
#include "daemon_functions.h"


int open_log(void)
{
	int log_descriptor;
	log_descriptor = open(LOG_NAME, O_CREAT|O_RDWR|O_TRUNC, 0666);
	if(log_descriptor < 0)
	{
		printf("Cannot open log-file, errno = %d\n%s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);	
	}
	return log_descriptor;
}


char* allocate_buffer_memory(int buf_size)
{
	char *buffer;
	buffer = (char *)malloc(buf_size * sizeof(char));
	if(buffer == NULL)
	{
		printf("Cannot allocate memory for buffer, errno = %d\n%s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
		
	}
	return buffer;
}


int open_pipe(char *pipe_name)
{
	int fifo_pipe_created;
	int pipe_file_descriptor;
 
	fifo_pipe_created = mkfifo(pipe_name, 0666);
	if(fifo_pipe_created < 0)
	{
		printf("Daemon: Error number: %d\n", errno);
		printf("%s\n", strerror(errno));
		if(errno != EEXIST) exit(EXIT_FAILURE);
	}

	// Opening pipe for reading:
	pipe_file_descriptor = open(pipe_name, O_RDONLY | O_NONBLOCK, 0666);
	if(pipe_file_descriptor < 0)
	{
		printf("Daemon: Cannot open pipe for writing\nErrno = %d\n", errno);
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return pipe_file_descriptor;
}


int create_socket(void)
{
	int sock_raw;	

	sock_raw = socket(AF_PACKET , SOCK_RAW , htons(ETH_P_ALL));
	if(sock_raw < 0)
	{
		printf("Daemon: cannot open socket\nErrno = %d\n", errno);
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	return sock_raw;
}


void set_iface(int sock_raw, char *iface_name)
{
	int set_socket_success;
	
	set_socket_success = setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , iface_name , strlen(iface_name) + 1 );
	if(set_socket_success < 0)
	{
		printf("Daemon: cannot set socket options\nErrno = %d\n", errno);
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}


void make_daemon(void)
{
	int fd_in, fd_out, fd_err;
	
	printf("Daemon: daemon is being created\n");
	for(int i = 0; i<3; i++) close(i);
	fd_in = open("/dev/null", O_WRONLY, 0);
	fd_out = open("/dev/null", O_RDONLY, 0);
	fd_err = open("/dev/null", O_RDONLY, 0);
}


void signal_handler(int signo, siginfo_t* si, void* ucontext)
{
	int pipe_file_descriptor = *handler_data.pipe_file_descriptor;
	int log_file_descriptor = *handler_data.log_file_descriptor;
	int sock_raw = *handler_data.pipe_file_descriptor;
	char *buffer = handler_data.buffer;
	char iface_name[10];
	
	char cli_status;
	
	if(signo == SIGUSR1)
	{
		read(pipe_file_descriptor, &cli_status, 1);
		switch(cli_status)
		{
			case STATUS_START: // Actually, this status is not needed, beacuse calling the CLI starts daemon itself
				break;
			case STATUS_STOP:
				free(buffer);
				close(pipe_file_descriptor);
				close(log_file_descriptor);
				close(sock_raw);
				exit(EXIT_SUCCESS);
				break; //break is not needed, but still...
			case STATUS_SELECT:
				read(pipe_file_descriptor, iface_name, 10);
				set_iface(sock_raw, iface_name);
				break;
			case STATUS_SHOW: // CLI shows statistics, so it just need to be flushed to file
			case STATUS_STAT_IP:
			case STATUS_STAT_ALL:
				// fflush(log_file_descriptor); // I thougth thar was needed for writing, but no...
				break;
			default: // if something went wrong, then return failure;
				free(buffer);
				close(pipe_file_descriptor);
				close(log_file_descriptor);
				close(sock_raw);
				exit(EXIT_FAILURE);
				break; 
		}
	}
}


void process_packet(int log_file_descriptor, unsigned int data_size, char *buffer, char *iface_name)
{
	struct iphdr *iph = (struct iphdr*)buffer;
	char source_ip[4];
	char ch_data_size[4];
	
	source_ip[0] = ((iph->saddr) >> 24) & 0xff;
	source_ip[1] = ((iph->saddr) >> 16) & 0xff;
	source_ip[2] = ((iph->saddr) >> 8) & 0xff;
	source_ip[3] = (iph->saddr) & 0xff;

	ch_data_size[0] = (data_size >> 24) & 0xff;
	ch_data_size[1] = (data_size >> 16) & 0xff;
	ch_data_size[2] = (data_size >> 8) & 0xff;
	ch_data_size[3] = data_size & 0xff;

	write(log_file_descriptor, source_ip, 4);
	write(log_file_descriptor, iface_name, 10);
	write(log_file_descriptor, ch_data_size, 4);
	//printf("%d.%d.%d.%d\n", (source_ip>>24)&0xff, (source_ip>>16)&0xff, (source_ip>>8)&0xff, (source_ip)&0xff);
	
}

/* Here is not working code... It doesn't work only because I don't want to waste my time to make it work, but here me out!
The following idea was to decrease search time for each interface by creating its own log-file but it would have taken too much time to make it work
and will greatly increase the complexity of code.
So I just cut it and I will try to make it simplier. And for now, look at Zoidberg, he is cute!

                                                                       xmHTTTTT%ms.
                                                                    z?!!!!!!!!!!!!!!?m
                                                                  z!!!!!!!!!!!!!!!!!!!!%
                                                               eHT!!!!!!!!!!!!!!!!!!!!!!!L
                                                              M!!!!!!!!!!!!!!!!!!!!!!!!!!!>
                                                           z!!!!!!!!!!XH!!!!!!!!!!!!!!!!!!X
                                                           "$$F*tX!!W?!!!!!!!!!!!!!!!!!!!!!
                                                           >     M!!!   4$$NX!!!!!!!!!!!!!t
                                                           tmem?!!!!?    ""   "X!!!!!!!!!!F
                                                      um@T!!!!!!!!!!!!s.      M!!!!!!!!!!F
                                                   .#!!!!!!!!!!!!!!!XX!!!!?mM!!!!!!!!!!t~
                                                  M!!!@!!!!X!!!!!!!!!!*U!!!!!!!!!!!!!!@
                                                 M!!t%!!!W?!!!XX!!!!!!!!!!!!!!!!!!!!X"
                                                :!!t?!!!@!!!!W?!!!!XWWUX!!!!!!!!!!!t
                                                4!!$!!!M!!!!8!!!!!@$$$$$$NX!!!!!!!!-
                                                 *P*!!!$!!!!E!!!!9$$$$$$$$%!!!!!!!K
                                                    "H*"X!!X&!!!!R**$$$*#!!!!!!!!!>
                                                        'TT!?W!!9!!!!!!!!!!!!!!!!M
                                                        '!!!!!!!!!!!!!!!!!!!!!!!!F
                                                        '!!!!!!!!!!!!!!!!!!!!!!!!>
                                                        '!!!!!!!!!!!!!!!!!!!!!!!M
                                                        J!!!!!!!!!!!!!!!!!!!!!!!F K!%n.
         @!!!!!??m.                                  x?F'X!!!!!!!!!!!!!!!!!!!!HP X!!!!!!?m.
Z?L      '%!!!!!!!!!?s                            .@!\~ MB!!!!!!!!!!!!!!!!!U#!F X!!!!!!!!X#!%.
E!!N!k     't!!!!!!!!!?:                       zTX?!t~ M!t!!!!!!!!!!!!!!UM!!!F 4!!!!!!!!t%!!!!?.
!!!!!!hzh.   "X!!!!!!!!!>                  .+?!!3?!X  Z!!!B!!!!!!!!!!UM!!!!!" 4!!!!!!!!t?!!!!!!!h
?!!!!!!!!!*!?L %!!!!!!!!?               .+?!!!!3!!\  P!!!!?X!!!!!!U#!!!!!!X" 4!!!!!!!!\%!!!!!!!!!?
'X!!!!!!!!!!!!?TTTT*U!!!!k            z?!!!!!!t!!!- J!!!!!!9!!X@T!!!!!!!!X~ d!!!!!!!!!%!!!!!!!!!!!!L
 4!!!!!!!!!!!!!!!!!!!!!!!M          'W!!!!!!!X%!!P  %!!!!!!!T!!!!!!!!!!!X~ J!!!!!!!!!P!!!!!!!!!!!!!!\
  5!!!!!!!!!!!!!!!!!!!!!!!?m.       .@Ti!!!!!Z!!t  d!!!!!!!!!!!!!!!!!!!X-.JUUUUX!!!!J!!!!!!!!!!!!!!!!!
   %!!!!!!!!!!!!!!!!!!!!!!!!!!!TnzT!!!!!#/!!?!!X"  ^"=4UU!!!!!!!!!!U@T!!!!!!!!!!!!Th2!!!!!!!!!!!!!!!!!!
    ^t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!?K!K!!f               `""#X!!!!!!!!!!!!!!!!!?t!!!!!!!!!!!!!!!!(>
       "U!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!$!!F                      "tX!!!!!!!!!!!!!!!!b!!!!!!!!!!!!!!!(>
          '"*tUUX!X!!!!!!!!!!!!!!!!!!!!!!!!$!Z                          ^4!!!!!!!!!!!!!!!N!!!!!!!!!!!!!!!!
                 %!!!!!!!!!!!!!!!!!!!!!!!!X!X                              %W@@WX!!!!!!!!!N!!!!!!!!!!!!!!!
                  "X!!!!!!!!!!!!!!!!!!!!!@!!*        ..    ..  :m.. ETThmuM!!!!!!!!!!!!!!!!@m@*TTTT?!!!W!!
                    %!!!!!!!!!!!!!!!!!!W?!!X         M!!!TT?!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!9UU!!!!!!!!!M!f
                     't!!!!!!!!!!!!!!!P!!!!X          5!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!?NX!!!!!!L
                       "W!!!!!!!!!!!X#!!!!!R           "X!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!R!!!!!t
                         ^*X!!!!!!!t%!!!!!h              %X!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!>
                             "*U!!M!!!!!!X~ :?!!!T!+s...   *X!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                 :?!!!!!!> :?!!!!!!!!!!!!!!!!?tX!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!>
                                 %!!!!!!F .%!!!!!!!!!!!!!!!!!!!!!#4U!!!!!!!!!!!!U!!!!!!!!!!!!!!!!!!!!!!!!~
                                K!!!!!!Z  K!!!!!!!!!!!!!!!!!!!!!!!  F!!!!!?!!?X!!!!!!!!!!!!!!!!!!!!!!!!!Z
                               X!!!!!!t  H!!!!!!!!!!!!!!!!!!!!!!!!> !!!!!!!!!!!W!!!!!!!!!!!!!!!!!!!!!!!t
                               %!!!!!!F :!!!!!!!!!!!!!!!!!!!!!!!!!> !!!!!!!!!!!!#X!!!!!!!!!!!!!!!!!!!!X
                              '!!!!!!X  K!!!!!!!!!!!!!!!!!!!!!!!!!> K!!!!!!!!!!!!!?W!!!!!!!!!!!!!!!!X"

------------------------------------------------------------------------------------------------

char ** open_log_files(int * log_files, int * sock_raw, int * number_of_interfaces)
{
	struct ifaddrs *ifaddr;
	struct ifaddrs *temp_ifaddr;
	int got_addresses;
	char **interface_names;
	char *temp_log_file_name = (char *)malloc(sizeof(char)*128);

	(*number_of_interfaces) = 0;

	interface_names = (char **)malloc(sizeof(char*)*MAX_INTERFACES_NUMBER); // Hope 64 interfaces is enougth
	for(int i = 0; i < 64; i++)
	{	
		interface_names[i] = (char*)malloc(sizeof(char)*16); // And 16 characters for each interface name
		interface_names[i][0] = 0;
	}

	got_addresses = getifaddrs(&ifaddr);
	if(got_addresses < 0)
	{
		printf("Daemon: troubles getting ifaddr; errno = %d\n%s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	
	for(temp_ifaddr = ifaddr; temp_ifaddr != NULL; temp_ifaddr = temp_ifaddr->ifa_next)
	{	
		if(temp_ifaddr->ifa_addr == NULL) continue;
		if(temp_ifaddr->ifa_addr->sa_family != AF_INET) continue;
		strcat(interface_names[(*number_of_interfaces)++], temp_ifaddr->ifa_name);
		printf("%s\n", interface_names[(*number_of_interfaces)-1]);
	}
	
	log_files = (int *)malloc((*number_of_interfaces) * sizeof(int));
	
	for(int i = 0; i < (*number_of_interfaces); i++)
	{
		temp_log_file_name[0] = 0;
		temp_log_file_name = strcat(temp_log_file_name, log_folder);
		temp_log_file_name = strcat(temp_log_file_name, interface_names[i]);
		log_files[i] = open(temp_log_file_name, O_CREAT|O_RDWR|O_TRUNC, 0777);
		printf("%d\n", log_files[i]);
	}


	

	return interface_names;
}

*/
