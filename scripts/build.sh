#!/bin/bash

gcc -g -o compiler -I../src ../src/*.c ../src/lib/*.c -lfl

