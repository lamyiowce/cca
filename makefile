all: model clean

main.o: main.cpp
	g++ main.cpp -c -fopenmp

model: main.o
	g++ -o model main.o -fopenmp -lsfml-graphics -lsfml-window -lsfml-system
clean: 
	rm main.o