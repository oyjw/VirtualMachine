#include "String.h"
#include "VirtualMachine.h"
#include "slang.h"
#include "Object.h"
#include <string>

std::string toPrintableStr(void* state,const Object& o,bool addQuote){
	VirtualMachine *vm = (VirtualMachine*)state;
	std::string ret;
	switch (o.type){
		case NILOBJ:{
			ret = "None";
			break;
		}
		case BOOLOBJ:{
			ret =  o.value.boolval? "True": "False";
			break;
		}
		case NUMOBJ:{
			bool isInteger = (o.value.numval == (int)o.value.numval)? true: false;
			std::string s;
			if (isInteger)
				ret = std::to_string((int)o.value.numval);
			else
				ret = std::to_string(o.value.numval);
			break;
		}
		case STROBJ: {
			if (addQuote)
				ret = "\"" + o.value.strObj->str + "\"";
			else 
				ret = o.value.strObj->str;
			break;
		}
		case FUNOBJ:{
			ret = "function " + o.value.funObj->functionName;
		}
		case CFUNOBJ:{
			ret = "cFunction " + o.value.cFunObj->functionName;
		}
		case CLSTYPE:
		case USERTYPE:{
			ret = "class " + o.value.clsType->clsName;
		}
		case CLSOBJ:{
			ret = "object " + o.value.clsObj->clsType->clsName;			 
		}
		default:{
			if (o.type & METHOD){
				ret = "method " + o.value.method.self->clsType->clsName + "." + 
					(o.type & FUNOBJ ? o.value.method.funObj->functionName:
						o.value.method.cFunObj->functionName);
			}
			else if (o.type & USEROBJ){
				if (o.type & STROBJ){
					if (addQuote)
						ret = "\"" + ((StrObj*)o.value.userData->data)->str + "\"";
					else 
						ret = ((StrObj*)o.value.userData->data)->str;
				}
				else {
					size_t fp = vm->getFP();
					int top = vm->getTop();
					vm->pushObject(o);
					vm->setFP(top);
					std::string& str = vm->callCFunc(o.value.userData->type, "str", vm->getFP()).value.strObj->str;
					vm->setFP(fp);
					vm->setTop(top);
					return str;
				}
			}
			else assert(0);
		}
	}
	return ret;
}

Object toStr(void* state, const Object& o){
	VirtualMachine *vm = (VirtualMachine*)state;
	Object result;
	result.type = STROBJ;
	switch (o.type){
		case NILOBJ:{
			result.value.strObj = vm->getStrObj("None");
			break;
		}
		case BOOLOBJ:{
			result.value.strObj = o.value.boolval? vm->getStrObj("True"): vm->getStrObj("False");;
			break;
		}
		case NUMOBJ:{
			bool isInteger = (o.value.numval == (int)o.value.numval)? true: false;
			std::string s;
			if (isInteger)
				s = std::to_string((int)o.value.numval);
			else
				s = std::to_string(o.value.numval);
			result.value.strObj = vm->addStrObj(s);
			break;
		}
		case STROBJ: {
			return o;
		}
		case FUNOBJ:{
			result.value.strObj = vm->addStrObj("function " + o.value.funObj->functionName);
		}
		case CFUNOBJ:{
			result.value.strObj = vm->addStrObj("cFunction " + o.value.cFunObj->functionName);
		}
		case CLSTYPE:
		case USERTYPE:{
			result.value.strObj = vm->addStrObj("class " + o.value.clsType->clsName);
		}
		case CLSOBJ:{
			result.value.strObj = vm->addStrObj("object " + o.value.clsObj->clsType->clsName);			 
		}
		default:{
			if (o.type & METHOD){
				result.value.strObj = vm->addStrObj("method " + o.value.method.self->clsType->clsName + "." + 
					(o.type & FUNOBJ ? o.value.method.funObj->functionName:
						o.value.method.cFunObj->functionName));
			}
			else if (o.type & USEROBJ){
				if (o.type & STROBJ){
					return o;
				}
				return vm->callCFunc(o.value.userData->type, "str", vm->getFP());
			}
			else assert(0);
		}
	}
	return result;
}

static size_t checkIndex(void* state, StrObj* strObj, Object& obj){
	VirtualMachine *vm = (VirtualMachine*)state;
	if (obj.type != NUMOBJ){
		vm->throwError("list index type isn't num",TYPEERROR);
	}
	float index = obj.value.numval;
	if (index != (int)index){
		vm->throwError("index isn't int",ARGUMENTERROR);
	}
	if (index < 0 || (size_t)index >= strObj->str.size()){
		vm->throwError("index out of range",ARGUMENTERROR);
	}
	return (size_t)index;
}

Object strNew(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len = 1;
	Object arg;
	getArgs(state, &len, &arg);
	Object obj;
	arg = toStr(state, arg);
	if (arg.type == STROBJ){
		obj.type = USEROBJ | STROBJ;
		obj.value.userData = new UserData(vm->strCls,arg.value.strObj);
		setGC2(state, obj.value.userData);
	}
	else
		obj = arg;
	return obj;
}

Object strGet(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object* objs = NULL;
	getArgs2(state, &len, &objs);
	StrObj* strObj = (StrObj*)(objs[0].value.userData->data);
	size_t index = checkIndex(state, strObj,objs[1]);
	Object obj;
	obj.value.userData = new UserData(vm->strCls, vm->addStrObj(&(strObj->str[(size_t)index])));

	delete[] objs;
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
	size_t index = checkIndex(state, strObj,objs[1]);
	strObj->str[index] = value->str[0];
	delete[] objs;
	return NilObj;
}

Object strLen(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	int len;
	Object obj;
	getArgs(state, &len, &obj);
	StrObj* strObj = (StrObj*)(obj.value.userData->data);
	Object ret;
	ret.type = NUMOBJ;
	ret.value.numval = (float)strObj->str.size();
	return ret;
}

//static Object strFind(void* state){
//	VirtualMachine *vm = (VirtualMachine*)state;
//}
void strInit(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	ClsType* cls = defineClass(state, "str");
	vm->strCls = cls;
	assert(cls != NULL);
	defineClassMethod(state, cls, "constructor", strNew, 1);
	defineClassMethod(state, cls, "[]", strGet, 2);
	defineClassMethod(state, cls, "[]=", strSet, 3);
	defineClassMethod(state, cls, "len", strLen, 1);
}