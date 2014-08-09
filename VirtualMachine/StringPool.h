#ifndef _STRING_POOL_H_
#define _STRING_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"

class StringPool{
private:
	std::unordered_map<std::string,int> sharedStrings;
	std::vector<StrObj*> strings;
	size_t constants;
public:
	StringPool() :constants(0) {
		putStringConstant("None");
		putStringConstant("True");
		putStringConstant("False");
		putStringConstant("__init__");
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
				strings[i]->mark = false;
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
		sharedStrings.insert(std::make_pair(str,(int)constants-1));
		return (int)constants-1;
	}
	StrObj* putString(const std::string &str){
		StrObj *sobj = new StrObj(str);
		strings.push_back(sobj);
		return sobj;
	}
	int getStringConstant(const std::string& str){
		auto iter = sharedStrings.find(str);
		if(iter == sharedStrings.end())
			return -1;
		return iter->second;
	}
	/*StrObj* getStringConstant(const std::string& str){
		auto iter = sharedStrings.find(str);
		if(iter == sharedStrings.end())
			return NULL;
		return strings[iter->second];
	}*/
	StrObj* getStrObj(size_t n){
		return strings[n];
	}
	int getStrNum(){
		return int(strings.size()-constants);
	}

};

#endif