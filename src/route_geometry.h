#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
struct GeoPoint{float lng,lat;};
struct MapPoint{int16_t x,y;};
struct RoadData{bool valid=false;uint32_t gen=0;int count=0;MapPoint points[256];MapPoint stops[3];};
inline float geoDistance2(GeoPoint a,GeoPoint b){float x=(a.lng-b.lng)*.925f,y=a.lat-b.lat;return x*x+y*y;}
// Match stops in travel order, including looping paths, without sorting by latitude.
inline bool fitRoad(const GeoPoint*points,int count,const GeoPoint*stops,int stopCount,RoadData&out){
 if(count<2||stopCount<1||stopCount>3)return false;
 float best[3]={1e9f,1e9f,1e9f};int picked[3][3]={};
 for(int i=0;i<count;i++){for(int s=0;s<stopCount;s++){float v=geoDistance2(points[i],stops[s])+(s?best[s-1]:0);if(v<best[s]){best[s]=v;if(s)for(int j=0;j<s;j++)picked[s][j]=picked[s-1][j];picked[s][s]=i;}}}
 int first=picked[stopCount-1][0],last=picked[stopCount-1][stopCount-1];
 for(int i=0;i<stopCount;i++)if(geoDistance2(points[picked[stopCount-1][i]],stops[i])>0.00000182f)return false; // about150m
 if(last<=first)return false;
 float minx=1e9,maxx=-1e9,miny=1e9,maxy=-1e9;
 auto bounds=[&](GeoPoint p){float x=p.lng*.925f,y=-p.lat;minx=std::min(minx,x);maxx=std::max(maxx,x);miny=std::min(miny,y);maxy=std::max(maxy,y);};
 for(int i=first;i<=last;i++)bounds(points[i]);for(int i=0;i<stopCount;i++)bounds(stops[i]);
 float dx=std::max(maxx-minx,.00005f),dy=std::max(maxy-miny,.00005f),scale=std::min(104.0f/dx,101.0f/dy);
 float ox=150+(104-dx*scale)/2,oy=49+(101-dy*scale)/2;
 auto project=[&](GeoPoint p){return MapPoint{(int16_t)std::lround(ox+(p.lng*.925f-minx)*scale),(int16_t)std::lround(oy+(-p.lat-miny)*scale)};};
 out.count=0;int span=last-first;int n=std::min(span+1,256);
 for(int j=0;j<n;j++){int index=first+(int)std::lround(j*span/float(n-1));MapPoint p=project(points[index]);if(out.count&&out.points[out.count-1].x==p.x&&out.points[out.count-1].y==p.y)continue;out.points[out.count++]=p;}
 for(int j=0;j<stopCount;j++)out.stops[j]=project(stops[j]);out.valid=out.count>=2;return out.valid;
}
