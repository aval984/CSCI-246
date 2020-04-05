a.out: main.o 
	g++ -g -std=c++17 main.o
main.o: Node.h main.cpp
	g++ -g -std=c++17 -c main.cpp
clean:
	rm a.out core *.o
