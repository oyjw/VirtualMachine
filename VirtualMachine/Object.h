#ifndef _OBJECT_H_
#define _OBJECT_H_
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#define NILOBJ  0
#define NUMOBJ  1
#define BOOLOBJ 2
#define USERTYPE 3
#define CLSOBJ  4
#define CLSTYPE 5
#define FUNOBJ  1<<3
#define CFUNOBJ 1<<4
#define METHOD  1<<5
#define USEROBJ 1<<6
#define LISTOBJ 1<<7
#define DICTOBJ 1<<8
#define STROBJ  1<<9


class StrObj {
public:
	bool mark;
	std::string str;
	StrObj() :mark(false) {}
	StrObj(const std::string& s):str(s),mark(false) {}
};

static size_t strHasher(StrObj* const & s){
	return std::hash<std::string>()((*s).str);
}

static bool strEq( StrObj* const & s1,StrObj* const & s2){
	return (*s1).str == (*s2).str;
}

struct FunObj {
	std::string functionName;
	std::vector<char> bytes;
	int nArgs;
	FunObj():nArgs(0) {}
};

class ClsType;
class ClsObj;

struct Object;
typedef Object (*cFunc)(void* state);

struct CFunObj{
	int nArgs;
	cFunc fun;
	std::string functionName;
	CFunObj() :nArgs(0), fun(NULL) {}
};

struct Method{
	union {
		FunObj* funObj;
		CFunObj* cFunObj;
	};
	ClsObj* self;
};

class ClsType {
public:
	std::string clsName;
	std::unordered_map<StrObj*,Object,decltype(strHasher)*,decltype(strEq)*> clsAttrs{0,strHasher,strEq};
	ClsType() {}
};

class ClsObj {
public:
	bool mark;
	ClsType* clsType;
	std::unordered_map<StrObj*,Object,decltype(strHasher)*,decltype(strEq)*> attrs{0,strHasher,strEq};
	ClsObj() :mark(false),clsType(NULL) {}
};

struct UserData{
	bool mark;
	void* data;
	ClsType* type;
	UserData(ClsType* t, void* d) :type(t), data(d), mark(false) {}
};

struct Object{
	int type;
	union{
		bool boolval;
		float numval;
		StrObj* strObj;
		FunObj* funObj;
		CFunObj* cFunObj;
		ClsObj* clsObj;
		ClsType* clsType;
		Method method;
		UserData* userData;
	} value;
};

static Object NilObj = { NILOBJ, {} };


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