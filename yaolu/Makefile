CC=g++
CFLAGS=libftd2xx.a -ldl -lrt -lpthread -Wall -Wextra
APP = project

all: $(APP)

$(APP): ft232h.h ft232h.cpp main.cpp
	$(CC) main.cpp ft232h.cpp -o $(APP) $(CFLAGS)
	
clean:
	rm -f *.o ; rm $(APP)
