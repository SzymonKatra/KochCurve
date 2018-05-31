# KochCurve

## Summary
Interactive koch curve fractal. Runs on Linux systems.

Written in C with Allegro5 support and assembly (both x86 and x86_64). Algorithm uses SSE instruction set for computing points of fractal. 

## Build
Use makefile, provides support for compiling 32 and 64 bit version.  
```make``` or ```make 64```- compile for x86_64, generates *main64* executable  
```make 86``` - compile for x86, generates *main86* executable

Prerequisites to compile:
- gcc
- nasm
- allegro5.2

## Screenshot

![Screenshot](screenshot.png)
