#include <functional>
#include <iostream>
#include <fstream>
#include <memory>
#include "VirtualMachine.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "SymbolTable.h"
bool readline(std::string& str){
	return std::getline(std::cin, str) ? true : false;
}




int main(){
	Tokenizer* tokenizer = new Tokenizer("1.txt");
	SymPtr globalSymTab = std::make_shared<SymbolTable>();
	std::shared_ptr<Parser> parser = std::make_shared<Parser>(tokenizer, globalSymTab);
	parser->program();
	std::shared_ptr<VirtualMachine> vm = std::make_shared< VirtualMachine>(std::move(parser->getByteCodePtr()->v), globalSymTab);
	vm->run("2.txt");
	return 0;
}