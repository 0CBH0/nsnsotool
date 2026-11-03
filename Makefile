DEPPATH=./dep
CC=clang
CXX=clang++
CFLAGS=-g -Wall -O3 -pedantic -I $(DEPPATH)
CXXFLAGS=$(CFLAGS)
LDFLAGS=-static

SRC=nsnsotool.cpp
OBJ=$(SRC:.cpp=.o) $(DEPPATH)/lz4.o
BIN=nsnsotool.exe

.PHONY: all clean

all: $(BIN)
	@echo Done

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

$(SRC:.cpp=.o): $(SRC) nsnsotool.h $(DEPPATH)/lz4.h
	$(CXX) $(CXXFLAGS) -c $(SRC) -o $@

$(DEPPATH)/lz4.o: $(DEPPATH)/lz4.c $(DEPPATH)/lz4.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)
