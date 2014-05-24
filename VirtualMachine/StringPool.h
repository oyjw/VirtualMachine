#ifndef _STRING_POOL_H_
#define _STRING_POOL_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "Object.h"

class StringPool{
private:
	std::vector<StrObj> strings;
	size_t constants;
public:
	StringPool() :constants(0) {}
	void collect(){
		std::vector<StrObj> newvec;
		StrObj so;
		for (size_t i = 0; i < strings.size(); ++i){
			if (i<constants || strings[i].mark){
				so.str = std::move(strings[i].str);
				newvec.push_back(so);
			}
		}
		strings = std::move(newvec);
	}
	int putStringConstant(const std::string& str){
		strings.push_back(StrObj(str));
		constants++;
		return (int)strings.size()-1;
	}
	int putString(const std::string &str){
		strings.push_back(StrObj(str));
		return (int)strings.size()-1;
	}
	StrObj* getStrObj(size_t n){
		return &strings[n];
	}
	int getStrNum(){
		return int(strings.size()-constants);
	}

};

#endif