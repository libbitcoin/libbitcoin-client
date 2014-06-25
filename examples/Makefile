CXXFLAGS += $(shell pkg-config --cflags libbitcoin-client) -ggdb -std=c++11
LIBS += $(shell pkg-config --libs libbitcoin-client)

default: all

.cpp.o:
	$(CXX) -o $@ -c $< $(CXXFLAGS)

client: client.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

all: client

clean:
	rm -f client
	rm -f *.o
