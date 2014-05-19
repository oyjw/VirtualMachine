#include "slang.h"

int main(){
	void *state = newstate();
	parseFile(state,"1.txt");
	return 0;
}