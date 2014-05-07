#ifndef _OBJECT_H_
#define _OBJECT_H_
#include <string>
#include <vector>

enum ObjectType{
	NUMOBJ, STROBJ, FUNOBJ,BOOLOBJ,NILOBJ
};

struct CollectableObject{
};




class StrObj {
public:
	std::string str;
	StrObj(std::string s) :str(s) {}
};

class FunObj {
public:
	std::vector<char> bytes;
	int nargs;
	FunObj() {}
};

struct Object{
	ObjectType type;
	union{
		bool boolval;
		float numval;
		StrObj* strObj;
		FunObj* funObj;
	} value;
};

struct Symbol{
	Object obj;
	std::string objName;
	//Symbol(Object* o, std::string s) :obj(o), objName(s) {}
};

union CodeWord{
	short word;
	struct{
		char c1;
		char c2;
	}c;
};

union CodeFloat{
	float f;
	struct{
		char c1;
		char c2;
		char c3;
		char c4;
	}c;
};





#endif