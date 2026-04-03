#!/bin/bash

# Compile the main.cpp file

# Check if g++ is installed
if ! command -v g++ &> /dev/null
then
    echo "g++ could not be found, please install it to compile the program."
    exit
fi

# Compile main.cpp
g++ main.cpp -o main

# Check if the compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful, executable created as 'main'."
else
    echo "Compilation failed."
fi