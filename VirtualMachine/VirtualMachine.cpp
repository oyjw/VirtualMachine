#include "VirtualMachine.h"
#include "OpCode.h"
#include "Object.h"
#include <cassert>
#include <fstream>
#include <sstream>

void VirtualMachine::exectue(std::vector<char>& byteCodes){
	decltype(byteCodes.size()) pos;
	while (pos <byteCodes.size()){
		switch (byteCodes[pos++]){
		case PUSHLOCAL:{
			int n = getWord();
			stack.push_back(stack[base + n]);
			top++;
			break;
		}
		case PUSHGLOBAL:{			
			int n = getWord();
			stack.push_back(symTab->getObj(n));
			break;
		}
		case STORELOCAL:{
			int n = getWord();
			stack[base + n] = stack[--top];
			top--;
		}
		case STOREGLOBAL:{
			int n = getWord();
			symTab->putObj(n,stack[--top]);
			break;
		}
		case PUSHREAL:{
			int f = getFloat();
			stack[top].type=NUMOBJ;
			stack[top].value.numval=f;
		}
		case OP_ADD:{
			Object& l = stack[top-2];
			Object& r = stack[top-1];
			if (l.type == NUMOBJ && r.type == NUMOBJ){

			}
			else if (l.type == STROBJ && r.type==NUMOBJ || 
					l.type == NUMOBJ && r.type == STROBJ ||
					l.type==STROBJ && r.type==STROBJ){
			
			}
			break;
		}
		case PUSHPC:{
			int n = getWord();
			Object obj;
			obj.type = NUMOBJ;
			obj.value.numval = n;
			stack.push_back(obj);
			top++;
			break;
		}
		case PUSHBASE:{
			Object obj;
			obj.type=NUMOBJ;
			obj.value.numval=base;
			stack.push_back(obj);
			top++;
			break;
		}	
		case CALLFUNC:{
			int nparam = getWord();
			base = top-nparam;
			break;
		}
		case RETCODE:{
		
			base = stack[base - 1].value.numval;
			pos = stack[base - 2].value.numval;
			stack[base - 2] = stack[top];
			top = base - 1;
		}
		default:assert(0);
		}
	}
}

void VirtualMachine::dump(std::vector<char> &byteCodes, const std::string& fileName){
	std::ofstream ofs(fileName);
	if (!ofs.is_open()){
		std::ostringstream oss;
		oss << "file open fail: " << fileName << std::endl;
		throw std::runtime_error(oss.str());
	}
	decltype(byteCodes.size()) pos=0;
	while (pos <byteCodes.size()){
		switch (byteCodes[pos++]){
		case PUSHLOCAL:{
			int n = getWord();
			ofs << "\tPUSHLOCAL\t" << n <<std::endl;
			break;
		}
		case PUSHGLOBAL:{
			int n = getWord();
			ofs << "\tPUSHGLOBAL\t" << n << std::endl;
			break;
		}
		case STORELOCAL:{
			int n = getWord();
			ofs << "\tSTORELOCAL\t" << n << std::endl;
			break;
		}
		case STOREGLOBAL:{
			int n = getWord();
			ofs << "\tSTOREGLOBAL\t" << n << std::endl;
			break;
		}
		case PUSHREAL:{
			int f = getFloat();
			ofs << "\tPUSHREAL\t" << f << std::endl;
			break;
		}
		case OP_ADD:{
			Object& l = stack[top - 2];
			Object& r = stack[top - 1];
			ofs << "\tOP_ADD\t" << l.value.numval << r.value.numval << std::endl;
			break;
		}
		case PUSHPC:{
			int n = getWord();
			ofs << "\tPUSHPC\t" << n << std::endl;
			break;
		}
		case PUSHBASE:{
			ofs << "\tPUSHPC\t" <<std::endl;
			break;
		}
		case CALLFUNC:{
						  int nparam = getWord();
						  base = top - nparam;
						  break;
		}
		case RETCODE:{

						 base = stack[base - 1].value.numval;
						 pos = stack[base - 2].value.numval;
						 stack[base - 2] = stack[top];
						 top = base - 1;
						 break;
		}
		default:assert(0);
		}
	}
}