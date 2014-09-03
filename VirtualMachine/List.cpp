#include "List.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"
#include "String.h"
#include <sstream>

static size_t checkIndex(void* state, List* list, Object& obj){
	VirtualMachine *vm = (VirtualMachine*)state;
	if (obj.type != NUMOBJ){
		vm->throwError("list index type isn't num",TYPEERROR);
	}
	float index = obj.value.numval;
	if (index != (int)index){
		vm->throwError("index isn't int",ARGUMENTERROR);
	}
	if (index < 0 || (size_t)index >= list->vec.size()){
		vm->throwError("index out of range",ARGUMENTERROR);
	}
	return (size_t)index;
}

Object listNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	/*int len = 10;
	Object objArr[10];
	getArgs(state, &len, objArr);
	Object* objs = objArr;
	Object *objs2;
	if (len != 10){
		objs2 = new Object[len];
		getArgs(state, &len, objs2);
		objs = objs2;
	}*/
	int len;
	Object* objs = NULL;
	getArgs2(state, &len, &objs);
	List* l = new List;
	for (int i = 0; i < len; ++i)
		l->vec.push_back(objs[i]);
	Object obj;
	delete []objs;
	obj.type = USEROBJ | LISTOBJ;
	obj.value.userData = new UserData(vm->listCls,l);
	setGC(state, obj.value.userData);
	return obj;
}

Object listGet(void* state){
	int len = 2;
	Object objArr[2];
	getArgs(state, &len, objArr);
	Object* objs = objArr;
	List* l =(List*)objs[0].value.userData->data;
	size_t index = checkIndex(state, l, objs[1]);
	return l->vec[(size_t)index];
}

Object listSet(void* state){
	int len = 3;
	Object objArr[3];
	getArgs(state, &len, objArr);
	Object* objs = objArr;
	List* l =(List*)objs[0].value.userData->data;
	size_t index = checkIndex(state, l, objs[1]);
	
	l->vec[index] = objs[2];
	return {NILOBJ,{}};
}

Object listStr(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len = 1;
	Object obj;
	getArgs(state, &len, &obj);
	List* l =(List*)obj.value.userData->data;
	std::ostringstream oss;
	oss << "[";
	for (size_t i = 0; i < l->vec.size(); ++i){
		oss << toPrintableStr(state, l->vec[i],true);
		if (i != l->vec.size()-1)
			oss << ",";
	}
	oss << "]";
	Object ret;
	ret.type = STROBJ;
	ret.value.strObj = vm->addStrObj(oss.str());
	return ret;
}

Object listLen(void* state){
	int len = 1;
	Object obj;
	getArgs(state, &len, &obj);
	List* l =(List*)obj.value.userData->data;
	Object ret;
	ret.type = NUMOBJ;
	ret.value.numval = (float)l->vec.size();
	return ret;
}

Object listAppend(void* state){
	int len;
	Object *objs = NULL;
	getArgs2(state, &len, &objs);
	List* l =(List*)objs[0].value.userData->data;
	for (int i = 1; i < len; ++i){
		l->vec.push_back(objs[i]);
	}
	delete[]objs;
	return NilObj;
}

void listInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "list");
	vm->listCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", listNew, ANYARG);
	defineClassMethod(state, cls, "[]", listGet, 2);
	defineClassMethod(state, cls, "[]=", listSet, 3);
	defineClassMethod(state, cls, "str", listStr, 1);
	defineClassMethod(state, cls, "len", listLen, 1);
	defineClassMethod(state, cls, "append", listAppend, ANYARG);
}