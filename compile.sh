#!/bin/sh
clang main.c -o main.exec `pkg-config --libs glfw3 glew`
