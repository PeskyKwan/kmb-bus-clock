#include <initializer_list>
#include "../src/native_input.h"
#include <cassert>
#include <cstdio>
int main(){char s[8]="";assert(appendRouteKey(s,'9'));assert(appendRouteKey(s,'2'));assert(!strcmp(s,"92"));eraseRouteKey(s);assert(!strcmp(s,"9"));s[0]=0;for(char c: {'N','2','6','9'})assert(appendRouteKey(s,c));assert(!strcmp(s,"N269"));assert(!appendRouteKey(s,'!'));assert(!appendRouteKey(s,'x'));assert(appendRouteKey(s,'X'));assert(appendRouteKey(s,'1'));assert(!appendRouteKey(s,'2'));while(strlen(s))eraseRouteKey(s);eraseRouteKey(s);assert(!strlen(s));assert(pageNext(0,3)==0);assert(pageNext(0,4)==3);assert(pageNext(3,4)==3);assert(pagePrevious(3)==0);assert(pagePrevious(0)==0);puts("Native route keys and pagination passed");}
