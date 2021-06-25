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

#ifndef CODE_CLI_FUNCTIONS_H_
#define CODE_CLI_FUNCTIONS_H_

#define log_path "/log"
#define pipe "andrii_s_sniffer_pipe"
#define buf_len 8192

#define STATUS_START 1
#define STATUS_STOP 2
#define STATUS_SELECT 4
#define STATUS_SHOW 8
#define STATUS_STAT_IP 16
#define STATUS_STAT_ALL 32


void check_input(int, char **, uint8_t *);
void get_daemon_pid(pid_t*);

#endif // CODE_CLI_FUNCTIONS_H_
