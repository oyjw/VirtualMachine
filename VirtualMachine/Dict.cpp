#include "Dict.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"
#include "String.h"
#include <sstream>

void checkKeyType(void* state, Object& key, Dict* dict = NULL){
	VirtualMachine *vm = (VirtualMachine*)state;
	if (key.type != NUMOBJ && (!(key.type & USEROBJ) || !(key.type & STROBJ))){
		delete dict;
		vm->throwError("dict key type isn't num or str",TYPEERROR);
	}
}
Object dictNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len = 128;

	Object objArr[128];
	getArgs(state, &len, objArr);
	Object* objs = objArr;
	if (len > 128){
 		vm->throwError("the number of arguments for dict excesses",ARGUMENTERROR);
	}
	Dict* dict = new Dict; 
	assert(len%2 == 0);
	for (int i = 0; i < len; i = i + 2){
		checkKeyType(state, objs[i], dict);
		auto pair = dict->attrs.insert(std::make_pair(objs[i],objs[i+1]));
		if (!pair.second){
			delete dict;
			vm->throwError("dict already has the same key",ATTRERROR);
		}
	}
	Object obj;
	obj.type = USEROBJ | DICTOBJ;
	obj.value.userData = new UserData(vm->dictCls,dict);
	setGC(state, obj.value.userData);
	return obj;
}

Object dictGet(void* state){
	int len = 2;
	Object objs[2];
	getArgs(state, &len, objs);
	assert(len == 2);
	Dict* dict =(Dict*)(objs[0].value.userData->data);
	checkKeyType(state, objs[1]);
	auto iter = (dict->attrs).find(objs[1]);
	if (iter == dict->attrs.end()){
		return NilObj;
	}
	return iter->second;
}

Object dictSet(void* state){
	int len = 3;
	Object objs[3];
	getArgs(state, &len, objs);
	Dict* dict =(Dict*)(objs[0].value.userData->data);
	checkKeyType(state, objs[1]);

	(dict->attrs)[objs[1]] = objs[2];
	return NilObj;
	//auto iter = (dict->attrs).find(objs[1]);
	//if (iter == dict->attrs.end()){
	//	dict->attrs.insert(std::make_pair(objs[1], objs[2]));
	//	delete[] objs;
	//	return NilObj;
	//}
	////Object obj = iter->second;
	//dict->attrs.erase(iter);
	//dict->attrs.insert(std::make_pair(objs[1], objs[2]));
	//delete[] objs;
	//return iter->second;
}

Object dictStr(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len = 1;
	Object obj;
	getArgs(state, &len, &obj);
	Dict* dict =(Dict*)obj.value.userData->data;
	std::ostringstream oss;
	oss << "{";
	auto iter = dict->attrs.begin();
	for (; iter != dict->attrs.end(); ++iter){
		oss << toPrintableStr(state, iter->first,true) << ":" << toPrintableStr(state, iter->second, true);
		auto iter2 = --dict->attrs.end();
		if (iter != iter2)
			oss << ",";
	}
	oss << "}";
	Object ret;
	ret.type = STROBJ;
	ret.value.strObj = vm->addStrObj(oss.str());
	return ret;
}

void dictInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "dict");
	vm->dictCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", dictNew, EVENARG);
	defineClassMethod(state, cls, "[]", dictGet, 2);
	defineClassMethod(state, cls, "[]=", dictSet, 3);
	defineClassMethod(state, cls, "str", dictStr, 1);
}