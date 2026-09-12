#include "../src/wifi_input.h"
#include <cassert>
#include <string>
#include <cstdio>
int main(){assert(validWifiCredentials("Home",""));assert(validWifiCredentials("Phone","Ab12!xyz"));assert(!validWifiCredentials("","Ab12!xyz"));assert(!validWifiCredentials("Phone","1234567"));assert(validWifiCredentials(std::string(32,'n').c_str(),std::string(63,'p').c_str()));assert(!validWifiCredentials(std::string(33,'n').c_str(),"12345678"));assert(!validWifiCredentials("Phone",std::string(64,'p').c_str()));assert(!validWifiCredentials("一二三四五六七八九十一","12345678"));
 assert(storedWifiName("Home","Phone","Home"));assert(storedWifiName("Phone","Phone","Home"));assert(!storedWifiName("Guest","Phone","Home"));
 assert(activeWifiName("Phone",true,"Phone"));assert(!activeWifiName("Home",true,"Phone"));assert(!activeWifiName("Phone",false,"Phone"));
 puts("Wi-Fi credential limits, saved profiles and single active network PASS");}
