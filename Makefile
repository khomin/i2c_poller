#LIB= -lpq -lgps#-lssl -lcrypto #-levent -levent_core
#INC= -I$(CANTCOAP_DIR) -I$(RAPIDJSON_DIR)

CXX=g++
LIB=-lpq
#CXX=arm-linux-gnueabi-g++
CXXFLAGS=-Wall -std=c++11 -pthread -g -O0 -DDEBUG

all: main

default: main

dbConnection.o:
	$(CXX) $(CXXFLAGS) ./src/dbConnection.cpp -c -o ./bin/$@
	
global.o:
	$(CXX) $(CXXFLAGS) ./src/global.cpp -c -o ./bin/$@
		
logger.o:
	$(CXX) $(CXXFLAGS) ./src/logger.cpp -c -o ./bin/$@
	
I2cpoll.o: dbConnection.o
	$(CXX) $(CXXFLAGS) ./src/I2cpoll.cpp -c -o ./bin/$@
	
main.o: 
	$(CXX) $(CXXFLAGS) ./src/main.cpp -c -o ./bin/$@ 

main: main.o I2cpoll.o logger.o global.o
	$(CXX) $(CXXFLAGS) ./bin/main.o ./bin/I2cpoll.o ./bin/logger.o ./bin/global.o ./bin/dbConnection.o -o ./bin/i2c_poller $(LIB)

clean:
	rm -f ./bin/*

