CC=gcc
CXX = g++
CFLAGS=-Wall -Wextra
CXXFLAGS=-Wall -Wextra -O3
LFLAGS=-lftd2xx  -Wl,-rpath /usr/local/lib -L/usr/local/lib
APP = project

all: $(APP)

%.o: %.cpp
	@echo Compiling $*
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(APP):  simuladc.o example.o  Socket.o TCPSocket.o
	$(CXX)  -o $(APP)  simuladc.o example.o  Socket.o TCPSocket.o $(LFLAGS)
	@echo Build Complete!

clean:
	@rm -fv *.o
	@rm -fv $(APP)
