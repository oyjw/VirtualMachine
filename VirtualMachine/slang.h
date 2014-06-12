#ifndef _SLANG_H_
#define _SLANG_H_


#include "Object.h"
#ifdef __cplusplus
extern "C" {
#endif
#define SYMBOLEXIST 1
int defineMethod(void* state,cFunc func);
void* newState();
void parseFile(void* state, const char* fileName);
void getArgs(void* state, int *len, Object** objects);
ClsType* defineClass(void* state, char* className);
#ifdef __cplusplus
}
#endif

#endif