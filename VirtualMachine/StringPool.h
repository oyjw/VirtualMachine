#ifndef _STRING_POOL_H_
#define _STRING_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"

class StringPool{
private:
	std::unordered_map<std::string,int> builtIns;
	std::vector<StrObj*> strings;
	size_t constants;
public:
	StringPool() :constants(0) {
		int sindex = putStringConstant("__init__");
		builtIns.insert(std::make_pair("__init__",sindex));
	}
	~StringPool(){
		for (auto &sobj : strings){
			delete sobj;
		}
	}
	void collect(){
		size_t j = constants;
		for (size_t i = constants; i < strings.size(); ++i){
			if (strings[i]->mark){
				strings[j++] = strings[i];
			}
			else{
				delete strings[i];
			}
		}
		strings.resize(j);
	}
	int putStringConstant(const std::string& str){
		StrObj *sobj = new StrObj(str);
		strings.push_back(sobj);
		constants++;
		return (int)strings.size()-1;
	}
	int putString(const std::string &str){
		StrObj *sobj = new StrObj(str);
		strings.push_back(sobj);
		return (int)strings.size()-1;
	}
	StrObj* getStringConstant(const std::string& str){
		auto iter = builtIns.find(str);
		assert(iter != builtIns.end());
		return strings[iter->second];
	}
	StrObj* getStrObj(size_t n){
		return strings[n];
	}
	int getStrNum(){
		return int(strings.size()-constants);
	}

};

#endif