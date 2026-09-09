#include "../src/route_geometry.h"
#include "../src/road_simplify.h"
#include "../src/tile_hash.h"
#include <cassert>
#include <cstdio>
int main(){
 GeoPoint road[]={{114.2f,22.35f},{114.2f,22.34f},{114.2f,22.33f},{114.2f,22.32f},{114.2f,22.31f}};RoadWindow w=upstreamWindow(road,4);assert(fabs(w.meters-3000)<1);assert(w.first>0);assert(w.start.lat>road[w.first].lat);WindowStop stop[]={{road[4],0}};RoadData r;assert(projectWindow(road,w,stop,1,r));assert(r.points[0].x==r.points[r.count-1].x);assert(r.points[0].y<r.points[r.count-1].y);
 GeoPoint h[]={{114.20f,22.32f},{114.21f,22.32f},{114.22f,22.32f},{114.23f,22.32f},{114.24f,22.32f}};w=upstreamWindow(h,4);WindowStop hs[]={{h[4],0}};assert(projectWindow(h,w,hs,1,r));assert(r.points[0].y==r.points[r.count-1].y);assert(r.points[0].x<r.points[r.count-1].x);
 GeoPoint reverse[]={{114.24f,22.32f},{114.23f,22.32f},{114.22f,22.32f},{114.21f,22.32f},{114.20f,22.32f}};w=upstreamWindow(reverse,4);WindowStop rs[]={{reverse[4],0}};assert(projectWindow(reverse,w,rs,1,r));assert(r.points[0].x>r.points[r.count-1].x);
 GeoPoint diag[]={{114.20f,22.32f},{114.211f,22.33f},{114.222f,22.34f}};w=upstreamWindow(diag,2);WindowStop ds[]={{diag[2],0}};assert(projectWindow(diag,w,ds,1,r));assert(abs((r.points[r.count-1].x-r.points[0].x)-(r.points[0].y-r.points[r.count-1].y))<=3);
 GeoPoint u[]={{114.2f,22.32f},{114.2f,22.33f},{114.21f,22.33f},{114.21f,22.32f}};w=upstreamWindow(u,3);assert(fabs(w.meters-3000)<1);WindowStop us[]={{u[3],0}};assert(projectWindow(u,w,us,1,r));for(int i=0;i<r.count;i++)assert(r.points[i].x>=149&&r.points[i].x<=255&&r.points[i].y>=48&&r.points[i].y<=151);
 w=upstreamWindow(road,1);assert(w.first==0&&w.meters<3000);w=upstreamWindow(road,0);assert(w.meters==0);WindowStop origin[]={{road[0],0}};assert(projectWindow(road,w,origin,1,r));assert(r.count==1);
 std::vector<GeoPoint> reduced;RoadSimplifier simp(reduced);for(int i=0;i<100;i++)assert(simp.feed({114.2f+i*.00001f,22.32f}));assert(simp.finish());assert(reduced.size()<10);assert(geoMeters(reduced.back(),{114.20099f,22.32f})<1);
 uint8_t frame[1024]={};auto hash=tileHash(frame,32,0,0,16,16);assert(hash==tileHash(frame,32,0,0,16,16));frame[0]=1;assert(hash!=tileHash(frame,32,0,0,16,16));puts("3km window/direction/bounds/simplification/tile checks PASS");}
