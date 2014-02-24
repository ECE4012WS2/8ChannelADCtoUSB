CC=gcc
CXX = g++
CFLAGS=-Wall -Wextra
LFLAGS=-lftd2xx  -Wl,-rpath /usr/local/lib 
APP = project

all: clean $(APP)

%.o: %.cpp
	@echo Compiling $*
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(APP): ft232h.h ft232h.o main.o
	$(CXX)  -o $(APP)  ft232h.o main.o $(LFLAGS)
	@echo Build Complete!

clean:
	@rm -fv *.o 
	@rm -fv $(APP)
