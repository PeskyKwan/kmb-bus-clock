#pragma once
#include <cstring>
#include <cctype>
inline bool appendRouteKey(char* route,char key){size_t n=strlen(route);if(n>=6||!((key>='0'&&key<='9')||(key>='A'&&key<='Z')))return false;route[n]=key;route[n+1]=0;return true;}
inline void eraseRouteKey(char* route){size_t n=strlen(route);if(n)route[n-1]=0;}
inline int pageNext(int current,int total){return current+3<total?current+3:current;}
inline int pagePrevious(int current){return current>=3?current-3:0;}
inline void rotateLogicalPoint(bool flipped,int&x,int&y,int width=320,int height=240){if(flipped){x=width-1-x;y=height-1-y;}}
inline int adjustLeadSeconds(int current,int delta){int next=current+delta;return next<0?0:next>90?90:next;}
