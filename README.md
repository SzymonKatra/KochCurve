# KochCurve

Interactive koch curve algorithm. Runs on Linux systems.

Written in C with Allegro5 support and assembly (both x86 and x86_64). Uses SSE instruction set.

To compile, use makefile. It's compiling both 32 and 64 bit version. If you want to compile only for one architecture, edit makefile and comment out unnecessary commands.

Prerequisites before compiling:
- gcc
- nasm
- allegro5.2
