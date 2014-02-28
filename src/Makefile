CC=gcc
CXX = g++ 
CFLAGS=-Wall -Wextra
CXXFLAGS=-Wall -Wextra
LFLAGS=-lftd2xx  -Wl,-rpath /usr/local/lib -L/usr/local/lib 
APP = project

all: $(APP)

%.o: %.cpp
	@echo Compiling $*
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(APP): ft232h.h ft232h.o main.o Sample.o Socket.o UDPSocket.o TCPSocket.o
	$(CXX)  -o $(APP)  ft232h.o main.o Sample.o Socket.o UDPSocket.o TCPSocket.o $(LFLAGS)
	@echo Build Complete!

clean:
	@rm -fv *.o 
	@rm -fv $(APP)
