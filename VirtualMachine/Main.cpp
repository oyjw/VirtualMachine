#include "slang.h"

int main(){
	void *state = newState();
	parseFile(state,"1.txt");
	return 0;
}