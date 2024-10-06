#include <PCD8544.h>

#define v1_pin A0
#define v2_pin A1
#define v3_pin A2
#define v4_pin A3
#define switch_uart 8
#define switch_ac 9
#define led 13


float volt1=0;
float volt2=0;
float volt3=0;
float volt4=0;
int v;
int t_delay=100;
PCD8544 lcd;


void setup() {
  lcd.begin(84, 48);
  Serial.begin(9600);
  pinMode(v1_pin, INPUT);
  pinMode(v2_pin, INPUT);
  pinMode(v3_pin, INPUT);
  pinMode(v4_pin, INPUT);
  pinMode(switch_uart, INPUT);
  pinMode(switch_ac, INPUT);
  pinMode(led, OUTPUT);
}

// lectura del valor maximo de la onda
float volt_pico(int pin) {
  float maximo = 0;
  float minimo = 0;
  float v = 0;
  uint32_t tiempo = millis();
  while((millis() - tiempo) < 100){
    v = analogRead(pin);
    if (v > maximo) maximo = v;
  }
  return maximo;
}

float volt_rms (float tension_pico) {
  return (tension_pico)/(sqrt(2));
}


void loop() {
    if (digitalRead(switch_ac)==HIGH){
      t_delay=100;
      volt1 = volt_rms(map(volt_pico(v1_pin), 0, 1023, -2400, 2400));
      volt1 = volt1/100;
      volt2 = volt_rms(map(volt_pico(v2_pin), 0, 1023, -2400, 2400));
      volt2 = volt2/100;
      volt3 = volt_rms(map(volt_pico(v3_pin), 0, 1023, -2400, 2400));
      volt3 = volt3/100;
      volt4 = volt_rms(map(volt_pico(v4_pin), 0, 1023, -2400, 2400));
      volt4 = volt4/100;
    }
    else if (digitalRead(switch_ac)==LOW){
      t_delay=400;
      volt1 = map(analogRead(v1_pin), 0, 1023, -2400, 2400);
      volt1 = volt1/100;
      volt2 = map(analogRead(v2_pin), 0, 1023, -2400, 2400);
      volt2 = volt2/100;
      volt3 = map(analogRead(v3_pin), 0, 1023, -2400, 2400);
      volt3 = volt3/100;
      volt4 = map(analogRead(v4_pin), 0, 1023, -2400, 2400);
      volt4 = volt4/100;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.drawColumn(48,48);
    lcd.setCursor(20, 0);
    lcd.drawColumn(48,48);
    lcd.setCursor(3, 0);
    lcd.print("V1");
    lcd.setCursor(25, 0);
    lcd.print(volt1);
    lcd.print(" V");
    lcd.setCursor(3,1);
    lcd.print("V2");
    lcd.setCursor(25, 1);
    lcd.print(volt2);
    lcd.print(" V");  
    lcd.setCursor(3, 2);
    lcd.print("V3");
    lcd.setCursor(25, 2);
    lcd.print(volt3);
    lcd.print(" V"); 
    lcd.setCursor(3,3);   
    lcd.print("V4");
    lcd.setCursor(25, 3);
    lcd.print(volt4);
    lcd.print(" V");

    if(digitalRead(switch_uart) == HIGH){  
      Serial.println(volt1);
      Serial.println(volt2);
      Serial.println(volt3);
      Serial.println(volt4);
    }

    if (volt1 < -20 || volt1 > 20 || volt2 < -20 || volt2 > 20 || volt3 < -20 || volt3 > 20 || volt4 < -20 || volt4 > 20){
      digitalWrite(led, HIGH);        
    }else {
      digitalWrite(led, LOW);
    }

    delay(t_delay);
 }