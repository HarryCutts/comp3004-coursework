CFLAGS=-I. -I../glm `pkg-config --cflags --static --libs gl glew` -lglfw -Wall -Werror

main: main.cpp utils.cpp scene.cpp generators.cpp glm.c
	g++ -g -o main $^ $(CFLAGS)
