#include "VirtualMachine.h"
#include "OpCode.h"
#include "Object.h"
#include "Exceptions.h"
#include "SymbolTable.h"
#include "StringPool.h"
#include "slang.h"
#include "Tokenizer.h"
#include "Parser.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>


VirtualMachine::VirtualMachine():threshold(100),top(0),symTab(new SymbolTable),byteCodePtr(new ByteCode),stringPoolPtr(new StringPool) {}

void VirtualMachine::execute(std::vector<char>& byteCodes,int base){
	size_t pos = 0;
	while (pos <byteCodes.size()){
		switch (byteCodes[pos++]){
		case PUSHLOCAL:{
			int n = getWord(byteCodes,pos);
			stack.push_back(stack[base + n]);
			top++;
			break;
		}
		case PUSHGLOBAL:{			
			int n = getWord(byteCodes,pos);
			stack.push_back(symTab->getObj(n));
			top++;
			break;
		}
		case STORELOCAL:{
			int n = getWord(byteCodes,pos);
			stack[base + n] = stack[top-1];
			top--;
			stack.resize(top);
			break;
		}
		case STOREGLOBAL:{
			int n = getWord(byteCodes,pos);
			symTab->putObj(n,stack[top-1]);
			top--;
			stack.resize(top);
			break;
		}
		case PUSHREAL:{
			float f = getFloat(byteCodes,pos);
			Object obj;
			obj.type=NUMOBJ;
			obj.value.numval=f;
			stack.push_back(obj);
			top++;
			break;
		}
		case PUSHSTRING:{
			int index = getWord(byteCodes,pos);
			Object obj;
			obj.type = STROBJ;
			obj.value.strObj = stringPoolPtr->getStrObj(index);
			stack.push_back(obj);
			top++;
			break;
		}
		case OP_ADD:{
			Object& l = stack[top-2];
			Object& r = stack[top-1];
			if (l.type == NUMOBJ && r.type == NUMOBJ){
				float result=l.value.numval+r.value.numval;
				l.value.numval=result;
			}
			else if (l.type == STROBJ && r.type==NUMOBJ || 
					l.type == NUMOBJ && r.type == STROBJ ||
					l.type==STROBJ && r.type==STROBJ){
				nobjs++;
				collect();
				std::string leftStr;
				std::string rightStr;
				if( l.type == NUMOBJ ){
					char buf[1024];
					sprintf_s(buf, "%f",l.value.numval);
					leftStr = buf;
				}
				else 
					leftStr = l.value.strObj->str;
				if( r.type == NUMOBJ ){
					char buf[1024];
					sprintf_s(buf, "%f",r.value.numval);
					rightStr = buf;
				}

				else 
					rightStr = l.value.strObj->str;
				int index = stringPoolPtr->putString(leftStr+rightStr);
				l.type = STROBJ;
				l.value.strObj = stringPoolPtr->getStrObj(index);
			}
			else{
				assert(0);
			}
			top--;
			stack.resize(top);
			break;
		}
		case OP_SUB:{
			compute(OP_SUB);
			break;
		}
		case OP_MUL:{
			compute(OP_MUL);
			break;
		}
		case OP_DIV:{
			compute(OP_DIV);
			break;
		}
		case OP_EQ:{
			compare(OP_EQ);
			break;
		}
		case OP_NOTEQ:{
			compare(OP_NOTEQ);
			break;
		}
		case OP_LT:{		
			compare(OP_LT);
			break;
		}
		case OP_GT:{
			compare(OP_GT);
			break;
		}
		case OP_LE:{
			compare(OP_LE);
			break;
		}
		case OP_GE:{
			compare(OP_GE);
			break;
		}
		case OP_NOT:{
			Object& obj=stack[top-1];
			int steps = getWord(byteCodes,pos);
			if (obj.type == NUMOBJ && obj.value.numval==0  ||  obj.type==NILOBJ  ||
				obj.type == BOOLOBJ && obj.value.boolval==false){
				obj.value.boolval = true;
			}		
			else {
				obj.value.boolval = false;
			}
			obj.type = BOOLOBJ;	
			break;
		}
		case JMP:{
			int steps = getWord(byteCodes,pos);
			pos+=steps;		
			break;
		}
		case JMPIFF:{
			Object& obj=stack[top-1];
			int steps = getWord(byteCodes,pos);
			if (obj.type == NUMOBJ && obj.value.numval==0  ||  obj.type==NILOBJ  ||
				obj.type == BOOLOBJ && obj.value.boolval==false){
				pos+=steps;	
			}		
			top--;
			stack.resize(top);
			break;
		}
		case ADJUST:{
			int nDecls = byteCodes[pos++];
			top = top+nDecls;
			if (nDecls > 0){
				Object obj={NILOBJ,0};
				for(int i=0;i<nDecls;++i){
					stack.push_back(obj);
				}		
			}
			else if (nDecls < 0){
				stack.resize(top);
			}
			break;
		}
		case CALLFUNC:{
			int nargs = byteCodes[pos++];
			Object &obj = stack[ top - nargs - 1 ];
			int newBase = top - nargs;
			assert(obj.type == FUNOBJ);
			execute(obj.value.funObj->bytes,newBase);
			/*top = newBase - 1;
			stack.resize(top);*/
			break;
		}
		case RETCODE:{
			stack[base - 1] = stack[top-1];
			top=base;
			stack.resize(top);
			return ;
		}
		case PRINTFUNC:{
			Object& obj = stack[top-1];
			if (obj.type == NUMOBJ)
				std::cout << (int)obj.value.numval <<std::endl;
			else if (obj.type == STROBJ)
				std::cout << obj.value.strObj->str <<std::endl;
			top--;
			stack.resize(top);
			system("pause");
			break;
		}
		default:assert(0);
		}
	}
}

void VirtualMachine::compute(int opcode){
	Object& l = stack[top - 2];
	Object& r = stack[top - 1];
	if (l.type != NUMOBJ || r.type != NUMOBJ){
		std::ostringstream oss;
		throw TypeError(oss.str());
	}
	float result;
	switch (opcode){
		case OP_SUB:result=l.value.numval - r.value.numval; break;
		case OP_MUL:result=l.value.numval * r.value.numval; break;
		case OP_DIV:result=l.value.numval / r.value.numval; break;
		default:assert(0);
	}
	l.value.numval=result;
	top--;
	stack.resize(top);
}

void VirtualMachine::compare(int opcode){
	Object& l = stack[top - 2];
	Object& r = stack[top - 1];
	if (l.type != NUMOBJ || r.type != NUMOBJ){
		std::ostringstream oss;
		throw TypeError(oss.str());
	}
	bool b;
	switch (opcode){
		case OP_EQ:b=l.value.numval == r.value.numval; break;
		case OP_NOTEQ:b=l.value.numval != r.value.numval; break;
		case OP_LT:b=l.value.numval < r.value.numval; break;
		case OP_GT:b=l.value.numval > r.value.numval; break;
		case OP_LE:b=l.value.numval <= r.value.numval; break;
		case OP_GE:b=l.value.numval >= r.value.numval; break;
		default:assert(0);
	}
	l.type = BOOLOBJ;
	l.value.boolval=b;
	top--;
	stack.resize(top);
}

void VirtualMachine::collect(){
	if (nobjs > threshold){
		stringPoolPtr->collect();
		nobjs = stringPoolPtr->getStrNum();
		threshold = nobjs*2;
	}
}

void VirtualMachine::dump(std::vector<char> &byteCodes,std::ofstream& ofs){
	size_t pos = 0;
	while (pos <byteCodes.size()){
		switch (byteCodes[pos++]){
		case PUSHLOCAL:{
			ofs << pos-1 ;
			int n = getWord(byteCodes,pos);
			ofs << "\tPUSHLOCAL\t" << n <<std::endl;
			break;
		}
		case PUSHGLOBAL:{
			ofs << pos-1;
			int n = getWord(byteCodes,pos);
			ofs << "\tPUSHGLOBAL\t" << n << std::endl;
			break;
		}
		case STORELOCAL:{
			ofs << pos-1;
			int n = getWord(byteCodes,pos);
			ofs << "\tSTORELOCAL\t" << n << std::endl;
			break;
		}
		case STOREGLOBAL:{
			ofs << pos-1;
			int n = getWord(byteCodes,pos);
			ofs << "\tSTOREGLOBAL\t" << n << std::endl;
			break;
		}
		case PUSHREAL:{
			ofs << pos-1;
			float f = getFloat(byteCodes,pos);
			ofs << "\tPUSHREAL\t" << f << std::endl;
			break;
		}
		case OP_ADD:{
			ofs << pos-1 << "\tOP_ADD\t" << std::endl;
			break;
		}
		case OP_SUB:{
			ofs << pos-1 << "\tOP_SUB\t" << std::endl;
			break;
		}
		case OP_MUL:{
			ofs << pos-1 << "\tOP_MUL\t" << std::endl;
			break;
		}

		case OP_DIV:{
			ofs << pos-1 << "\tOP_DIV\t" << std::endl;
			break;
		}
		case OP_EQ:{
			ofs << pos-1 << "\tOP_EQ\t"  <<std::endl;
			break;
		}
		case OP_NOTEQ:{
			ofs << pos-1 << "\tOP_NOTEQ\t"  <<std::endl;
			break;
		}
		case OP_LT:{		
			ofs << pos-1 << "\tOP_LT\t"  <<std::endl;
			break;
		}
		case OP_GT:{
			ofs << pos-1 << "\tOP_GT\t"  <<std::endl;
			break;
		}
		case OP_LE:{
			ofs << pos-1 << "\tOP_LE\t"  <<std::endl;
			break;
		}
		case OP_GE:{
			ofs << pos-1 << "\tOP_GE\t"  <<std::endl;
			break;
		}
		case OP_NOT:{
			ofs << pos-1 << "\tOP_NOT\t"  <<std::endl;	
			break;
		}
		case JMP:{
			ofs << pos-1;
			int steps = getWord(byteCodes,pos);
			ofs << "\tJMP\t" << steps <<std::endl;			
			break;
		}
		case JMPIFF:{
			ofs << pos-1;
			int steps = getWord(byteCodes,pos);
			ofs << "\tJMPIFF\t" << steps <<std::endl;			
			break;
		}
		case ADJUST:{
			ofs << pos-1;
			int nDecls = byteCodes[pos++];
			ofs << "\tADJUST\t" << nDecls <<std::endl;			
			break;
		}
		case CALLFUNC:{
			ofs << pos-1;
			int nargs = byteCodes[pos++];
			ofs << "\tCALLFUNC\t" << nargs <<std::endl;			
			break;
		}
		case RETCODE:{
			ofs << pos-1 << "\tRETCODE\t" <<std::endl;
			break;
		}
		case PRINTFUNC:{
			ofs << pos-1 << "\tPRINT\t"  << std::endl;
			break;
		}
		default:assert(0);
		}
	}
}

void VirtualMachine::run(){
	execute(byteCodePtr->v,0);
}

void VirtualMachine::run(const std::string& fileName) {
	std::ofstream ofs(fileName);
	if (!ofs.is_open()){
		std::ostringstream oss;
		oss << "file open fail: " << fileName << std::endl;
		throw std::runtime_error(oss.str());
	}
	dump(byteCodePtr->v, ofs);
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

int VirtualMachine::getWord(std::vector<char>& byteCode ,size_t &pos){
	CodeWord code;
	code.c.c1 = byteCode[pos++];
	code.c.c2 = byteCode[pos++];
	return code.word;
}

float VirtualMachine::getFloat(std::vector<char>& byteCode ,size_t &pos){
	CodeFloat code;
	code.c.c1 = byteCode[pos++];
	code.c.c2 = byteCode[pos++];
	code.c.c3 = byteCode[pos++];
	code.c.c4 = byteCode[pos++];
	return code.f;
}



bool VirtualMachine::pushFunObj(const std::string& symbol){
	auto p = symTab->findSym(symbol);
	if (!p.first){
		return false;
	}
	stack.push_back(symTab->getObj(p.second));
}