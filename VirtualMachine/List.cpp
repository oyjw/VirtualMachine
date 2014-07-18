#include "List.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"

struct list{
	std::vector<Object> vec;
};

Object listNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	list* l = new list;
	for (int i = 0; i < len; ++i)
		l->vec.push_back(objs[i]);
	Object obj;
	obj.type = USERDATA | CLSOBJ;
	obj.value.clsObj.clsType = vm->listCls;
	obj.value.clsObj.data = (void*)l;
	return obj;
}

void listInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "list");
	vm->listCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", listNew, ANY);
}