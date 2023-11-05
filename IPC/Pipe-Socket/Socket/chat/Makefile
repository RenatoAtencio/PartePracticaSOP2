all:
	make clean
	g++ -o server servidor.cpp -pthread -O3 -Wall 
	g++ -o client cliente.cpp -O3 -Wall -pthread
clean:
	rm -f main
