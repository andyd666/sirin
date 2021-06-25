# This is the very first make file I evere created d:)
#                                   a guy in a cap ^^^

# compiler -> gcc
cc = gcc

# flags:
# -g		-> for debugging
# -Wall		-> turns on some compiler warnings
compiler_flags = -Wall

# CLI target:
cli = cli_sniffer
cli_func = cli_functions

# daemon target:
daemon = daemon_sniffer
daemon_func = daemon_functions

all:
	$(cc) $(compiler_flags) -o $(cli) $(cli).c $(cli_func).c
	$(cc) $(compiler_flags) -o $(daemon) $(daemon).c $(daemon_func).c

cli:
	$(cc) $(compiler_flags) -o $(cli) $(cli).c $(cli_func).c

daemon:
	$(cc) $(compiler_flags) -o $(daemon) $(daemon).c $(daemon_func).c

clean:
	$(rm) $(cli)
	$(rm) $(daemon)
