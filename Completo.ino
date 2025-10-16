#include <OneWire.h>
#include <DallasTemperature.h> 

#define ONE_WIRE_BUS 4
const uint8_t MOSFET_PIN[4] = {11, 10, 6, 5};
const uint8_t ESTOP_PIN     = 2;

DeviceAddress SENS_ADDR[4] = {
  //{0x28,0x61,0x64,0x35,0xCB,0x5D,0xF6,0xE8}, // 1 SX alto
  {0x28,0x61,0x64,0x35,0xC5,0xA3,0x78,0xCA}, // 1 SX alto 
  {0x28,0x61,0x64,0x35,0xCB,0x34,0xE1,0x1E}, // 2 SX basso
  {0x28,0x61,0x64,0x35,0xCB,0x39,0x8B,0x8C}, // 3 DX alto
  {0x28,0x61,0x64,0x35,0xCB,0x50,0xC6,0xDF}  // 4 DX basso
};

// --- Serial ---
static const uint32_t BAUD = 250000;

// --- DS18B20 ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
static const uint16_t CONV_MS = 750;   // 12-bit
uint32_t convStart = 0;
bool     waiting   = false;

// --- Control ---
static const float MIN_DEFAULT = 33.0f;
static const float HYST        = 0.2f; // ±0.3 °C
static const float SOFT_CUT    = 90.0f;
static const float HARD_CUT    = 95.0f;

// --- State ---
float   tgt[4]      = {MIN_DEFAULT,MIN_DEFAULT,MIN_DEFAULT,MIN_DEFAULT};
float   lastT[4]    = {NAN,NAN,NAN,NAN};
bool    valid[4]    = {false,false,false,false};
uint8_t pwm[4]      = {0,0,0,0};
bool    offHold[4]  = {false,false,false,false};
bool    hardFault[4]= {false,false,false,false};
bool    estopLatch  = false;

// --- Utils ---
static inline uint8_t clampPWM(int v){ return (v<0)?0:((v>255)?255:v); }
static inline void setPWM(uint8_t i, uint8_t d){ pwm[i]=d; analogWrite(MOSFET_PIN[i], d); }
static inline void allPWM(uint8_t d){ for(uint8_t i=0;i<4;i++) setPWM(i,d); }

// --- E-STOP ---
void checkEstop(){
  if (estopLatch) return;
  if (digitalRead(ESTOP_PIN)==LOW){
    delay(10);
    if (digitalRead(ESTOP_PIN)==LOW){
      estopLatch = true;
      allPWM(0);
      Serial.println(F("ESTOP=1"));
    }
  }
}

// --- DS18B20 ---
void startConv(){ sensors.requestTemperatures(); convStart=millis(); waiting=true; }
void tryRead(){
  if(!waiting) return;
  if(millis()-convStart < CONV_MS) return;
  waiting=false;
  for(uint8_t i=0;i<4;i++){
    float t = sensors.getTempC(SENS_ADDR[i]);
    if (t>-55 && t<125){ lastT[i]=t; valid[i]=true; }
    else               { lastT[i]=NAN; valid[i]=false; }
  }
  // Telemetria: subito dopo aver aggiornato tutte le letture
  for(uint8_t i=0;i<4;i++){
    bool on = (pwm[i]>0) && !estopLatch && !hardFault[i] && !offHold[i];
    Serial.print(i+1); Serial.print(F(": "));
    Serial.print(on?F("ON "):F("OFF "));
    if (valid[i]) Serial.println(lastT[i],1);
    else Serial.println(F("NA"));
  }
}

// --- Control bang-bang + safety ---
void serviceControl(){
  if (estopLatch){
    for(uint8_t i=0;i<4;i++) setPWM(i,0);
    return;
  }
  for(uint8_t i=0;i<4;i++){
    if (hardFault[i] || offHold[i]){ setPWM(i,0); continue; }
    if (!valid[i]){ setPWM(i, clampPWM(pwm[i]-8)); continue; }

    float T = lastT[i];
    if (T >= HARD_CUT){ hardFault[i]=true; setPWM(i,0); continue; }
    if (T >= SOFT_CUT){ setPWM(i,0); continue; } // prudenza

    float lo = tgt[i]-HYST, hi = tgt[i]+HYST;
    if (T < lo)      setPWM(i,255);   // sempre 255 in salita
    else if (T > hi) setPWM(i,0);
    // altrimenti mantieni il pwm attuale (niente dithering)
  }
}

// --- Parser comandi ---
void parseTA(char* eq){
  // TA=a,b,c,d
  float v[4] = {tgt[0],tgt[1],tgt[2],tgt[3]};
  char* p = eq+1;
  for(uint8_t i=0;i<4;i++){
    v[i] = atof(p);
    char* c = strchr(p, i<3 ? ',' : '\0');
    if (!c && i<3) return;
    if (c) p = c+1;
  }
  for(uint8_t i=0;i<4;i++){
    if (v[i] <= 0.01f){ offHold[i]=true; setPWM(i,0); }
    else { offHold[i]=false; tgt[i]=v[i]; }
  }
}

void parseLine(char* line){
  if (estopLatch) return;
  if (line[0]=='T' || line[0]=='t'){
    if (line[1]=='A' || line[1]=='a'){
      char* eq = strchr(line,'=');
      if (eq) parseTA(eq);
      return;
    }
    int n = line[1]-'0';
    if (n<1 || n>4) return;
    char* eq = strchr(line,'=');
    if (!eq) return;
    float v = atof(eq+1);
    uint8_t i = n-1;
    if (v<=0.01f){ offHold[i]=true; setPWM(i,0); }
    else { offHold[i]=false; tgt[i]=v; }
  } else if (strcmp_P(line, PSTR("ALL_OFF"))==0){
    for(uint8_t i=0;i<4;i++){ offHold[i]=true; setPWM(i,0); }
  }
}

void serviceSerial(){
  static char buf[40]; static uint8_t pos=0;
  while (Serial.available()){
    char c = Serial.read();
    if (c=='\r') continue;
    if (c=='\n'){ buf[pos]=0; if (pos>0) parseLine(buf); pos=0; }
    else if (pos<sizeof(buf)-1){ buf[pos++]=c; }
  }
}

// --- Setup/loop ---
void setup(){
  Serial.begin(BAUD);
  pinMode(ESTOP_PIN, INPUT_PULLUP);
  for(uint8_t i=0;i<4;i++){ pinMode(MOSFET_PIN[i],OUTPUT); setPWM(i,0); }

  sensors.begin(); sensors.setResolution(12); sensors.setWaitForConversion(false);

  for(uint8_t i=0;i<4;i++){ tgt[i]=MIN_DEFAULT; offHold[i]=false; hardFault[i]=false; }
  Serial.println(F("ESTOP=0")); // stato iniziale
  startConv();
}
void loop(){
  checkEstop();
  serviceSerial();
  tryRead();
  serviceControl();
  if (!waiting) startConv();
}
