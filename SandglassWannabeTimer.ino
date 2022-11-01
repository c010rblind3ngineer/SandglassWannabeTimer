/* Arduino 'Sandglass-wannabe' timer project

   Components:
                - Arduino Uno
                - ADXL335
                - I2C LCD screen (20x4)
                - Passive Buzzer
                - Push button tactile switch
                - 220Ohm resistor
                - 10KOhm resistor
                - Breadboard
                - Some jumper wires

   Created on 31 October 2022 by c010blind3ngineer
*/

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

/*
  Nokia Tune
  Connect a piezo buzzer or speaker to pin 11 or select a new pin.
  More songs available at https://github.com/robsoncouto/arduino-songs

                                              Robson Couto, 2019
*/
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

// change this to make the song slower or faster
int tempo = 180;

// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {

  // Nokia Ringtone
  // Score available at https://musescore.com/user/29944637/scores/5266155

  NOTE_E5, 8, NOTE_D5, 8, NOTE_FS4, 4, NOTE_GS4, 4,
  NOTE_CS5, 8, NOTE_B4, 8, NOTE_D4, 4, NOTE_E4, 4,
  NOTE_B4, 8, NOTE_A4, 8, NOTE_CS4, 4, NOTE_E4, 4,
  NOTE_A4, 2,
};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;

byte fillin[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

byte nofill[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

// Do note Arduino Uno only can store up to 16-bit (2-byte)
// https://www.arduino.cc/reference/en/language/variables/data-types/int/
// https://www.arduino.cc/reference/en/language/variables/data-types/unsignedint/

const int secs_per_box = 3750;  // Time for each array of pixels in one box (total of 80 array of pixel boxes on the LCD (20x4))

int Mins = 0;
int Secs = 0;
int i; // columns
int j; // rows

const int btnPin = 8;
const int buzzerPin = 9;

int X_axis = A0;
int Y_axis = A1;
int Z_axis = A2;
int x, y, z;
const int deg_acc = 10;
boolean trigCDT = false;    // Trigger countdown timer
boolean trigBeep = false;   // Trigger tone when timer ends

unsigned long trigStart;
unsigned long trigEnd;
unsigned int t = 0;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, fillin);
  lcd.createChar(1, nofill);
  Serial.begin(9600);
  pinMode(btnPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.setCursor(0, 0);
  lcd.print("Press button");
  lcd.setCursor(0, 1);
  lcd.print("to start...");
  while (digitalRead(btnPin) != HIGH) {}
  lcd.clear();
}

void loop() {
  if (digitalRead(btnPin) == HIGH) {
    lcd.clear();
    Serial.print("Calibrating");
    delay(500);
    t = 0;
    tone(buzzerPin, 2000);
    delay(100);
    noTone(buzzerPin);
    delay(100);
    tone(buzzerPin, 2000);
    delay(100);
    noTone(buzzerPin);

    lcd.setCursor(0, 0);
    lcd.print("Please flip device");
    lcd.setCursor(0, 1);
    lcd.print("to confirm...");

    trigCDT = false;
    trigBeep = false;
  }
  // Read XYZ axis values
  x = analogRead(X_axis);
  y = analogRead(Y_axis);
  z = analogRead(Z_axis);
  // Display XYZ axis values on Serial Monitor
  Serial.print(x);
  Serial.print("\t");
  Serial.print(y);
  Serial.print("\t");
  Serial.print(z);
  Serial.println();

  // User has to hold the device upside down for 3 secs to verify that they want to start the timer
  if (z < 290) {
    while (t < 1) { // Detect the time/moment the User 'flipped' the device upside down
      lcd.clear();
      trigStart = millis();
      t = 1;
    }
    trigEnd = millis();   // This here to compare the current time and the 'flipped' time below

    // Compare the 'flipped' time vs current time,
    // check if it already exceeded the time limit (3 secs) for verification.
    // Then start the countdown timer after 1 sec delay.
    if (trigEnd - trigStart > 3000) {
      lcd.setCursor(0, 0);
      lcd.print("You can put the");
      lcd.setCursor(0, 1);
      lcd.print("device down...");
      tone(buzzerPin, 2000);
      delay(3000);
      noTone(buzzerPin);
      lcd.clear();
      stbar_pos1();
      delay(1000);
      trigCDT = true;
    }
  }

  // Start countdown timer when the device is FLIPPED
  if (trigCDT == true) {
    // Beep three times before start of countdown timer
    tone(buzzerPin, 2000);
    delay(500);
    noTone(buzzerPin);
    delay(500);
    tone(buzzerPin, 2000);
    delay(500);
    noTone(buzzerPin);
    delay(500);
    tone(buzzerPin, 2000);
    delay(500);
    noTone(buzzerPin);

    // Start countdown timer function
    ctndownbar_pos1();

    // Once timer ends, show message on LCD and trigger tone
    lcd.setCursor(0, 0);
    lcd.print("Time's Up!");
    trigCDT = false;
    trigBeep = true;
  }
  if (trigBeep == true) {
    // I chose to have the Nokia ringtone :)
    // You can uncomment the below lines to get the default beeping tones.
    // Once the ringtone is playing, to stop just LONG PRESS the button.
    timer_ringtone();

    //    tone(buzzerPin, 3000);
    //    delay(500);
    //    noTone(buzzerPin);
  }
  delay(100);
}

// Startup loading bar during initialization
void stbar_pos1() {
  for (i = 19; i >= 0; i--) {  //columns
    for (j = 3; j >= 0; j--) { //rows
      lcd.setCursor(i, j);
      lcd.write(byte(0));
      delay(20);
    }
  }
}

// Countdown bar when the timer starts
void ctndownbar_pos1() {
  for (i = 0; i < 20; i ++) { //columns
    for (j = 0; j < 4; j++) { //rows
      delay(secs_per_box);
      lcd.setCursor(i, j);
      lcd.write(byte(1));
    }
  }
}

// Nokia ringtone
void timer_ringtone() {
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzerPin, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(buzzerPin);
  }
}
