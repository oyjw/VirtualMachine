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
	obj.type = USEROBJ | LISTOBJ;
	obj.value.userData = (void*)l;
	return obj;
}

Object listGet(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	assert(len == 2);
	list* l =(list*)objs[0].value.userData;
	return obj;
}

void listInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "list");
	vm->listCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", listNew, ANYARG);
	defineClassMethod(state, cls, "[]", listGet, 2);
}