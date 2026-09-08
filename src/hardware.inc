void panelCommand(uint8_t command, std::initializer_list<uint8_t> values) {
  uint8_t buffer[16]; size_t n=0;
  for (uint8_t v : values) buffer[n++]=v;
  lcd.sendCommand(command, buffer, n);
}

void hardwareInit() {
  Serial.begin(115200);
  // Keep the amplifier disabled; no speaker is connected yet.
  pinMode(4, OUTPUT); digitalWrite(4, HIGH);
  for (int pin : {22,16,17}) { pinMode(pin,OUTPUT); digitalWrite(pin,HIGH); }
  pinMode(21,OUTPUT); digitalWrite(21,LOW);
  pinMode(33,OUTPUT); digitalWrite(33,HIGH);
  lcdSPI.begin(14,12,13,15);
  lcd.init(240,320,SPI_MODE0);
  // Panel-specific tuning from LCDWIKI ST7789P3_Init.txt.
  panelCommand(0xB2,{0x0C,0x0C,0x00,0x33,0x33});
  panelCommand(0xB7,{0x55}); panelCommand(0xBB,{0x1D});
  panelCommand(0xC0,{0x2C}); panelCommand(0xC2,{0x01});
  panelCommand(0xC3,{0x0F}); panelCommand(0xC4,{0x20});
  panelCommand(0xC6,{0x0F}); panelCommand(0xD0,{0xA4,0xA1});
  panelCommand(0xD6,{0xA1});
  panelCommand(0xE0,{0xF0,0x06,0x12,0x15,0x15,0x1E,0x3E,0x44,0x4E,0x09,0x11,0x10,0x1A,0x1D});
  panelCommand(0xE1,{0xF0,0x04,0x0E,0x0D,0x0E,0x27,0x3F,0x54,0x4E,0x3F,0x1D,0x1D,0x22,0x27});
  lcd.invertDisplay(false); lcd.setRotation(1);
  touchSPI.begin(25,39,32,33); pinMode(36,INPUT);
  ledcSetup(0,5000,8);ledcAttachPin(21,0);ledcWrite(0,190);
}
