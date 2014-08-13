#ifndef _STRING_H_
#define _STRING_H_
#include "Object.h"
#include <string>
std::string toPrintableStr(void* state,const Object& o,bool addQuote);
void strInit(void* state);
#endif