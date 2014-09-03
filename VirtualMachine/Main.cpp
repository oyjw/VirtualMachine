#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "slang.h"
#include <cstring>
#include <iostream>
void usage(){
	std::cout << "use vm -p distFile srcFile to see the bytecodes or" << std::endl;
	std::cout << "    vm srcFile to run the interpreter" << std::endl;
}

void getPrintFile(int i, int argc, const char* args[], std::string& printFile){
	if (i >= argc){
		usage();
		exit(1);
	}
	else{
		printFile = args[i];
	}
}

void parseOpt(int argc, const char* args[], std::string& srcFile, std::string& printFile){
	if (argc == 0){
		usage();
		exit(1);
	}
	for(int i = 0; i < argc; i++){
		if (strcmp(args[i], "-p") == 0){
			getPrintFile(++i, argc, args, printFile);
		}
		else{
			srcFile = args[i];
			if (i != argc - 1){
				usage();
				exit(1);
			}
		}
	}
}

int main(int argc, const char* argv[]){
	{std::string srcFile, printFile;
	parseOpt(argc-1, &argv[1], srcFile, printFile);
	void *state = newState();
	parseFile(state,srcFile.c_str(),printFile.c_str());
	freeState(state);
	}
	#ifdef WIN32
	_CrtDumpMemoryLeaks();
	#endif
	return 0;
}