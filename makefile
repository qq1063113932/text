CXX ?= g++

# 启用 C++11 支持
CXXFLAGS = -std=c++11 -Wall -O2

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: *.cpp
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient -ljsoncpp

run: server
	./server

clean:
	rm  -r server