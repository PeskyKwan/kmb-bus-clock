#include <esp_wifi.h>
#include "wifi_input.h"
// Wi-Fi operations run only after outstanding ETA/route TLS jobs finish.
// Secrets are never included in replies or prefilled into the keyboard.
bool networkWorkIdle();
class ClockWifiScan:public WiFiScanClass{public:static void allowFinish(){_scanTimeout=30000;}};
struct WifiRow{String ssid;int rssi;bool open;};
WifiRow wifiRows[20];int wifiCount=0,wifiStage=0,wifiScanDiagnostic=0;
String wifiCandidate,wifiCandidatePass,wifiNotice="",wifiPreviousName,wifiPreviousPass;
unsigned long wifiStarted=0;
bool wifiWorkActive(){return wifiStage!=0;}
void wifiReply(const String&id=""){
 DynamicJsonDocument out(4096);out["event"]="wifi";out["id"]=id;out["stage"]=wifiStage;out["diagnostic"]=wifiScanDiagnostic;out["message"]=wifiNotice;out["connected"]=WiFi.status()==WL_CONNECTED;out["current"]=wifiName;
 JsonArray rows=out.createNestedArray("networks");for(int i=0;i<wifiCount;i++){auto r=rows.createNestedObject();r["ssid"]=wifiRows[i].ssid;r["rssi"]=wifiRows[i].rssi;r["open"]=wifiRows[i].open;r["saved"]=wifiRows[i].ssid==wifiName||wifiRows[i].ssid==wifiPreviousName;}serializeJson(out,Serial);Serial.println();
}
void restoreWifi(){WiFi.setAutoReconnect(true);if(wifiName.length())WiFi.begin(wifiName.c_str(),wifiPassword.c_str());}
void cancelWifi(){int was=wifiStage;wifiStage=0;wifiCandidatePass="";if(was==2){esp_wifi_scan_stop();WiFi.scanDelete();}if(was==2||was==5||was==4){WiFi.disconnect(false,false);restoreWifi();}wifiNotice="已取消";needsDraw=true;}
bool requestWifi(const String&name,const String&password){
 if(wifiWorkActive())return false;
 if(!validWifiCredentials(name.c_str(),password.c_str())){wifiNotice="請檢查密碼";return false;}
 wifiCandidate=name;wifiCandidatePass=password;
 if(!password.length()&&name==wifiName)wifiCandidatePass=wifiPassword;else if(!password.length()&&name==wifiPreviousName)wifiCandidatePass=wifiPreviousPass;
 wifiStage=3;wifiNotice="連接中";needsDraw=true;return true;
}
void scanWifi(){if(wifiWorkActive())return;wifiStage=1;wifiNotice="搜尋中";needsDraw=true;}
void pollWifiSetup(){
 if(wifiStage==1&&networkWorkIdle()){wifiCount=0;WiFi.setAutoReconnect(false);if(esp_wifi_set_storage(WIFI_STORAGE_RAM)!=ESP_OK){wifiStage=0;wifiNotice="搜尋失敗";return;}WiFi.disconnect(true,true);WiFi.mode(WIFI_STA);wifiStage=5;wifiStarted=millis();}
 if(wifiStage==5&&millis()-wifiStarted>750){WiFi.scanDelete();int code=WiFi.scanNetworks(true,true,false,300);ClockWifiScan::allowFinish();wifiScanDiagnostic=100+code;if(code==WIFI_SCAN_FAILED){wifiStage=0;wifiNotice="搜尋失敗";restoreWifi();}else{wifiStage=2;wifiStarted=millis();}needsDraw=true;}
 if(wifiStage==2){int n=WiFi.scanComplete();wifiScanDiagnostic=200+n;if(n>=0){for(int i=0;i<n&&wifiCount<20;i++){String name=WiFi.SSID(i);if(!name.length())continue;bool duplicate=false;for(int j=0;j<wifiCount;j++)if(wifiRows[j].ssid==name)duplicate=true;if(!duplicate)wifiRows[wifiCount++]={name,WiFi.RSSI(i),WiFi.encryptionType(i)==WIFI_AUTH_OPEN};}WiFi.scanDelete();wifiStage=0;wifiNotice=wifiCount?"":"未有網絡";restoreWifi();needsDraw=true;}else if(millis()-wifiStarted>32000){esp_wifi_scan_stop();WiFi.scanDelete();wifiStage=0;wifiNotice="搜尋失敗";restoreWifi();needsDraw=true;}}
 if(wifiStage==3&&networkWorkIdle()){motion.reset();positionTracker.reset();lastPositionPoll=0;etaEpoch=0;etaCode=0;dataStamp=0;lastPoll=0;WiFi.setAutoReconnect(false);WiFi.disconnect(false,false);WiFi.begin(wifiCandidate.c_str(),wifiCandidatePass.c_str());wifiStage=4;wifiStarted=millis();needsDraw=true;}
 if(wifiStage==4){if(millis()-wifiStarted>750&&WiFi.status()==WL_CONNECTED&&WiFi.SSID()==wifiCandidate&&WiFi.localIP()!=IPAddress(0,0,0,0)){if(wifiName.length()&&wifiName!=wifiCandidate){wifiPreviousName=wifiName;wifiPreviousPass=wifiPassword;prefs.putString("prevssid",wifiPreviousName);prefs.putString("prevpass",wifiPreviousPass);}wifiName=wifiCandidate;wifiPassword=wifiCandidatePass;prefs.putString("ssid",wifiName);prefs.putString("pass",wifiPassword);wifiCandidatePass="";wifiStage=0;wifiNotice="已連線";WiFi.setAutoReconnect(true);lastPoll=0;failures=0;needsDraw=true;}else if(millis()-wifiStarted>25000){wifiCandidatePass="";wifiStage=0;wifiNotice="連線失敗";WiFi.disconnect(false,false);restoreWifi();needsDraw=true;}}
}
void wifiCommand(JsonDocument&d){String a=d["action"]|"status";if(a=="scan")scanWifi();else if(a=="connect")requestWifi(d["ssid"]|"",d["password"]|"");else if(a=="cancel")cancelWifi();wifiReply(d["id"]|"");}
