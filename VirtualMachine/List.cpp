#include "List.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"

struct list{
	std::vector<Object> vec;
};

Object listNew(void* state){
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	list* l = new list;
	for (int i = 0; i < len; ++i)
		l->vec.push_back(objs[i]);
	Object obj;
	obj.type = USERDATA;
		
	return obj;
}

void listInit(void* state){
	defineClass(state, "list");
}