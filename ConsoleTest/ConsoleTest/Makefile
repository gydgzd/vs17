CXX=g++
CC=gcc

INCLUDES=-I./  -I./include/ -I/usr/local/boost_1_68_0/include

LDFLAGS=-L./lib -L/usr/local/boost_1_68_0/lib
LIBS=-lpthread -lboost_thread -lboost_system -Wl,-rpath=.

CFLAGS=-g -Wall -Wno-unused -fPIC -D__LINUX__ ${INCLUDES}
CXXFLAGS = -g -std=c++11 -Wall -Wno-unused -fmessage-length=0 ${INCLUDES} -D__linux

TARGET_CPPS := $(shell ls src/*.cpp)
TARGET_OBJS=$(TARGET_CPPS:.cpp=.o)

TARGET = test

$(TARGET):	$(TARGET_OBJS)
	$(CXX) ${LDFLAGS} -o $@ $^ ${LIBS}

all:	$(TARGET)

%.o:%.cpp
	$(CXX) -c ${LDFLAGS} $(CXXFLAGS) -o $@ $<
	
clean:
	rm -f $(TARGET_OBJS) $(TARGET)
