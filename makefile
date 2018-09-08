CC=g++
CFLAGS=-c -Wall -ggdb
LDFLAGS= -lrt -ldl -lm -lflxmlrpc -lncurses  -O2 -Wall
SOURCES= main.cpp datarobot.cpp dev.cpp myxmlrpc.cpp
OBJECTS=$(SOURCES:.cpp=.o) sqlite3.obj
EXECUTABLE=run


all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o: 
	$(CC) $(CFLAGS) $< -o $@
