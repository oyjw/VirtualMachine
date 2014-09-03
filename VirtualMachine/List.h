#ifndef _LIST_H_
#define _LIST_H_
#include <vector>
#include "Object.h"
struct List{
	std::vector<Object> vec;
};
void listInit(void* state);
#endif