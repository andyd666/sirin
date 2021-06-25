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

int main(int argc, char * argv[])
{
	int pipe_file_descriptor;
	int log_file_descriptor;
	int sock_raw;
	unsigned int socket_address_size;
	ssize_t data_size;
	struct sockaddr socket_address;
	char *buffer;
	struct sigaction sa;
	char iface_name[10] = "enp0s3"; // I am using virtual machine, so it is the only device I have :(

	handler_data.pipe_file_descriptor = &pipe_file_descriptor;
	handler_data.log_file_descriptor = &log_file_descriptor;
	handler_data.sock_raw = &sock_raw;
	handler_data.buffer = buffer;

	if(fork() > 0) exit(EXIT_SUCCESS); // this is needed to make sure daemon works properly

	log_file_descriptor = open_log();

	buffer = allocate_buffer_memory(BUF_LEN);
	buffer[0] = 0;	// just in case...

	pipe_file_descriptor = open_pipe(PIPE_NAME);

	sock_raw = create_socket();

	set_iface(sock_raw, iface_name);
	
	sa.sa_sigaction = signal_handler;
	sa.sa_flags |= SA_SIGINFO;
	sigaction(SIGUSR1, &sa, 0); // assign signal action

	make_daemon();

	
	// After this line the process is daemon and the only way to interact with it is to send signals and data through pipe:

	while(1)
	{	
		data_size = recvfrom(sock_raw , buffer , 65536 , 0 , &socket_address , &socket_address_size);
		process_packet(log_file_descriptor, data_size, buffer, iface_name);
	}

	free(buffer);
	close(log_file_descriptor);
	close(pipe_file_descriptor);
	close(sock_raw);
	return 0;
}
