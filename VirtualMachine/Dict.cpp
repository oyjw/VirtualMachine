#include "Dict.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"
#include "String.h"
#include <sstream>

void checkKeyType(void* state, Object& key){
	VirtualMachine *vm = (VirtualMachine*)state;
	if (key.type != NUMOBJ && (!(key.type & USEROBJ) || !(key.type & STROBJ))){
		vm->throwError("dict key type isn't num or str",TYPEERROR);
	}
}
Object dictNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs;
	getArgs2(state, &len, &objs);
	Dict* dict = new Dict;
	assert(len%2 == 0);
	for (int i = 0; i < len; i = i + 2){
		checkKeyType(state, objs[i]);
		auto pair = dict->attrs.insert(std::make_pair(objs[i],objs[i+1]));
		if (!pair.second){
			vm->throwError("dict already has the same key",ATTRERROR);
		}
	}
	delete []objs;
	Object obj;
	obj.type = USEROBJ | DICTOBJ;
	obj.value.userData = new UserData(vm->dictCls,dict);
	setGC(state, obj.value.userData);
	return obj;
}

Object dictGet(void* state){
	int len;
	Object* objs = NULL;
	getArgs2(state, &len, &objs);
	assert(len == 2);
	Dict* dict =(Dict*)(objs[0].value.userData->data);
	checkKeyType(state, objs[1]);
	auto iter = (dict->attrs).find(objs[1]);
	delete []objs;
	if (iter == dict->attrs.end()){
		return { NILOBJ, {}};
	}
	return iter->second;
}

Object dictSet(void* state){
	int len;
	Object* objs = NULL;
	getArgs2(state, &len, &objs);
	assert(len == 3);
	Dict* dict =(Dict*)(objs[0].value.userData->data);
	checkKeyType(state, objs[1]);

	(dict->attrs)[objs[1]] = objs[2];
	delete[] objs;
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