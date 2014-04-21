#include "VirtualMachine.h"
#include "OpCode.h"
#include "Object.h"
#include <cassert>

void VirtualMachine::exectue(){
	char code = byteCode[pos++];
	switch (code){
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
		symTab->putObj(stack[--top]);
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


		base = stack[base-1].value.numval;
		pos = stack[base-2].value.numval;
		stack[base - 2] = stack[top];
		top = base - 1;
	}
	default:assert(0);
}