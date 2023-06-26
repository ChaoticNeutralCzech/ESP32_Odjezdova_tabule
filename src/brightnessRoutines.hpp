#define LUX_pin 34         //ADC1_6
#define BL_pin 21
const int PWMchannel = 0;
const int freq = 1000;
const int resolution = 12;

int measureLight() {
  return analogRead(LUX_pin);
}

int light = measureLight();
int lightRoll;     //rolling sum of light history
int lightHistory[256];
uint8_t lightPhase = 0;

void setupPins(){
  pinMode(LUX_pin, INPUT);    //Photoresistor input
  pinMode(BL_pin, OUTPUT);    //BL_pin is output
//  digitalWrite(BL_pin, LOW);  //Turns off BL before display is initialized, later will be PWM-driven
  ledcSetup(PWMchannel,freq,resolution);
  ledcAttachPin(BL_pin, PWMchannel);
  ledcWrite(PWMchannel, 0);
  for (uint8_t i = 0; i < lightAge; i++) lightHistory[i] = light;
  lightRoll = lightAge * light;
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



int updateRollingSum()
{
  int lightnow = measureLight();
  lightRoll -= lightHistory[lightPhase];
  lightHistory[lightPhase] = lightnow;
  lightRoll += lightnow;
  lightPhase = (++lightPhase % lightAge);
  return lightRoll / lightAge;
}
int param_a = 300;
int param_b = 2700;
int param_c = 6077;

int linearize(int l)
{
  return (((param_a*l/100)*l)/10000-param_b*l/1000+param_c);
}   