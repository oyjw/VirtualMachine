#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include "Object.h"

#include <memory>
typedef std::shared_ptr<SymbolTable> SymPtr;

class SymbolTable{
private:
	std::map<std::string, int> map;
	std::vector<Symbol> symVec;
	SymPtr global;
	typedef std::map<std::string, int>::iterator map_iter;
public:
	SymbolTable() {}
	SymbolTable(SymPtr g) :global(g)  {}
	int findSym(const std::string& symbol){
		map_iter iter = map.find(symbol);
		if (iter == map.end()) return -1;
		else return iter->second;
	}
	int putSym(const std::string& symbol){
		Object tmp;
		symVec.push_back({ tmp, symbol });
		int n = symVec.size() - 1;
		map.insert(make_pair(symbol, n));
		return n;
	}
	Object& getObj(int idx){
		return symVec[idx].obj;
	}
	bool putObj(const std::string& symbol, Object obj){
		map_iter iter = map.find(symbol);
		if (iter == map.end()) return false;
		symVec[iter->second].obj = obj;
		return true;
	}
	int getSymNum() {
		return (int)symVec.size();
	}
	SymPtr getGlobalSym(){
		return global;
	}
	/*bool isExist(const std::string &symbol){
		map_iter iter = map.find(symbol);
		return iter != map.end();
	}*/
};





#endif