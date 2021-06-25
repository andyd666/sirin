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

#define LOG_NAME "log.hex"
#define PIPE_NAME "andrii_s_sniffer_pipe"
#define BUF_LEN 65536

#define STATUS_START     1
#define STATUS_STOP      2
#define STATUS_SELECT    4
#define STATUS_SHOW      8
#define STATUS_STAT_IP  16
#define STATUS_STAT_ALL 32


#ifndef CODE_DAEMON_FUNCTIONS_H_
#define CODE_DAEMON_FUNCTIONS_H_

struct sig_handler_data
{
	int *pipe_file_descriptor;
	int *log_file_descriptor;
	int *sock_raw;
	char *buffer;
};

struct sig_handler_data handler_data;

int open_log(void);
char* allocate_buffer_memory(int);
int open_pipe(char*);
int create_socket(void);
void set_iface(int, char*);
void make_daemon(void);
void signal_handler(int, siginfo_t*, void*);
void process_packet(int, unsigned int, char*, char*);

#endif // CODE_DAEMON_FUNCTIONS_H_
