#include "../src/route_geometry.h"
#include "../src/tile_hash.h"
#include <cassert>
#include <cstdio>
int main(){RoadData r;
 GeoPoint v[]={{114.2f,22.322f},{114.2f,22.321f},{114.2f,22.320f}};assert(fitRoad(v,3,v,3,r));assert(r.stops[0].x==r.stops[2].x);assert(r.stops[0].y<r.stops[2].y);
 GeoPoint h[]={{114.2f,22.32f},{114.201f,22.32f},{114.202f,22.32f}};assert(fitRoad(h,3,h,3,r));assert(r.stops[0].y==r.stops[2].y);assert(r.stops[0].x<r.stops[2].x);
 GeoPoint d[]={{114.2f,22.32f},{114.20108f,22.321f},{114.20216f,22.322f}};assert(fitRoad(d,3,d,3,r));assert(abs((r.stops[2].x-r.stops[0].x)-(r.stops[0].y-r.stops[2].y))<=2);
 GeoPoint u[]={{114.2f,22.32f},{114.2f,22.33f},{114.22f,22.33f},{114.22f,22.32f}};GeoPoint ends[]={u[0],u[3]};assert(fitRoad(u,4,ends,2,r));for(int i=0;i<r.count;i++)assert(r.points[i].x>=149&&r.points[i].x<=255&&r.points[i].y>=48&&r.points[i].y<=151);
 GeoPoint bad[]={{114.7f,22.8f},{114.8f,22.9f}};assert(!fitRoad(u,4,bad,2,r));
 GeoPoint same[]={{114.2f,22.32f},{114.2f,22.32f}};assert(!fitRoad(same,2,same,2,r));
 uint8_t frame[32*32]={};uint32_t hashes[4];for(int y=0;y<2;y++)for(int x=0;x<2;x++)hashes[y*2+x]=tileHash(frame,32,x*16,y*16,16,16);
 assert(tileHash(frame,32,0,0,16,16)==hashes[0]);frame[3*32+18]=2;int changed=0;for(int y=0;y<2;y++)for(int x=0;x<2;x++)changed+=hashes[y*2+x]!=tileHash(frame,32,x*16,y*16,16,16);assert(changed==1);frame[3*32+18]=0;assert(tileHash(frame,32,16,0,16,16)==hashes[1]);puts("Map orientations/bounds and unchanged-tile checks passed");}
