#include "VirtualMachine.h"
#include "OpCode.h"
#include "Object.h"
#include "Exceptions.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>

void VirtualMachine::execute(std::vector<char>& byteCodes,int base){
	int pos = 0;
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
			int f = getFloat(byteCodes,pos);
			Object obj;
			obj.type=NUMOBJ;
			obj.value.numval=f;
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
			int n = getWord(byteCodes,pos);
			Object obj={NILOBJ,0};
			for(int i=0;i<n;++i){
				stack.push_back(obj);
			}		
			break;
		}
		case CALLFUNC:{
			int nargs = byteCodes[pos++];
			Object &obj = stack[ top - nargs - 1 ];
			int newBase = top - nargs;
			assert(obj.type == FUNOBJ);
			execute(obj.value.funObj->bytes,newBase);
			top = newBase - 1;
			stack.resize(top);
			break;
		}
		case RETCODE:{
			stack[base - 1] = stack[top-1];
			top=base;
			stack.resize(top);
			break;
		}
		case PRINTFUNC:{
			int n = getWord(byteCodes,pos);
			Object& obj = symTab->getObj(n);
			if (obj.type == NUMOBJ)
				std::cout << symTab->getObj(n).value.numval <<std::endl;
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

void VirtualMachine::dump(std::vector<char> &byteCodes,std::ofstream& ofs){
	int pos = 0;
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
			int f = getFloat(byteCodes,pos);
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