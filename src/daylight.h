#pragma once
#include <cmath>
struct SolarMinutes{int rise,set;};
// Approximate solar declination/equation of time; 90.833 degrees includes
// standard horizon refraction. Hong Kong UTC+8; no network/location tracking.
inline SolarMinutes solarMinutes(int day,double lat,double lng){
 const double pi=3.141592653589793,g=2*pi/365.0*(day-1);
 double eq=229.18*(.000075+.001868*cos(g)-.032077*sin(g)-.014615*cos(2*g)-.040849*sin(2*g));
 double dec=.006918-.399912*cos(g)+.070257*sin(g)-.006758*cos(2*g)+.000907*sin(2*g)-.002697*cos(3*g)+.00148*sin(3*g);
 double c=(cos(90.833*pi/180)/cos(lat*pi/180)/cos(dec)-tan(lat*pi/180)*tan(dec));
 if(c>1)return {720,720};if(c<-1)return {0,1440};
 double half=acos(c)*180/pi*4,noon=720-4*lng-eq+480;
 return {(int)lround(noon-half),(int)lround(noon+half)};
}
