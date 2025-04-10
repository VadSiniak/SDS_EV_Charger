#include <TimerOne.h>
#include <LiquidCrystal.h>
#include <PZEM004Tv30.h>
//---------------PZEM-------------
#include <SoftwareSerial.h>

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 9
#define PZEM_TX_PIN 8
#endif

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);
//----------Could be changed-----------------
const int min_volt = 202;
volatile byte amper = 6;
int cur_load = 192;
int old_load = 192;
char ver[]="1.3";
//----------Do not change----------------
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int analogPin = A6; 
int state = 0;
int oldState = 0;
int minus = 1;
int plus = 1;
int i = 0;
float i_energy = 0;
float c_energy = 0;
int voltage = 0;
int current = 0;
float energy = 0;
int cents = 0;
int cents2 = 0;
int seconds = 0;
int minutes = 0;
int hours = 0;
const int interval = 10; //Every 10 milliseconds i increase  1 cent
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
int c1, c2, s1, s2, m1, m2, h; //Variables used to put in the form 
                              //h:m2m1:s2s1:c2c1

byte customChar0[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

void setup(){
    // set up the LCD's number of columns and rows:
lcd.begin(20, 2);
//Serial.begin(9600);
lcd.setCursor(0, 0);
lcd.print("SDS EV Charger");
lcd.setCursor(0, 1);
lcd.print("Home edition v");
lcd.print(ver);
pinMode(6,INPUT_PULLUP);
pinMode(7,INPUT_PULLUP);
pinMode(10,OUTPUT);
Timer1.initialize(1000);  // Frequency, 1000us = 1khz
//Timer1.pwm(10,512);       // 50% DC on pin 9 (255-16,384-24,512-32)  
delay(3000);
i_energy = pzem.energy(); // initial energy from eeprom of pzem

lcd.setCursor(0, 0);
lcd.print("                    ");
lcd.setCursor(0, 1);
lcd.print("                    ");
lcd.setCursor(0, 0);
lcd.print("Overall: ");
lcd.print(i_energy,1);
lcd.print("kWh");
//------Loading bar--------
lcd.createChar(0, customChar0); // create a new custom character (index 0)
while (i < 20) {
    lcd.setCursor(i, 1);
    lcd.write((byte)0);
    delay(200);
    i++;
    start_debug_mode();
}
//--------------------------

digitalWrite(10, HIGH);
lcd.setCursor(0, 0);
lcd.print("                    ");
lcd.setCursor(0, 0);
lcd.print("Not connected");
lcd.setCursor(0, 1);
lcd.print("                    ");
lcd.setCursor(0, 1);
lcd.print("0:00:00");    
lcd.setCursor(17, 1);
lcd.print("10A");

}

void loop()
{
  button();
  readState();
  if (oldState != state) {
    work();
  }
  if (state == 0 || state == 1 || state == 2) {
    nc_volt();
  }
  if (state == 3) {
    chronometer();
    check_voltage();
    if (cur_load != old_load){
      Timer1.pwm(10,cur_load);           // Start PWM on "pilot" wire
      old_load = cur_load;
    }
  }
  
}

void start_debug_mode(){
   // 6, 7
  minus = 1;
  plus = 1;
  minus = digitalRead(6);
  plus = digitalRead(7);

  if (minus == 0 && plus == 0) {
        delay(500);
        debug_mode();
   }
}

void debug_mode(){
  beep();
  while (true){  //Start custom loop function
    lcd.setCursor(0, 0);
    lcd.print("                    ");
    lcd.setCursor(0, 1);
    lcd.print("                    "); 
    lcd.setCursor(0, 0);
    lcd.print("Not connected");
    lcd.setCursor(7, 1);
    lcd.print("*Debug mode*");
    Timer1.disablePwm(10);
    analogWrite(A0, 0);
    digitalWrite(10, HIGH);
        while (true){   // "Not connected" cycle 
            int i = 0;
            int val = 0;  // variable to store the value read
            int tempval = 0;
              while (i < 31) {
                i++;
                int tempval = analogRead(analogPin);
                if (val > tempval) {
                }
                else if (val < tempval) {
                  val = tempval;
                }
              }
                lcd.setCursor(1, 1); 
                lcd.print(val);      
                delay(500);
            plus = 1;
            plus = digitalRead(7);
              if (plus == 0) {
              beep();
              break;
                }              
          }
     lcd.setCursor(0, 0);
     lcd.print("                    ");
     lcd.setCursor(0, 0);       
     lcd.print("Connected");
        while (true){  // "Connected" cycle
          int i = 0;
          int val = 0;  // variable to store the value read
          int tempval = 0;
            while (i < 31) {
              i++;
              int tempval = analogRead(analogPin);
              if (val > tempval) {
              }
              else if (val < tempval) {
                val = tempval;
              }
            }
              lcd.setCursor(1, 1); 
              lcd.print(val);      
              delay(500);
          plus = 1;
          plus = digitalRead(7);
            if (plus == 0) {
            beep();
            break;
              }              
            }
     lcd.setCursor(0, 0);
     lcd.print("                    ");
     lcd.setCursor(0, 0);       
     lcd.print("Ready to charge");
     Timer1.pwm(10,448);  // set 18A of current
        while (true){  // "Ready to charge" cycle
            int i = 0;
            int val = 0;  // variable to store the value read
            int tempval = 0;
              while (i < 31) {
                i++;
                int tempval = analogRead(analogPin);
                if (val > tempval) {
                }
                else if (val < tempval) {
                  val = tempval;
                }
              }
                lcd.setCursor(1, 1); 
                lcd.print(val);      
                delay(500);
            plus = 1;
            plus = digitalRead(7);
              if (plus == 0) {
              beep();
              break;
                }              
              }
     lcd.setCursor(0, 0);
     lcd.print("                    ");   
     lcd.setCursor(0, 0);    
     lcd.print("Charging");
     Timer1.pwm(10,160);  // set 10A of current
     analogWrite(A0, 255);
        while (true){  // "Charging" cycle
            int i = 0;
            int val = 0;  // variable to store the value read
            int tempval = 0;
              while (i < 31) {
                i++;
                int tempval = analogRead(analogPin);
                if (val > tempval) {
                }
                else if (val < tempval) {
                  val = tempval;
                }
              }
                lcd.setCursor(1, 1); 
                lcd.print(val);      
                delay(500);
            plus = 1;
            plus = digitalRead(7);
              if (plus == 0) {
              beep();                
              break;
                }              
              }
     lcd.setCursor(0, 0);
     lcd.print("                    ");   
     lcd.setCursor(0, 0);    
     lcd.print("Error!!!");
     Timer1.disablePwm(10);
     digitalWrite(10, LOW);
     delay(2000);
     analogWrite(A0, 0);
        while (true){  // "Error" cycle
            int i = 0;
            int val = 0;  // variable to store the value read
            int tempval = 0;
              while (i < 31) {
                i++;
                int tempval = analogRead(analogPin);
                if (val > tempval) {
                }
                else if (val < tempval) {
                  val = tempval;
                }
              }
                lcd.setCursor(1, 1); 
                lcd.print(val);      
                delay(500);
            plus = 1;
            plus = digitalRead(7);
              if (plus == 0) {
              beep();                
              break;
                }              
              }
  } // Close custom loop function
}

void beep(){ //This function beeping
  tone(13, 3000); // Send 1KHz sound signal...
  delay(500);        // ...for 0.5 sec
  noTone(13);
}

void check_voltage(){   //This function reading voltage from "PZEM"
  unsigned long currentMillis2 = millis();  //If for the updating. If it is true, it means 1 cent of a second had passed. Update cents, minutes, seconds, hours and then i write on the lcd
  if (currentMillis2 - previousMillis2 >= interval) {  
    previousMillis2 = currentMillis2;
    cents2++;
    if (cents2 == 100){
        cents2 = 0;
        read_voltage();
        if (voltage > 190 && voltage < min_volt) {
          cur_load -= 32;
          amper--;
            }
        }
    }
}

void read_power(){   //This function reading voltage and kWh from "PZEM"
// Read the data from the sensor
    voltage = pzem.voltage();
    current = pzem.current();
//    float power = pzem.power();
//    power = power / 1000;
    energy = pzem.energy();
    c_energy = energy - i_energy;
}

void read_voltage(void){   //This function reading voltage from "PZEM"
// Read the data from the sensor
    voltage = pzem.voltage();
}

void print_power(){  //This function printing current voltage, current and consumed kWh 
        lcd.setCursor(12, 0);
        lcd.print("     "); 
        lcd.setCursor(12, 0);
        lcd.print(voltage);    
        lcd.print("V");
        lcd.setCursor(17, 0);
        lcd.print("   "); 
        lcd.setCursor(17, 0);
        if (current < 10) lcd.print("0");  
        lcd.print(current); 
        lcd.print("A");
        lcd.setCursor(8, 1);
        lcd.print("         "); 
        lcd.setCursor(8, 1);
        lcd.print(c_energy,1); 
        lcd.print("kWh");
        
//        lcd.print(power,1); Serial.println("kW");
}

void readState() {   //This function reading voltage from "pilot" wire and setting "state" value
  int i = 0;
  int val = 0;  // variable to store the value read
  int tempval = 0;
  while (i < 31) {
    i++;
    int tempval = analogRead(analogPin);
    if (val > tempval) {
    }
    else if (val < tempval) {
      val = tempval;
    }
   }

//  lcd.setCursor(8, 1); // temp ---------------------------------
//  lcd.print(val);      // temp ---------------------------------
//  delay(500);         // temp ---------------------------------

  if (val > 880){   // 897-915
    state = 0;
  }
  else if (val < 880 && val > 810 ){ // 804  Connected
    state = 1;
  }
  else if (val < 811 && val > 780 ){ // 801-799 PWM is ON Ready to charge
    state = 2;
  }
  else if (val < 781 && val > 650 ){ // 715  Charging
    state = 3;
  }
  else {
    state = 4;
  }
}

void work() {        //This function is enabling charging depends on the "state"
  //8,9
  if (state == 0){
    analogWrite(A0, 0);
    Timer1.disablePwm(10);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("Not connected"); //13
    oldState = 0;
  }
  else if (state == 1){
    analogWrite(A0, 0);         
    Timer1.pwm(10,cur_load);           // Start PWM on "pilot" wire
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("Connected"); //9
    oldState = 1;
  }
  else if (state == 2){
    analogWrite(A0, 0);
    Timer1.pwm(10,cur_load);           // Start PWM on "pilot" wire
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("Ready to charge"); //15
    oldState = 2;
  }
  else if (state == 3){
    analogWrite(A0, 255);
    lcd.setCursor(0, 0);
    lcd.print("                "); 
    lcd.setCursor(0, 0);
    lcd.print("Charging..."); //11
    oldState = 3;    
  }
  else {
    analogWrite(A0, 0);  
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("Error!!!!!");
    oldState = 4;
  }
}

void nc_volt(void){   //This function print:   "Voltage" in Not Connected state
  unsigned long currentMillis = millis();  //If for the updating. If it is true, it means 1 cent of a second had passed. Update cents, minutes, seconds, hours and then i write on the lcd
  if (currentMillis - previousMillis >= interval) {  
    previousMillis = currentMillis;
    cents++;
    if (cents == 100){
        read_voltage();
        lcd.setCursor(16, 0);
        lcd.print("    "); 
        lcd.setCursor(16, 0);
        lcd.print(voltage);    
        lcd.print("V");
        cents = 0;
    }
  } 
}

void chronometer(void){   //This function print:   "Actual time"
  unsigned long currentMillis = millis();  //If for the updating. If it is true, it means 1 cent of a second had passed. Update cents, minutes, seconds, hours and then i write on the lcd
  if (currentMillis - previousMillis >= interval) {  
    previousMillis = currentMillis;
    cents++;
    if (cents == 100){
      read_power();
      print_power();
      cents = 0;
      seconds++;
      if (seconds == 60) {
        seconds = 0;
        minutes++;
        if (minutes == 60){
          minutes = 0; 
          hours++;
          if (hours == 24)
            hours = 0;
          }
      }
   }
    int cent = cents;
    int sec = seconds;
    int minu = minutes; //Taking the digits separeted
    h = hours;  //For the other funcionts, so i can put hours = 0 and h still is the last value
    c1 = cent%10;
    cent /= 10;
    c2 = cent%10;
    s1 = sec%10;
    sec /= 10;
    s2 = sec%10;
    m1 = minu%10;
    minu /= 10;
    m2 = minu%10;
    lcd.setCursor(0, 1);
    lcd.print(h);
    lcd.print(':');
    lcd.print(m2);
    lcd.print(m1);
    lcd.print(':');
    lcd.print(s2);
    lcd.print(s1);
  } 
}

void button() {     //This function is reading buttons and defining desired carging current 
  // 6, 7
  minus = 1;
  plus = 1;
  minus = digitalRead(6);
  plus = digitalRead(7);

  if (minus != plus) {

        if (minus == 0) {
          amper--;
        }
        if (plus == 0) {
          amper++;
        }

        if (amper < 1 ) {
        amper = 1;
        Timer1.pwm(10,112);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print(" 5A");
        }
        else if (amper == 1) {
        Timer1.pwm(10,112);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print(" 5A");
        }
        else if (amper == 2) {
        Timer1.pwm(10,128);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print(" 6A");
        }
        else if (amper == 3) {
        Timer1.pwm(10,1144);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print(" 7A");
        }
        else if (amper == 4) {
        Timer1.pwm(10,160);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print(" 8A");
        }
        else if (amper == 5) {
        Timer1.pwm(10,176);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print(" 9A");
        }
        else if (amper == 6) {
        Timer1.pwm(10,192);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print("10A");
        }
        else if (amper == 7) {
        Timer1.pwm(10,208);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print("11A");
        }
        else if (amper == 8) {
        Timer1.pwm(10,224);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print("12A");
        }
        else if (amper == 9) {
        Timer1.pwm(10,240);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print("13A");
        }
        else if (amper == 10) {
        Timer1.pwm(10,256);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print("14A");
        }
        else if (amper == 11) {
        Timer1.pwm(10,272);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print("15A");
        }
        else if (amper > 11) {
        amper = 11;
        Timer1.pwm(10,272);
        lcd.setCursor(17, 1);
        lcd.print("   ");
        lcd.setCursor(17, 1);
        lcd.print("15A");
        }
        delay(500);
          }
}
