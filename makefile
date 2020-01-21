CC=g++
CFLAGS=-c -Wall -ggdb
LDFLAGS= -lrt -ldl -lm -lncurses -lsqlite3 -O2 -Wall
SOURCES= main.cpp library.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=run


all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o: 
	$(CC) $(CFLAGS) $< -o $@
