CC=g++

CFLAGS= -Wall
LIBFLAGS= -c -fPIC
# .cpp-files
SRC = src/
# .h-files
INC = inc/
# Where .o-files will be created
OBJ = obj
# Where program will be created
BIN = bin/

OBJOUT = -o $(OBJ)/

fileops.o : $(SRC)fileops.cpp $(INC)fileops.h $(OBJ)
			$(CC) $(CFLAGS) -I $(INC) $(LIBFLAGS) $(SRC)fileops.cpp $(OBJOUT)fileops.o

netops.o : $(SRC)netops.cpp $(INC)netops.h $(OBJ)
			$(CC) $(CFLAGS) -I $(INC) $(LIBFLAGS) $(SRC)netops.cpp -lpthread $(OBJOUT)netops.o

fsm.o : $(SRC)fsm.cpp $(INC)fsm.h $(OBJ)
			$(CC) $(CFLAGS) -I $(INC) $(LIBFLAGS) $(SRC)fsm.cpp $(OBJOUT)fsm.o

libtftp : fileops.o netops.o fsm.o $(BIN)
			$(CC) -shared -Wl,-soname,libtftp.so.1 -o $(BIN)libtftp.so.1.0.1 $(OBJ)/fsm.o $(OBJ)/netops.o $(OBJ)/fileops.o -lc

# g++ -Wall -g -I ../inc/ -lpthread -o client client.cpp fsm.cpp netops.cpp
client : fileops.o netops.o fsm.o $(SRC)client.cpp $(BIN)
#			$(CC) $(CFLAGS) -L. $(SRC)client.cpp $(OBJ)/fileops.o $(OBJ)/netops.o $(OBJ)/fsm.o -o ./$(BIN)client -I $(INC)
			$(CC) $(CFLAGS) -I $(INC) -lpthread -o $(BIN)client $(SRC)client.cpp $(OBJ)/fsm.o $(OBJ)/netops.o

server : fileops.o netops.o fsm.o $(SRC)server.cpp $(BIN)
			$(CC) $(CFLAGS) -I $(INC) -lpthread -o $(BIN)$@ $(SRC)server.cpp $(OBJ)/fsm.o $(OBJ)/netops.o

all : server client

clean:
	$(RM) $(OBJ)/*.o
	$(RM) $(BIN)/*

# Create directories which might be lost from CVS
$(DEP) $(OBJ) $(BIN) $(MO):
	mkdir -p $@
