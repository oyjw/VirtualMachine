#include "List.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"



Object listNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len = 10;
	Object objArr[10];
	getArgs(state, &len, objArr);
	Object* objs = objArr;
	Object *objs2;
	if (len != 10){
		objs2 = new Object[len];
		getArgs(state, &len, objs2);
		objs = objs2;
	}
	List* l = new List;
	for (int i = 0; i < len; ++i)
		l->vec.push_back(objs[i]);
	Object obj;
	//delete []objs;
	obj.type = USEROBJ | LISTOBJ;
	obj.value.userData = new UserData(vm->listCls,l);
	setGC(state, obj.value.userData);
	return obj;
}

Object listGet(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len = 10;
	Object objArr[10];
	getArgs(state, &len, objArr);
	Object* objs = objArr;
	Object *objs2;
	if (len != 10){
		objs2 = new Object[len];
		getArgs(state, &len, objs2);
		objs = objs2;
	}
	List* l =(List*)objs[0].value.userData->data;
	float index = objs[1].value.numval;
	if (index < 0 || (size_t)index >= l->vec.size()){
		vm->throwError("index out of range",ARGUMENTERROR);
	}
	return l->vec[(size_t)index];
}

Object listSet(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len = 10;
	Object objArr[10];
	getArgs(state, &len, objArr);
	Object* objs = objArr;
	Object *objs2;
	if (len != 10){
		objs2 = new Object[len];
		getArgs(state, &len, objs2);
		objs = objs2;
	}
	List* l =(List*)objs[0].value.userData->data;
	float index = objs[1].value.numval;
	if (index < 0 || (size_t)index >= l->vec.size()){
		vm->throwError("index out of range",ARGUMENTERROR);
	}
	l->vec[(size_t)index] = objs[2];
	return {NILOBJ,{}};
}

void listInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "list");
	vm->listCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", listNew, ANYARG);
	defineClassMethod(state, cls, "[]", listGet, 2);
	defineClassMethod(state, cls, "[]=", listSet, 3);
}