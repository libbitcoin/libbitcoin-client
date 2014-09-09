CXXFLAGS += $(shell pkg-config --cflags libbitcoin-client) -ggdb -std=c++11
LIBS += $(shell pkg-config --libs libbitcoin-client)

default: all

.cpp.o:
	$(CXX) -o $@ -c $< $(CXXFLAGS)

client: cli.o connection.o main.o read_line.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

all: client

clean:
	rm -f client
	rm -f *.o
