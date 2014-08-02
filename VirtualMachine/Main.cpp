#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "slang.h"

int main(){
	{void *state = newState();
	parseFile(state,"1.txt");
	freeState(state);
	}
	_CrtDumpMemoryLeaks();
	return 0;
}