#include "slang.h"
#include "VirtualMachine.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "Object.h"
#include "OpCode.h"
#include "SymbolTable.h"

struct Book
{
     char bname[20];
     int pages;
     char author[20];
     float price; 
}b1[3] = {
           {},
           {"Book2",500,"AAK",350.00},
           {"Book3",120,"HST",450.00}
         };

struct ErrorMsg
{
	int err; 
	char* msg; 
} errorMsg[] = {
	{SYMBOLEXIST, "symbol already exist"}
};

const char* strError(int err){
	return errorMsg[err-1].msg;
}

ClsType* defineClass(void* state, char* className){
	return NULL;
}

int defineMethod(void* state, char* funcName, cFunc func){
	VirtualMachine *vm = new VirtualMachine();
	if (vm->symTab->isSymExistLocal(funcName)){
		return SYMBOLEXIST;
	}
	int index = vm->symTab->putSym(funcName);
	Object object;
	object.type = CFUNOBJ;
	object.value.cFunObj = func;
	vm->symTab->putObj(index, object);
	return 0;
}

void* newState(){
	VirtualMachine *vm = new VirtualMachine();
	return (void*)vm;
}

int getargs(void* state, int *len, Object** objects){
	return 0;
}

Object funcall(void* state, const char* name, int len, Object* object){
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
	std::shared_ptr<Parser> parser = std::make_shared<Parser>(tokenizer, vm->symTab,vm->stringPoolPtr,vm->byteCodePtr,vm->objectPoolPtr);
	parser->program();
	vm->run();
}