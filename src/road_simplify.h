#pragma once
#include <vector>
#include "route_geometry.h"
class RoadSimplifier{
 std::vector<GeoPoint>&out;GeoPoint pending[128];int used=0;
 bool push(GeoPoint q){if(!out.empty()&&geoMeters(out.back(),q)<.3f)return true;if(out.size()>=2048)return false;out.push_back(q);return true;}
 public:
 explicit RoadSimplifier(std::vector<GeoPoint>&result):out(result){}
 bool feed(GeoPoint q){if(!used){pending[used++]=q;return push(q);}if(geoMeters(pending[used-1],q)<.3f)return true;GeoPoint a=pending[0];float vx=(q.lng-a.lng)*.925f,vy=q.lat-a.lat,len=vx*vx+vy*vy;bool fits=used<128&&geoMeters(a,q)<=80;for(int i=1;i<used&&fits;i++){float wx=(pending[i].lng-a.lng)*.925f,wy=pending[i].lat-a.lat,t=len>0?(wx*vx+wy*vy)/len:0;if(t<0||t>1){fits=false;break;}if(std::hypot(wx-t*vx,wy-t*vy)*111320.f>2)fits=false;}if(fits){pending[used++]=q;return true;}GeoPoint last=pending[used-1];if(!push(last))return false;pending[0]=last;pending[1]=q;used=2;return true;}
 bool finish(){return used&&push(pending[used-1]);}
};
