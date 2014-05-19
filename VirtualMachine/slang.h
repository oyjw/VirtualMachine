#ifndef _SLANG_H_
#define _SLANG_H_

#ifdef __cplusplus
extern "C" {
#endif

void* newstate();
void pushstring(void* state,char* str);
void pushnumber(void* state,float num);
void parseFile(void* state, const char* fileName);


#ifdef __cplusplus
}
#endif

#endif