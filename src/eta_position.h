#pragma once
#include <ctime>
#include <cmath>
#include <algorithm>
#include <cstdint>
constexpr int positionAnchors=12,positionForecasts=3;
struct EtaAnchor{int seq;float fraction;EtaAnchor(int s=0,float f=0):seq(s),fraction(f){}};
struct EtaLayout{int count=0;float meters=0;EtaAnchor anchors[positionAnchors];};
struct StopForecast{std::time_t eta,stamp;bool scheduled;uint8_t rank;StopForecast(std::time_t e=0,std::time_t t=0,bool s=false,int r=0):eta(e),stamp(t),scheduled(s),rank(r){}};
struct EtaFrame{EtaLayout layout;std::time_t generated=0;int counts[positionAnchors]={};StopForecast rows[positionAnchors][positionForecasts];
 void add(int seq,StopForecast f){for(int i=0;i<layout.count;i++)if(layout.anchors[i].seq==seq){for(int j=0;j<counts[i];j++)if(rows[i][j].eta==f.eta&&rows[i][j].rank==f.rank){if(f.stamp>rows[i][j].stamp)rows[i][j]=f;else if(f.stamp==rows[i][j].stamp)rows[i][j].scheduled|=f.scheduled;return;}if(counts[i]<positionForecasts)rows[i][counts[i]++]=f;return;}}
};
struct EtaPath{int first=0,count=0;EtaLayout layout;StopForecast rows[positionAnchors];std::time_t target=0,stamp=0;bool valid=false;};
inline bool usablePositionForecast(StopForecast r,std::time_t now){return r.eta>0&&r.stamp>0&&!r.scheduled&&r.stamp<=now+15&&now-r.stamp<=120&&r.eta>=now-180&&r.eta<=now+10800;}
class EtaPositionTracker{
 EtaPath previous{},accepted{};int confirmations=0;
 public:
 int samples()const{return confirmations;}
 void reset(){previous={};accepted={};confirmations=0;}
 bool accept(const EtaFrame&f,std::time_t now,std::time_t target){
  const int n=f.layout.count;if(n<2||n>positionAnchors||f.layout.meters<=0||!f.generated||std::abs(double(now-f.generated))>120){reset();return false;}
  for(int i=1;i<n;i++)if(f.layout.anchors[i].seq<=f.layout.anchors[i-1].seq||f.layout.anchors[i].fraction<f.layout.anchors[i-1].fraction){reset();return false;}
  EtaPath p;p.layout=f.layout;p.count=n;p.first=n-1;p.target=target;
  int matches=0;for(int j=0;j<f.counts[n-1];j++){auto r=f.rows[n-1][j];if(usablePositionForecast(r,now)&&std::abs(double(r.eta-target))<=90){p.rows[n-1]=r;matches++;}}
  if(matches!=1){reset();return false;}p.target=p.rows[n-1].eta;
  bool same=previous.valid&&previous.layout.count==n&&std::abs(double(previous.target-p.target))<=90;
  for(int i=0;i<n&&same;i++)if(previous.layout.anchors[i].seq!=f.layout.anchors[i].seq)same=false;
  for(int i=n-2;i>=0;i--){
   const float distance=(f.layout.anchors[i+1].fraction-f.layout.anchors[i].fraction)*f.layout.meters;
   if(distance<0)break;const float lower=std::max(1.f,distance/25.f),upper=std::min(600.f,120.f+distance/1.5f);
   int found=0;StopForecast chosen{};
   for(int j=0;j<f.counts[i];j++){auto r=f.rows[i][j];double gap=difftime(p.rows[i+1].eta,r.eta);if(usablePositionForecast(r,now)&&gap>=lower&&gap<=upper){chosen=r;found++;}}
   // Only a previously matched, recently expired forecast may bridge a removed row.
   if(found==0&&same&&i>=previous.first){auto r=previous.rows[i];double gap=difftime(p.rows[i+1].eta,r.eta);if(usablePositionForecast(r,now)&&r.eta<=now&&now-r.stamp<=90&&gap>=lower&&gap<=upper){chosen=r;found=1;}}
   if(found!=1)break;p.rows[i]=chosen;p.first=i;
  }
  if(n-p.first<2){reset();return false;}
  std::time_t oldest=now+15;for(int i=p.first;i<n;i++){p.stamp=std::max(p.stamp,p.rows[i].stamp);oldest=std::min(oldest,p.rows[i].stamp);}if(p.stamp-oldest>90){reset();return false;}
  p.valid=true;
  if(same){int common=0;for(int i=std::max(p.first,previous.first);i<n;i++)if(std::abs(double(p.rows[i].eta-previous.rows[i].eta))<=90)common++;
   if(common<2)same=false;}
  if(!same)confirmations=1;else if(p.stamp>previous.stamp)confirmations=std::min(2,confirmations+1);
  previous=p;if(confirmations>=2)accepted=p;else accepted={};return accepted.valid;
 }
 bool estimate(std::time_t now,std::time_t target,float&fraction)const{
  if(!accepted.valid||std::abs(double(target-accepted.target))>90||now<accepted.stamp-15||now-accepted.stamp>90)return false;
  for(int i=accepted.first;i<accepted.count-1;i++){auto a=accepted.rows[i],b=accepted.rows[i+1];if(!usablePositionForecast(a,now)||!usablePositionForecast(b,now))return false;
   if(a.eta<=now&&now<b.eta){double u=difftime(now,a.eta)/difftime(b.eta,a.eta);fraction=accepted.layout.anchors[i].fraction+u*(accepted.layout.anchors[i+1].fraction-accepted.layout.anchors[i].fraction);return fraction>=0&&fraction<=1;}}
  return false;
 }
};
