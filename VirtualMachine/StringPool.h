#ifndef _STRING_POOL_H_
#define _STRING_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"

class StringPool{
private:
	std::vector<StrObj*> strings;
	size_t constants;
public:
	StringPool() :constants(0) {}
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
	StrObj* getStrObj(size_t n){
		return strings[n];
	}
	int getStrNum(){
		return int(strings.size()-constants);
	}

};

#endif