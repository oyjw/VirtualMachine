#ifndef _OBJECT_H_
#define _OBJECT_H_
#include <string>
#include <vector>

enum ObjectType{
	NUMOBJ, STROBJ, FUNOBJ,MARKOBJ,NILOBJ
};

struct CollectableObject{
};

struct Object{
	ObjectType type;
	union{
		float numval;
		CollectableObject* collObj;
	} value;
};



class StrObj :public CollectableObject{
public:
	std::string str;
	StrObj(std::string s) :str(s) {}
private:
};

class FunObj :public CollectableObject{
public:
	std::vector<char> bytes;
	FunObj() {}
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