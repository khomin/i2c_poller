CPPFLAGS=-Wall -std=c++11 -pthread -g -O0 -DDEBUG

DEST_PATH = /usr/bin
BUILD_MODE = run
NAME_BIN = i2c_poll

CPPFLAGS += -I/usr/include/postgresql

ifeq ($(BUILD_MODE),debug)
	CFLAGS += -g
else ifeq ($(BUILD_MODE),run)
	CFLAGS += -O2
else
	CFLAGS += -g
endif

all: main

default: main
	
I2cpoll.o: DbConnection.o
	$(CXX) $(CPPFLAGS) ./src/I2cpoll.cpp -c -o ./build/$@

DbConnection.o: I2cpoll.o
	$(CXX) $(CPPFLAGS) ./src/DbConnection.cpp -c -o ./build/$@
	
global.o: global.o
	$(CXX) $(CPPFLAGS) ./src/global.cpp -c -o ./build/$@
	
Accel.o:
	$(CXX) $(CPPFLAGS) ./src/Accel.cpp -c -o ./build/$@
	
logger.o:
	$(CXX) $(CPPFLAGS) ./src/logger.cpp -c -o ./build/$@
	
main.o: DbConnection.o I2cpoll.o logger.o
	$(CXX) $(CPPFLAGS) ./main.cpp -c -o ./build/$@
	
main: main.o DbConnection.o I2cpoll.o logger.o Accel.o
	$(CXX) $(CPPFLAGS) ./build/main.o ./build/DbConnection.o ./build/logger.o ./build/I2cpoll.o -o ./build/out -lpq
	

install:
	install ./build/${NAME_BIN} ${DEST_PATH}

unistall:
	rm -rf ${DEST_PATH}/${NAME_BIN}

clean:
	rm -r ./build/*
