#ifndef _OBJECT_H_
#define _OBJECT_H_
#include <string>
#include <vector>

enum ObjectType{
	NUMOBJ, STROBJ, FUNOBJ,BOOLOBJ,NILOBJ, CLSOBJ
};

struct CollectableObject{
};




class StrObj {
public:
	std::string str;
	bool mark;
	StrObj() :mark(false) {}
	StrObj(const std::string& s):str(s),mark(false) {}
};

#define NORMALFUNC 0
#define STATICCLSFUNC 1
#define CLSFUNC 2

class FunObj {
public:
	std::vector<char> bytes;
	int funType;
	int nargs;
	FunObj():funType(0),nargs(0) {}
};

struct Object{
	ObjectType type;
	union{
		bool boolval;
		float numval;
		StrObj* strObj;
		FunObj* funObj;
		ClsObj* clsObj;
	} value;
};

struct Field{
	bool isStatic;
	Object obj;
};

class ClsType {
public:
	std::vector<FunObj> methods;
	std::map<std::string,std::pair<int,bool>> methodMap; //isStatic
	std::vector<Object> staticFields;
	std::map<std::string,std::pair<int,bool>> fieldMap; //isStatic
	std::unordered_map<std::string,Object> clsAttrs;
	ClsType() {}
};

class ClsObj {
public:
	ClsType *metadata;
	std::vector<Object> fields;
	ClsObj() {}
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