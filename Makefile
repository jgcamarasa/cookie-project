#Compiler
CC=g++

#Options
CFLAGS=-Wall -pedantic -g -pthread

#Libraries
LIBS=-L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lBox2D

#Includes
INCLUDES+=-I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads 
INCLUDES+=-I/opt/vc/include/interface/vmcs_host/linux -I./ -I../libs/ilclient -I../libs/vgfont -I/usr/include/glm

#Directories
SRC=src/

all: cookie

cookie: main.o Renderer.o State.o
	$(CC) main.o Renderer.o State.o $(LIBS) $(INCLUDES) -o main

main.o: $(SRC)main.cpp
	$(CC) -c $(SRC)main.cpp $(LIBS) $(INCLUDES)
	
Renderer.o: $(SRC)Renderer.cpp
	$(CC) -c $(SRC)Renderer.cpp $(LIBS) $(INCLUDES)
	
State.o: $(SRC)State.cpp
	$(CC) -c $(SRC)State.cpp $(LIBS) $(INCLUDES)
