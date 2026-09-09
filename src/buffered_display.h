#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "display_palette.h"
#include "tile_hash.h"
class BufferedDisplay:public Adafruit_GFX{
 uint8_t* bands[15]={};uint32_t hashes[300]={};bool pending=true,first=true,allocated=false;
 uint8_t* row(int y)const{return bands[y/16]+(y%16)*320;}
 public:
 uint32_t sentTiles=0,presents=0;
 BufferedDisplay():Adafruit_GFX(320,240){}
 bool begin(){if(allocated)return true;for(int i=0;i<15;i++){bands[i]=(uint8_t*)calloc(320*16,1);if(!bands[i]){for(int j=0;j<i;j++){free(bands[j]);bands[j]=nullptr;}return false;}}allocated=true;return true;}
 bool ready()const{return allocated;}
 uint16_t pixelColor(int x,int y)const{if(!allocated||x<0||y<0||x>=320||y>=240)return 0;return pgm_read_word(displayPalette+row(y)[x]);}
 void drawPixel(int16_t x,int16_t y,uint16_t color)override{if(!allocated||x<0||y<0||x>=320||y>=240)return;uint8_t c=pgm_read_byte(displayColorIndex+color);auto&p=row(y)[x];if(p!=c){p=c;pending=true;}}
 void fillRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c)override{if(!allocated||w<=0||h<=0)return;int x1=max(0,(int)x),y1=max(0,(int)y),x2=min(320,(int)x+w),y2=min(240,(int)y+h);if(x1>=x2||y1>=y2)return;uint8_t idx=pgm_read_byte(displayColorIndex+c);for(int j=y1;j<y2;j++)memset(row(j)+x1,idx,x2-x1);pending=true;}
 void drawFastHLine(int16_t x,int16_t y,int16_t w,uint16_t c)override{fillRect(x,y,w,1,c);}
 void drawFastVLine(int16_t x,int16_t y,int16_t h,uint16_t c)override{fillRect(x,y,1,h,c);}
 void present(Adafruit_ST7789&panel){if(!allocated||!pending)return;uint16_t tile[256];bool any=false;for(int ty=0;ty<15;ty++)for(int tx=0;tx<20;tx++){int n=ty*20+tx;uint32_t h=tileHash(bands[ty],320,tx*16,0,16,16);if(!first&&hashes[n]==h)continue;hashes[n]=h;for(int j=0;j<16;j++)for(int i=0;i<16;i++)tile[j*16+i]=pgm_read_word(displayPalette+bands[ty][j*320+tx*16+i]);panel.drawRGBBitmap(tx*16,ty*16,tile,16,16);sentTiles++;any=true;}if(any)presents++;pending=false;first=false;}
};
