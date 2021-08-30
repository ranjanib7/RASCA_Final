RM        := /bin/rm -rf
SIM       := ./sim
CC        := g++
CFLAGS    := -lz -O2 -lm  -W -Wall -Wno-deprecated -Wno-unknown-warning-option -Wno-self-assign -Wno-unused-parameter -Wno-unused-but-set-variable
DFLAGS    := -pg -g
PFLAGS    := -pg




all:  traces folders
	${CC} ${CFLAGS} dram.c  mgries.c scoreboard.c mcore.c os.c  mcache.c sim.c memsys.c  hash_lib.c malloc_lib.c -o ${SIM}

folders:
	mkdir -p ../RESULTS

traces:
	echo "Downloading traces..." ; wget -O traces.tar.gz "https://www.dropbox.com/s/au0zom0vu2rus4c/traces.tar.gz?dl=1" ; mv traces.tar.gz ../. ;  cd ../ ; tar -zxvf traces.tar.gz;

clean: 
	$(RM) ${SIM} *.o
