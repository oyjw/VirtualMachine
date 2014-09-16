CFLAGS=-std=c++11 -g -wall
all:
	g++ -std=c++11 -g -gdwarf-2 -O0 -o vm Dict.cpp List.cpp String.cpp SymbolTable.cpp Tokenizer.cpp Parser.cpp VirtualMachine.cpp Main.cpp Slang.cpp
.PHONY:clean
clean:
	rm vm *.o
