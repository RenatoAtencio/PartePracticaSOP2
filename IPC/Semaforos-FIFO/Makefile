CC=g++
CFLAGS=-Wall -std=c++17
BINS=fifo semf

all: clean fifo semf

fifo:
	$(CC) $(CFLAGS) -o fifo fifo.cpp

semf:
	$(CC) $(CFLAGS) -o semf semaforo.cpp

clean:
	@echo " [CLN] Removing binary files"
	@rm -f $(BINS)