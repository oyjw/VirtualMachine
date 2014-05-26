#include "slang.h"
#include "VirtualMachine.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "Object.h"
#include "OpCode.h"

void* newstate(){
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
	vm->execute(byteCode,0);

	return {NILOBJ,{}};
}

void parseFile(void* state, const char* fileName){
	VirtualMachine *vm = (VirtualMachine*)state;
	Tokenizer* tokenizer = new Tokenizer(fileName);
	std::shared_ptr<Parser> parser = std::make_shared<Parser>(tokenizer, vm->symTab,vm->stringPoolPtr,vm->byteCodePtr,vm->objectPoolPtr);
	parser->program();
	vm->run();
}