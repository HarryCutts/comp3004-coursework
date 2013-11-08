CFLAGS=-I./include -I./glm `pkg-config --cflags --static --libs gl glew` -lglfw

main: src/main.cpp src/utils.cpp src/generators.cpp
	g++ -g -o main $^ $(CFLAGS)
