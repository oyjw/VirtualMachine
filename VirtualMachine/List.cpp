#include "List.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"

struct list{
	std::vector<Object> vec;
	ClsType* type;
};

Object listNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	list* l = new list;
	for (int i = 0; i < len; ++i)
		l->vec.push_back(objs[i]);
	l->type = vm->listCls;
	Object obj;
	obj.type = USERTYPE;
	obj.value.userData = (void*)l;
	return obj;
}

void listInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "list");
	vm->listCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", listNew, ANY);
}