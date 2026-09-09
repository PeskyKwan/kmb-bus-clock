int wifiOffset=0,wifiKeysPage=0;String wifiInput,wifiChosen;
const char* wifiKeys[]={"qwertyuiopasdfghjklzxcvbnm-_. ","QWERTYUIOPASDFGHJKLZXCVBNM-_. ","1234567890!@#$%^&*()-_=+[]{};:","'\",.<>/?\\|`~ "};
void enterWifi(){wifiOffset=0;wifiInput="";nativePage=NS_WIFI;scanWifi();}
void drawWifi(){
 if(nativePage==NS_WIFI){label(12,40,wifiNotice.length()?wifiNotice.c_str():(WiFi.status()==WL_CONNECTED?"已連線":"Wi-Fi"),12,INK,296);
  for(int row=0;row<4;row++){int i=wifiOffset+row;if(i>=wifiCount)break;nativeButton(12,58+row*33,296,30,wifiRows[i].ssid.c_str(),(wifiRows[i].ssid==wifiName||wifiRows[i].ssid==wifiPreviousName)?FEATURE:PANEL,12);}
  nativeButton(8,200,71,32,"搜尋",PANEL,12);nativeButton(86,200,71,32,"手動",PANEL,12);nativeButton(164,200,71,32,"上一頁",PANEL,12);nativeButton(242,200,71,32,"下一頁",PANEL,12);return;
 }
 if(nativePage==NS_WCONNECT){centered(0,84,320,wifiNotice.c_str(),20);label(12,120,wifiChosen.c_str(),12,INK,296);nativeButton(12,201,296,32,wifiWorkActive()?"取消":"完成",INK);return;}
 label(12,40,nativePage==NS_WSSID?"網絡名稱":wifiChosen.c_str(),12,INK,296);if(nativePage==NS_WPASS&&(wifiChosen==wifiName||wifiChosen==wifiPreviousName))label(264,40,"已存",12);lcd.fillRoundRect(8,60,304,30,5,FIELD);textBG=FIELD;
 String shown=wifiInput;if(nativePage==NS_WPASS){shown="";for(int i=0;i<(int)wifiInput.length();i++)shown+='*';}while(textWidth(shown.c_str(),16)>280)shown=shown.substring(1);label(15,66,shown.c_str(),16,INK,280);textBG=BG;
 const char*keys=wifiKeys[wifiKeysPage];for(int i=0;keys[i];i++){char k[2]={keys[i],0};nativeButton(6+i%10*31,96+i/10*30,28,27,keys[i]==' '?"空格":k,PANEL,12);}
 nativeButton(6,192,48,40,"Aa",PANEL,12);nativeButton(58,192,48,40,"123",PANEL,12);nativeButton(110,192,48,40,"符號",PANEL,12);nativeButton(162,192,48,40,"刪除",PANEL,12);nativeButton(216,192,98,40,nativePage==NS_WSSID?"下一步":"連接",INK,16);
}
void touchWifi(int x,int y){
 if(nativePage==NS_WIFI){if(wifiWorkActive())return;if(y>=58&&y<190&&x>=12&&x<308){int i=wifiOffset+(y-58)/33;if(i<wifiCount&&(y-58)%33<30){wifiChosen=wifiRows[i].ssid;wifiInput="";wifiKeysPage=0;nativePage=NS_WPASS;}}else if(y>=200){if(x<80){wifiOffset=0;scanWifi();}else if(x<158){wifiInput="";wifiKeysPage=0;nativePage=NS_WSSID;}else if(x<236)wifiOffset=max(0,wifiOffset-4);else if(wifiOffset+4<wifiCount)wifiOffset+=4;}return;}
 if(nativePage==NS_WCONNECT){if(y>=201){if(wifiWorkActive())cancelWifi();nativePage=NS_WIFI;wifiInput="";}return;}
 if(y>=96&&y<186&&x>=6&&x<316){int c=(x-6)/31,r=(y-96)/30,i=r*10+c;const char*keys=wifiKeys[wifiKeysPage];int maxlen=nativePage==NS_WSSID?32:63;if((x-6)%31<28&&(y-96)%30<27&&i<(int)strlen(keys)&&wifiInput.length()<maxlen)wifiInput+=keys[i];return;}
 if(y>=192){if(x<54)wifiKeysPage=wifiKeysPage==0?1:0;else if(x<106)wifiKeysPage=2;else if(x<158)wifiKeysPage=3;else if(x<210){if(wifiInput.length())wifiInput.remove(wifiInput.length()-1);}else if(x>=216){if(nativePage==NS_WSSID){if(wifiInput.length()){wifiChosen=wifiInput;wifiInput="";nativePage=NS_WPASS;}}else if(requestWifi(wifiChosen,wifiInput)){wifiInput="";nativePage=NS_WCONNECT;}else nativeMessage=wifiNotice;}}
}
