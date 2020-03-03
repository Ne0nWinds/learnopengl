#!/bin/sh
gcc main.c -o main.exec -I./src/ -lm `pkg-config --libs glfw3 glew --cflags cglm`
