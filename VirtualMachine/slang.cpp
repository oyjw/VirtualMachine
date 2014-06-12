#include "slang.h"
#include "VirtualMachine.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "Object.h"
#include "OpCode.h"
#include "SymbolTable.h"
#include "StringPool.h"
#include <iostream>

const char* errorMsg[] = {
	"symbol already exist",
};

const char* strError(int err){
	return errorMsg[err-1];
}

ClsType* defineClass(void* state, char* className){
	return NULL;
}

int defineClassMethod(void* state, ClsType* cls, char* funcName, cFunc func, int nArgs){

	return 0;
}

int defineMethod(void* state, char* funcName, cFunc func, int nArgs){
	VirtualMachine *vm = new VirtualMachine();
	if (vm->symTab->isSymExistLocal(funcName)){
		return SYMBOLEXIST;
	}
	int index = vm->symTab->putSym(funcName);
	Object object;
	object.type = CFUNOBJ;
	object.value.cFunObj = new CFunObj;
	object.value.cFunObj->functionName = funcName;
	object.value.cFunObj->fun = func;
	vm->symTab->putObj(index, object);
	return 0;
}

void* newState(){
	VirtualMachine *vm = new VirtualMachine();
	return (void*)vm;
}

void getArgs(void* state, int *len, Object** objects){
	VirtualMachine *vm = new VirtualMachine();
	vm->getArgs(len, objects);
}

static Object subStr(void* state){
	VirtualMachine *vm = new VirtualMachine();
	int len;
	Object* objs;
	getArgs(state, &len, &objs);


}

static Object toStr(void* state){
	VirtualMachine *vm = (VirtualMachine*)state;
	Object result;
	result.type = STROBJ;
	int len;
	Object* objs;
	getArgs(state, &len, &objs);
	Object& o = objs[0];
	assert(len == 1);
	switch (o.type){
		case NUMOBJ:{
			char buf[100];
			o.value.strObj = vm->stringPoolPtr->putString(std::to_string(o.value.numval));
		}
		case STROBJ:{
			return o;
		}
		default:assert(0);
	}
	return o;
}

static Object strFind(void* state){
	VirtualMachine *vm = new VirtualMachine();
	int len;
	Object* objs;
	getArgs(state, &len, &objs);


}

Object functionCall(void* state, const char* name, int len, Object* object){
	VirtualMachine *vm = (VirtualMachine*)state;
	vm->pushFunObj(name);
	for (int i = 0; i < len; ++i){
		vm->pushObject(object[i]);
	}
	std::vector<char> byteCode;
	byteCode.push_back((char)CALLFUNC);
	byteCode.push_back((char)len);
	vm->execute(byteCode, 0, 0);

	return {NILOBJ,{}};
}

void parseFile(void* state, const char* fileName){
	VirtualMachine *vm = (VirtualMachine*)state;
	Tokenizer* tokenizer = new Tokenizer(fileName);
	std::shared_ptr<Parser> parser = std::make_shared<Parser>(tokenizer, vm->symTab,vm->stringPoolPtr,
	vm->byteCodePtr,vm->objectPoolPtr);
	parser->program();
	
		vm->run();
	
}