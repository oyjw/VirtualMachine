#ifndef _STRING_H_
#define _STRING_H_
#include "Object.h"
#include <string>
std::string& getStr(const Object& obj);
void strInit(void* state);
Object toStr(void* state, Object& o);
#endif