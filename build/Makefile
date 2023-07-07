CXX = g++
# CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ../code/*.cpp ../code/log/*.cpp ../code/server/*.cpp ../code/sql/*.cpp

all: $(OBJS)
	$(CXX) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient


clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
