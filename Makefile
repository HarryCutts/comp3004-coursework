CFLAGS=-I./include -I./glm `pkg-config --cflags --static --libs gl glew` -lglfw -Wall -Werror \
       -D ASSET_DIRECTORIES

main: src/main.cpp src/utils.cpp src/scene.cpp src/generators.cpp src/glm.c
	g++ -g -o main $^ $(CFLAGS)
