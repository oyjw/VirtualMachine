#include "slang.h"
#include "VirtualMachine.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "Object.h"
#include "OpCode.h"
#include "SymbolTable.h"
#include "StringPool.h"
#include "ObjectPool.h"
#include "List.h"
#include "String.h"

const char* errorMsg[] = {
	"symbol already exist",
};

const char* strError(int err){
	return errorMsg[err-1];
}

ClsType* defineClass(void* state, char* className){
	VirtualMachine *vm = (VirtualMachine*)state;
	if (vm->symTab->isSymExistLocal(className))
		return NULL;
	int index = vm->symTab->putSym(className);
	Object obj;
	obj.type = USERTYPE;
	ClsType* clsType = new ClsType;
	clsType->clsName = className;
	obj.value.clsType = clsType;
	vm->symTab->putObj(index, obj);
	return clsType;
}

int defineClassMethod(void* state, void* cls, char* funcName, cFunc func, int nArgs){
	VirtualMachine *vm = (VirtualMachine*)state;
	int index = vm->stringPoolPtr->getStringConstant(funcName);
	if (index == -1){
		index = vm->stringPoolPtr->putStringConstant(funcName);
	}
	StrObj* pStrObj = vm->stringPoolPtr->getStrObj(index);
	ClsType* clsType = (ClsType*)cls;
	Object obj;
	obj.type = CFUNOBJ;
	CFunObj *pCFunObj = new CFunObj;
	pCFunObj->functionName = funcName;
	pCFunObj->fun = func;
	pCFunObj->nArgs = nArgs;
	obj.value.cFunObj = pCFunObj;
	auto pair = clsType->clsAttrs.insert(std::make_pair(pStrObj,obj));
	if (!pair.second){
		return -1;
	}
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
	listInit((void*)vm);
	strInit(vm);
	return (void*)vm;
}

void freeState(void* state){
	delete (VirtualMachine*)state;
}

void getArgs(void* state, int *len, Object* objects){
	VirtualMachine *vm = (VirtualMachine*)state;
	vm->getArgs(len, objects);
}

void getArgs2(void* state, int *len, Object** objects){
	VirtualMachine *vm = (VirtualMachine*)state;
	vm->getArgs2(len, objects);
}

void setGC2(void* state, UserData *userData){
	VirtualMachine *vm = (VirtualMachine*)state;
	vm->objectPoolPtr->putUserData(userData, true);
}

void setGC(void* state, UserData *userData){
	VirtualMachine *vm = (VirtualMachine*)state;
	vm->objectPoolPtr->putUserData(userData, false);
}

static Object subStr(void* state){
	VirtualMachine *vm = new VirtualMachine();


}


static Object strFind(void* state){
	VirtualMachine *vm = new VirtualMachine();


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