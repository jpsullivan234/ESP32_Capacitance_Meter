#include <Arduino.h>

#define debugger 1
#if debugger == 1
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif

// Structs
struct doubleLong 
{
  long charged_value;
  long discharged_value;
};

// Functions
doubleLong measureADC(int num_measurements, byte charge_pin, byte sense_pin, byte charge_time_us, byte discharge_time_ms);

// Variables
const byte sensePin = 3;
const byte chargePin = 1;

const byte chargeTime_us = 30 + 54;   // instructions take ~54 us; dont change 54. Originally 30
const byte dischargeTime_ms = 40;

const byte numMeasurements = 40;
const uint16_t MAX_ADC_VALUE = 4095;
const float resistor_mohm = 2.035;
const float Vref = 5.0;

float capacitance_pf = 0.0;
float capVoltage = 0.0;


void setup() {
  Serial.begin(1000000L);
  delay(500);                   // delay to allow serial to begin

  pinMode(sensePin, INPUT);
  pinMode(chargePin,OUTPUT);

  digitalWrite(chargePin,LOW);  // begin discharging
  delay(dischargeTime_ms);

  debugln("Start");
}

void loop() {
  doubleLong ADCvalues = measureADC(
    numMeasurements,
    chargePin,
    sensePin,
    chargeTime_us,
    dischargeTime_ms 
  );

  // calculate capacitance
  capVoltage = (Vref * ADCvalues.charged_value) / (float)MAX_ADC_VALUE;
  capacitance_pf = -1.0 * ((chargeTime_us) / resistor_mohm ) / log(1 - (capVoltage / Vref));
  if (capacitance_pf > 0.0) capacitance_pf -= 9.0; // -= 9.0 when charge time is 30

  // capacitance is 8 pf too high (probably due to parasitic cap of breadboard)
  debug(capacitance_pf ); debug("pf | "); debug(ADCvalues.charged_value); debug(" | "); debugln(ADCvalues.discharged_value);

  delay(1);
}

doubleLong measureADC(
                      int num_measurements, 
                      byte charge_pin, 
                      byte sense_pin, 
                      byte charge_time_us,
                      byte discharge_time_ms
                      )
{
  long charged_val;
  long discharged_val;

  for (int i = 0; i < num_measurements; i++)
  {
    digitalWrite(charge_pin, HIGH);      // charge
    delayMicroseconds(charge_time_us - 54);
    charged_val += analogRead(sense_pin);     // read value and store            

    digitalWrite(charge_pin, LOW);       // discharge
    delay(discharge_time_ms);

    discharged_val += analogRead(sense_pin);
    if (analogRead(sense_pin) > 150) debugln("error discharged voltage too high");
  }
  return {charged_val /= num_measurements, discharged_val /= num_measurements};
}
