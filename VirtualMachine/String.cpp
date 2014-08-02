#include "String.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"

static Object toStr(void* state){
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
	Object& o = objs[0];
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
	Object result = o;
	//delete[] objs;
	return result;
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