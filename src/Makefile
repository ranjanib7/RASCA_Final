RM        := /bin/rm -rf
SIM       := ./sim
CC        := g++
CFLAGS    := -lz -O2 -lm  -W -Wall -Wno-deprecated -Wno-unknown-warning-option -Wno-self-assign -Wno-unused-parameter -Wno-unused-but-set-variable
DFLAGS    := -pg -g
PFLAGS    := -pg




all:  traces folders
	${CC} ${CFLAGS} dram.c  mgries.c scoreboard.c mcore.c os.c  mcache.c sim.c memsys.c  hash_lib.c malloc_lib.c -o ${SIM}

folders:
	mkdir -p ../RESULTS ../TRACES

traces:
	echo "TODO: Download traces"
clean: 
	$(RM) ${SIM} *.o
