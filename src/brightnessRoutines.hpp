#define LUX_pin 34         //ADC1_6
#define BL_pin 21
const int PWMchannel = 0;
const int freq = 1000;
const int resolution = 12;

void setupPins(){
  pinMode(LUX_pin, INPUT);    //Photoresistor input
  pinMode(BL_pin, OUTPUT);    //BL_pin is output
  digitalWrite(BL_pin, LOW);  //Turns off BL before display is initialized, later will be PWM-driven
}

int measureLight() {
  return analogRead(LUX_pin);
}

void setBrightness(int brightness) {
  if (brightness < 0) ledcWrite(PWMchannel, 0);
  else {
    if (brightness > 4095) ledcWrite(PWMchannel, 4095);
    else {
      ledcWrite(PWMchannel, brightness);
    }
  }
}

int light = measureLight();
int lightRoll;     //rolling sum of light history
int lightHistory[256] = {light};
uint8_t lightPhase = 0;

int updateRollingSum()
{
  int lightnow = measureLight();
  lightRoll -= lightHistory[lightPhase];
  lightHistory[lightPhase] = lightnow;
  lightRoll += lightnow;
  lightPhase = (++lightPhase % lightAge);
  return lightRoll / lightAge;
}