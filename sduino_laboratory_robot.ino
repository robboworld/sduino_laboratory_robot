// digital input pin
#define BUTTON 12

#define button_crest1 8
#define button_crest2 9
#define button_crest3 10
#define button_crest4 11

//параметры сдвигового регистра
#define data 2
#define clock 4

// analog input pin for laboratory
#define SLIDER 2
#define LIGHT  4
#define SOUND  3
#define R_A 0
#define R_B 1
#define R_C 5
#define R_D 5// Arduino Diecimila has 6 analog inputs

#define FIRMWAEW_ID 4  // ScratchBoard 1.1 Firmware ID

// analog input pin for robot
#define ANALOG0 0
#define ANALOG1 1
#define ANALOG2 2
#define ANALOG3 3
#define ANALOG4 4
#define ANALOG5 5

#define FIRMWAEW_ID 4  // ScratchBoard 1.1 Firmware ID

#define PWM1_PIN 6
#define PWM2_PIN 5
#define WAY1_PIN 7
#define WAY2_PIN 4

#define req_scratchboard 0  // request messge from Scratch
#define mask_scratcharduino 240  // request mask of Scratch+Ardunio
#define ch_analog0 0
#define ch_analog1 1
#define ch_analog2 2
#define ch_analog3 3
#define ch_analog4 4
#define ch_analog5 5
#define ch_button_rob 6
#define ch_firmware 15

// laboratory channels
#define ch_r_D 0
#define ch_r_C 1
#define ch_r_B 2
#define ch_button_lab 3
#define ch_r_A 4
#define ch_light 5
#define ch_sound 6
#define ch_slider 7

#define LAB 0
#define ROBOT 1

byte device = ROBOT;
int sensorValue = 0;  // sensor value to send
byte inByte = 0;  // incoming serial byte
byte motorDirection = 0;
byte isMotorOn = 0;
byte motorPower = 200;

void setup()
{
  // start serial port at 38400 bps:
  Serial.begin(38400);

  //find if we are running on robot or laboratory
  digitalWrite(14 + SOUND, HIGH);
  if (analogRead(SOUND) <600 && analogRead(LIGHT) > 200) {
    device = LAB;
  }
  digitalWrite(14 + SOUND, LOW);

  switch (device) {
    case LAB:
      digitalWrite(14 + R_C, HIGH);
      digitalWrite(14 + R_D, HIGH);

      //Пускаем волну светодиодов, показывая что загрузилось и Вырубаем сдвиговый регистр светодиодов

      pinMode(clock, OUTPUT); // make the clock pin an output
      pinMode(data , OUTPUT); // make the data pin an output

      //волна светодиодов
      for(int i = 0; i < 8; ++i) {//for 0 - 7 do
        shiftOut(data, clock, MSBFIRST, 1 << i); // bit shift a logic high (1) value by i
        delay(100);
      }
  
      shiftOut(data, clock, LSBFIRST, B00000000); // send this binary value to the shift register

      //Отдаем управление кнопке назад
      pinMode(BUTTON, INPUT);   // digital sensor is on digital pin 2
      digitalWrite(BUTTON, HIGH);
  
      break;

    case ROBOT:
      pinMode(PWM1_PIN, OUTPUT);
      pinMode(PWM2_PIN, OUTPUT);
      pinMode(WAY1_PIN, OUTPUT);
      pinMode(WAY2_PIN, OUTPUT);
      
      break;
  }
}

void loop()
{
  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
    Serial.flush();
    if (inByte >= req_scratchboard) {

      sendValue(ch_firmware, FIRMWAEW_ID);
      delay(10);
      
      switch (device) {

        case ROBOT:
          motorDirection = (inByte >> 5) & B11;
          isMotorOn = inByte >> 7;

          switch (motorDirection) {

            case B11:
              Motor1(motorPower * isMotorOn, false);
              Motor2(motorPower * isMotorOn, false);
              break;
            case B01:
              Motor1(motorPower * isMotorOn, false);
              Motor2(motorPower * isMotorOn, true);
              break;
            case B10:
              Motor1(motorPower * isMotorOn, true);
              Motor2(motorPower * isMotorOn, false);
              break;
            case B00:
              Motor1(motorPower * isMotorOn, true);
              Motor2(motorPower * isMotorOn, true);
          }   

          // read  switch, map it to 0 or 1023L
          sensorValue = map(digitalRead(BUTTON), 0, 1, 0, 1023);  
          sendValue(ch_button_rob, sensorValue);

          sensorValue = analogRead(ANALOG0);
          sendValue(ch_analog0, sensorValue);

          sensorValue = analogRead(ANALOG1);
          sendValue(ch_analog1, sensorValue);

          sensorValue = analogRead(ANALOG2);
          sendValue(ch_analog2, sensorValue);

          sensorValue = analogRead(ANALOG3);
          sendValue(ch_analog3, sensorValue);

          sensorValue = analogRead(ANALOG4);
          sendValue(ch_analog4, sensorValue);
          
          break;

        case LAB:
          sensorValue = map(digitalRead(BUTTON), LOW, HIGH, 1023, 0);  
          sendValue(ch_button_lab, sensorValue);

          sensorValue = map(digitalRead(button_crest1), LOW, HIGH, 0, 1023);
          if (sensorValue < 1023) {sensorValue = analogRead(R_A);}
          sendValue(ch_r_A, sensorValue);
          delay(10);

          sensorValue = map(digitalRead(button_crest2), LOW, HIGH, 0, 1023);
          if (sensorValue < 1023) {sensorValue = analogRead(R_B);}
          sendValue(ch_r_B, sensorValue);
          delay(10);

          sensorValue = map(digitalRead(button_crest3), LOW, HIGH, 0, 1023);
          if (sensorValue < 1023) {sensorValue = 1023 - analogRead(R_C);}
          sendValue(ch_r_C, sensorValue);
          delay(10);

          sensorValue = map(digitalRead(button_crest4), LOW, HIGH, 0, 1023);
          if (sensorValue < 1023) {sensorValue = 1023 - analogRead(R_D);}
          sendValue(ch_r_D, sensorValue);
          delay(10);

          sensorValue = 1023 - analogRead(LIGHT);
          sendValue(ch_light, sensorValue);
          delay(10);

          sensorValue = 2 * analogRead(SOUND);
          sendValue(ch_sound, sensorValue);
          delay(10);

          sensorValue = 1023 - analogRead(SLIDER);
          sendValue(ch_slider, sensorValue);
          delay(10);
          
          break;
      }
    }
  }
}

void Motor1(int pwm, boolean reverse)
{
  analogWrite(PWM1_PIN, pwm); //set pwm control, 0 for stop, and 255 for maximum speed
  if (reverse) {
    digitalWrite(WAY1_PIN, HIGH);
  } else {
    digitalWrite(WAY1_PIN, LOW);
  }
}
        
void Motor2(int pwm, boolean reverse)
{
  analogWrite(PWM2_PIN, pwm);
  if(reverse) {
    digitalWrite(WAY2_PIN, HIGH);
  } else {
    digitalWrite(WAY2_PIN, LOW);
  }
}

void sendValue(byte channel, int value) {
  byte high = 0;  // high byte to send
  byte low = 0;  // low byte to send
  high = (1 << 7) | (channel << 3) | (value >> 7);
  low =  (0xff >> 1) & value;
  Serial.write(high);
  Serial.write(low);
}

