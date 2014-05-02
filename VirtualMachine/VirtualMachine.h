#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Object.h"
#include "SymbolTable.h"


class VirtualMachine
{
public:
	VirtualMachine(std::vector<char>& b, SymPtr g) :byteCode(b), symTab(g), top(0) {}
	~VirtualMachine() {
	}
	void execute(std::vector<char> &byteCodes,int base);
	void dump(std::vector<char> &byteCodes, std::ofstream& ofs);
	void run(){
		execute(byteCode,0);
	}
	void run(const std::string& fileName) {
		
		//exectue(byteCode);
		
		std::ofstream ofs(fileName);
		if (!ofs.is_open()){
			std::ostringstream oss;
			oss << "file open fail: " << fileName << std::endl;
			throw std::runtime_error(oss.str());
		}
		dump(byteCode, ofs);
		for (auto& symbol : symTab->getSymbols()){
			if (symbol.obj.type == FUNOBJ){
				
				ofs << symbol.objName << ":" <<std::endl;
				std::vector<char> & v=symbol.obj.value.funObj->bytes;
				for (auto& c : v){
					std::cout << (int)c <<std::endl;
				}
					system("pause");
				dump(symbol.obj.value.funObj->bytes,ofs);
			}
		}
	}
private:
	int getWord(std::vector<char>& byteCode ,int &pos){
		CodeWord code;
		code.c.c1 = byteCode[pos++];
		code.c.c2 = byteCode[pos++];
		return code.word;
	}
	float getFloat(std::vector<char>& byteCode ,int &pos){
		CodeFloat code;
		code.c.c1 = byteCode[pos++];
		code.c.c2 = byteCode[pos++];
		code.c.c3 = byteCode[pos++];
		code.c.c4 = byteCode[pos++];
		return code.f;
	}
	std::vector<char>& byteCode;
	std::vector<Object> stack;
	SymPtr symTab;
	int top;
};

#endif