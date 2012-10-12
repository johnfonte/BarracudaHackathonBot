CXX = g++
CPPFLAGS += -g
CXXFLAGS += 
LDFLAGS +=
LIBS += $(shell xmlrpc-c-config c++2 abyss-server --libs)

all: hal9000

hal9000: main.o xmlrpc_methods.o
	$(CXX) $(LDFLAGS) -o hal9000 main.o xmlrpc_methods.o $(LIBS)

main.o: main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o main.o -c main.cpp

xmlrpc_methods.o: xmlrpc_methods.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o xmlrpc_methods.o -c xmlrpc_methods.cpp

.PHONY: clean
clean:
	rm -rf *.o hal9000
