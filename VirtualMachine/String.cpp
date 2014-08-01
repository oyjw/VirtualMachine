#include "String.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"

static Object toStr(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	Object& o = objs[0];
	assert(len == 1);
	switch (o.type){
		case NUMOBJ:{
			char buf[100];
			o.value.strObj = vm->addStrObj(std::to_string(o.value.numval));
		}
		case STROBJ:{
			return o;
		}
		default:assert(0);
	}
	return o;
}

Object strNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	Object obj;
	obj.type = USEROBJ | STROBJ;
	obj.value.userData = new UserData(vm->strCls,toStr(state).value.strObj);
	setGC2(state, obj.value.userData);
	return obj;
}


void strInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "str");
	vm->strCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", strNew, 1);
}