#pragma once
#include <cstdint>
inline uint32_t tileHash(const uint8_t*frame,int stride,int x,int y,int width,int height){uint32_t h=2166136261u;for(int j=0;j<height;j++)for(int i=0;i<width;i++){h^=frame[(y+j)*stride+x+i];h*=16777619u;}return h;}
