#ifndef _OBJECT_H_
#define _OBJECT_H_
enum ObjectType{
	NUMOBJ, STROBJ, FUNOBJ,MARKOBJ,NILOBJ
};

struct Object{
	ObjectType type;
	union{
		float numval;
		char*  strval;
	} value;
};

struct Symbol{
	Object obj;
	std::string objName;
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