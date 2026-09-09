#pragma once
#include <Adafruit_GFX.h>
// Reuse the off-screen map area as plate scratch space. Rotation writes into
// the disjoint left column; drawMap replaces this scratch before presentation.
// No extra frame allocation, leaving RAM available for TLS and ETA requests.
class PlateCanvas:public Adafruit_GFX{
 BufferedDisplay& target;
 public:
 explicit PlateCanvas(BufferedDisplay& display):Adafruit_GFX(112,112),target(display){}
 void drawPixel(int16_t x,int16_t y,uint16_t c)override{if(x>=0&&x<112&&y>=0&&y<112)target.drawPixel(134+x,35+y,c);}
 uint16_t pixelColor(int x,int y)const{return target.pixelColor(134+x,35+y);}
};
