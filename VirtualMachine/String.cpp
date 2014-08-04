#include "String.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"
#include <string>
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

Object strGet(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs = NULL;
	getArgs2(state, &len, &objs);
	StrObj* strObj = (StrObj*)(objs[0].value.userData->data);
	float index = objs[1].value.numval;
	if (index < 0 || (size_t)index >= strObj->str.size()){
		vm->throwError("index out of range",ARGUMENTERROR);
	}
	Object obj;
	obj.value.userData = new UserData(vm->strCls, vm->addStrObj2(&(strObj->str[(size_t)index])));
	return obj;
}

Object strSet(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs = NULL;
	getArgs2(state, &len, &objs);
	StrObj* strObj = (StrObj*)(objs[0].value.userData->data);
	StrObj* value = (StrObj*)(objs[2].value.userData->data);
	if (value->str.size() != (size_t)1)
		vm->throwError("the size of string to replace is not 1",ARGUMENTERROR);
	float index = objs[1].value.numval;
	if (index < 0 || (size_t)index >= strObj->str.size()){
		vm->throwError("index out of range",ARGUMENTERROR);
	}
	Object obj;
	strObj->str[(size_t)index] = value->str[0];

	return {NILOBJ,{}};
}

void strInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "str");
	vm->strCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", strNew, 1);
	defineClassMethod(state, cls, "[]", strGet, 2);
	defineClassMethod(state, cls, "[]=", strSet, 3);
}