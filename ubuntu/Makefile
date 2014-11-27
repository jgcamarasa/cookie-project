#Compiler
CC=g++ -DDESKTOP -DGLFW_INCLUDE_ES2

#Options
CFLAGS=-Wall -pedantic -g -pthread 

#Libraries
LIBS=-L/usr/local/lib -lGLESv2 -lEGL -lrt -lBox2D -lglfw3 -pthread -lGLEW -lGLU -lGL -lrt -lXrandr -lXxf86vm -lXi -lXinerama -lX11

#Includes
INCLUDES+=-I/usr/include/ 
INCLUDES+=-I/opt/vc/include/interface/vmcs_host/linux -I./ -I../libs/ilclient -I../libs/vgfont -I/usr/include/glm

#Directories
SRC=../src/

all: cookie

cookie: main.o Renderer.o State.o
	$(CC) main.o Renderer.o State.o $(LIBS) $(INCLUDES) -o main

main.o: $(SRC)main.cpp
	$(CC) -c $(SRC)main.cpp $(LIBS) $(INCLUDES)
	
Renderer.o: $(SRC)Renderer.cpp
	$(CC) -c $(SRC)Renderer.cpp $(LIBS) $(INCLUDES)
	
State.o: $(SRC)State.cpp
	$(CC) -c $(SRC)State.cpp $(LIBS) $(INCLUDES)
