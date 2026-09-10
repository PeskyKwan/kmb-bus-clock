#pragma once
#include <ctime>
#include <cmath>
#include <algorithm>
#include <cstdint>
constexpr int positionAnchors=12,positionForecasts=3,trackedBuses=2;
struct EtaAnchor{int seq;float fraction;EtaAnchor(int s=0,float f=0):seq(s),fraction(f){}};
struct EtaLayout{int count=0;float meters=0;EtaAnchor anchors[positionAnchors];};
struct StopForecast{std::time_t eta,stamp;bool scheduled;uint8_t rank;StopForecast(std::time_t e=0,std::time_t t=0,bool s=false,int r=0):eta(e),stamp(t),scheduled(s),rank(r){}};
struct EtaFrame{EtaLayout layout;std::time_t generated=0;int counts[positionAnchors]={};StopForecast rows[positionAnchors][positionForecasts];
 void add(int seq,StopForecast f){for(int i=0;i<layout.count;i++)if(layout.anchors[i].seq==seq){for(int j=0;j<counts[i];j++)if(rows[i][j].eta==f.eta&&rows[i][j].rank==f.rank){if(f.stamp>rows[i][j].stamp)rows[i][j]=f;else if(f.stamp==rows[i][j].stamp)rows[i][j].scheduled|=f.scheduled;return;}if(counts[i]<positionForecasts)rows[i][counts[i]++]=f;return;}}
};
struct EtaPath{int first=0,count=0;EtaLayout layout;StopForecast rows[positionAnchors];std::time_t target=0,stamp=0;bool valid=false;};
inline bool usablePositionForecast(StopForecast r,std::time_t now){return r.eta>0&&r.stamp>0&&!r.scheduled&&r.stamp<=now+15&&now-r.stamp<=120&&r.eta>=now-180&&r.eta<=now+10800;}
class EtaPositionTracker{
 EtaPath previous[trackedBuses]{},accepted[trackedBuses]{};int confirmations[trackedBuses]={};
 bool build(const EtaFrame&f,std::time_t now,StopForecast target,const EtaPath&old,const EtaPath*ahead,EtaPath&p,bool&same)const{
  const int n=f.layout.count;p={};p.layout=f.layout;p.count=n;p.first=n-1;p.target=target.eta;p.rows[n-1]=target;
  same=old.valid&&old.layout.count==n&&std::abs(double(old.target-p.target))<=90;
  for(int i=0;i<n&&same;i++)if(old.layout.anchors[i].seq!=f.layout.anchors[i].seq)same=false;
  for(int i=n-2;i>=0;i--){
   const float distance=(f.layout.anchors[i+1].fraction-f.layout.anchors[i].fraction)*f.layout.meters;
   if(distance<0)break;const float lower=std::max(1.f,distance/25.f),upper=std::min(600.f,120.f+distance/1.5f);
   int found=0;StopForecast chosen{};
   for(int j=0;j<f.counts[i];j++){auto r=f.rows[i][j];double gap=difftime(p.rows[i+1].eta,r.eta);bool ordered=!ahead||!ahead->valid||i<ahead->first||r.eta>ahead->rows[i].eta+30;if(usablePositionForecast(r,now)&&ordered&&gap>=lower&&gap<=upper){chosen=r;found++;}}
   // Only a previously matched, recently expired forecast may bridge a removed row.
   if(found==0&&same&&i>=old.first){auto r=old.rows[i];double gap=difftime(p.rows[i+1].eta,r.eta);bool ordered=!ahead||!ahead->valid||i<ahead->first||r.eta>ahead->rows[i].eta+30;if(usablePositionForecast(r,now)&&ordered&&r.eta<=now&&now-r.stamp<=90&&gap>=lower&&gap<=upper){chosen=r;found=1;}}
   if(found!=1)break;p.rows[i]=chosen;p.first=i;
  }
  if(n-p.first<2)return false;
  std::time_t oldest=now+15;for(int i=p.first;i<n;i++){p.stamp=std::max(p.stamp,p.rows[i].stamp);oldest=std::min(oldest,p.rows[i].stamp);}if(p.stamp-oldest>90)return false;
  p.valid=true;if(same){int common=0;for(int i=std::max(p.first,old.first);i<n;i++)if(std::abs(double(p.rows[i].eta-old.rows[i].eta))<=90)common++;if(common<2)same=false;}return true;
 }
 bool estimatePath(const EtaPath&p,std::time_t now,float&fraction)const{
  if(!p.valid||now<p.stamp-15||now-p.stamp>90)return false;
  for(int i=p.first;i<p.count-1;i++){auto a=p.rows[i],b=p.rows[i+1];if(!usablePositionForecast(a,now)||!usablePositionForecast(b,now))return false;if(a.eta<=now&&now<b.eta){double u=difftime(now,a.eta)/difftime(b.eta,a.eta);fraction=p.layout.anchors[i].fraction+u*(p.layout.anchors[i+1].fraction-p.layout.anchors[i].fraction);return fraction>=0&&fraction<=1;}}
  return false;
 }
 public:
 int samples(int index=0)const{return index>=0&&index<trackedBuses?confirmations[index]:0;}
 int count()const{return (accepted[0].valid?1:0)+(accepted[1].valid?1:0);}
 void reset(){for(int i=0;i<trackedBuses;i++){previous[i]={};accepted[i]={};confirmations[i]=0;}}
 bool accept(const EtaFrame&f,std::time_t now,std::time_t target){
  const int n=f.layout.count;if(n<2||n>positionAnchors||f.layout.meters<=0||!f.generated||std::abs(double(now-f.generated))>120){reset();return false;}
  for(int i=1;i<n;i++)if(f.layout.anchors[i].seq<=f.layout.anchors[i-1].seq||f.layout.anchors[i].fraction<f.layout.anchors[i-1].fraction){reset();return false;}
  StopForecast targets[trackedBuses];int targetCount=0,matches=0;
  for(int j=0;j<f.counts[n-1];j++){auto r=f.rows[n-1][j];if(usablePositionForecast(r,now)&&std::abs(double(r.eta-target))<=90){targets[0]=r;matches++;}}
  if(matches!=1){reset();return false;}targetCount=1;
  for(int j=0;j<f.counts[n-1];j++){auto r=f.rows[n-1][j];if(!usablePositionForecast(r,now)||r.eta<=targets[0].eta+90)continue;if(targetCount==1||r.eta<targets[1].eta){targets[1]=r;targetCount=2;}}
  EtaPath built[trackedBuses];for(int k=0;k<trackedBuses;k++){
   if(k>=targetCount){previous[k]={};accepted[k]={};confirmations[k]=0;continue;}bool same=false;if(!build(f,now,targets[k],previous[k],k?&built[0]:nullptr,built[k],same)){previous[k]={};accepted[k]={};confirmations[k]=0;if(k==0){previous[1]={};accepted[1]={};confirmations[1]=0;return false;}continue;}
   if(!same)confirmations[k]=1;else if(built[k].stamp>previous[k].stamp)confirmations[k]=std::min(2,confirmations[k]+1);previous[k]=built[k];if(confirmations[k]>=2)accepted[k]=built[k];else accepted[k]={};
  }
  return accepted[0].valid;
 }
 bool estimate(std::time_t now,std::time_t target,float&fraction)const{return std::abs(double(target-accepted[0].target))<=90&&estimatePath(accepted[0],now,fraction);}
 bool estimate(int index,std::time_t now,float&fraction)const{return index>=0&&index<trackedBuses&&estimatePath(accepted[index],now,fraction);}
};
