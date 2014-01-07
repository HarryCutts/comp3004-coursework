COMP3004 Coursework 3, Harry Cutts
==================================

This is my submission for Principles of Computer Graphics (COMP3004) Coursework 3. The program is written in C, C++, and GLSL, the models were made in Blender, and the textures were created or modified in GIMP. For credits, please see `credits.txt`.

Files
-----

`credits.txt` contains the credits for this program.
`screenshot.jpg` is a screenshot, taken from the starting point of the program.
`readme.txt` is this file.

`generators.cpp` contains code for generating shapes and a wrapper around Nate Robins' OBJ loader.
`glm.c` contains Nate Robins' OBJ loader (see `credits.txt`).
`main.cpp` sets up OpenGL, processes input, and contains the `main` method.
`scene.cpp` animates objects, and sets up the scene and its animations.
`utils.cpp` contains utility methods.

`paths.h` contains macros for managing asset (i.e. model, texture, shader) paths, to allow easier flattening of the directory structure for handin.
The remaining header files are the headers for their corresponding `c` or `cpp` files.

`Makefile` contains build instructions for the program, in a format compatible with GNU Make.

`shaders_fragment.glsl` contains the fragment shader.
`shaders_vertex.glsl` contains the vertex shader.

`models_clanger.obj` contains my clanger model.
`models_landscape.obj` contains the model of the terrain.
`models_spaceship.obj` contains a model of the spaceship.
The `mtl` files contain their corresponding material definitions.
`textures_*.tga` files contain textures for the corresponding model files.

Build instructions
------------------

To build the program on Linux (tested on Ubuntu 12.04) use the provided Makefile. GLFW 2, OpenGL development files and GLEW are required, as well as the usual build tools. If you have cloned the repository using Git, you will need to initialize the submodules like so:

	git submodule init
	git submodule update

Controls
--------

The mouse is not used in this program. The following keys are used:

UP:   Increase the forward speed of the camera
DOWN: Decrease the forward speed of the camera
LEFT, RIGHT:
      Look to your left or right
HOME, END:
      Look up or down
P:    Return the camera to the starting position (from which `screenshot.jpg` was taken)
D:    Print the coordinates of the camera to standard out

T:    Start the tour

H:    Print this help file to standard out

ESCAPE, Q:
      Quit
