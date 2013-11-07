CFLAGS=-I./include -I./glm `pkg-config --cflags --static --libs gl glew` -lglfw

main: src/main.cpp src/utils.cpp
	g++ -g -o main src/main.cpp src/utils.cpp $(CFLAGS)
