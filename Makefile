CXX = g++
CPPFLAGS += -g
CXXFLAGS += 
LDFLAGS +=
LIBS += $(shell xmlrpc-c-config c++2 abyss-server --libs)

all: t1000

t1000: main.o xmlrpc_methods.o
	$(CXX) $(LDFLAGS) -o t1000 main.o xmlrpc_methods.o $(LIBS)

main.o: main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o main.o -c main.cpp

xmlrpc_methods.o: xmlrpc_methods.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o xmlrpc_methods.o -c xmlrpc_methods.cpp

.PHONY: clean
clean:
	rm -rf *.o t1000
