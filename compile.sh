#!/bin/sh
gcc main.c -o main.exec -I/usr/include -lm `pkg-config --libs glfw3 glew --cflags cglm`
