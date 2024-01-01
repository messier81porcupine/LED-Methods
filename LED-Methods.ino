//****HARDWARE LAYOUT****//
/*
Arduino Nano Every
Interrupt button connected to ground and Digital Pin 2 (no other connections, just two wires)
WS2812B LED Strip (300 LEDs) wired to 5V and ground (different ground than button) with Data Wire connected to Digital Pin 3
100 µF capacitor placed between LED ground and 5V pins to smooth out power supply
*/



#include <FastLED.h>
#include <EEPROM.h>

#define NUM_LEDS 300

#define DATA_PIN 3

#define BUTTON 2

// Define the array of leds
CRGB leds[NUM_LEDS];


int volatile selected;
bool volatile end = false;

//Ant Variables - ag and b are function specific - passed as params - k is global and used by methods and kflip and kreset so it needs to be global, starts at 0 
int k = 0; // Turns every nth LED off
// int g = 6; // Size of colored section
// int b = 3; // Size of other colored sections - eats into g size; set b = 0 to have g = g; otherwise g will appear as g - b; adjust g accordingly (desired g of 7 andb of 2 means g should be 9)
// b is also used to flip k so as to make it set what was previously the first color to the other in cautionJump


// Blast Variables
int blastColor[] = {
  15, 255, 255
};

// Solid Rainbow Variables
int count = 0; 

int delayMS = 50;
int sat = 255;  int val = 255; // set default saturation and value (brightness)

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.clearData();
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt (digitalPinToInterrupt (BUTTON), changeEffect, RISING); // pressed
  
  selected = 0;
  // EEPROM.get(0,selected); 

}

void loop() {
  // Serial.println(selected);
  end = false;
  if(selected>7) { 
    selected=0;
    // EEPROM.put(0,selected); 
  } 
  switch (selected) {
    case 0:
      runAnt(7, 3, 35);
      break;
    case 1:
      cautionAnt(7, 3, 35);
      break;
    case 2:
      cautionJump(14, 7, 500);
      break;
    case 3:
      rainbowAnt(8, 5, 20);
      break;
    case 4:
      rainbowShotExplosion(10, 0, 1, 20, true, true);
      break;
    case 5:
      sizeableBounceableShot(1, 5, true);
      break;
    case 6:
      runSolidRainbow(10, true, false, true);
      break;
    case 7:
      setSolid(15, sat, 100);
      break;
  }
}

void changeEffect() {
  if (digitalRead (BUTTON) == HIGH) {
    selected++;
    end = true;
    FastLED.clearData();
    
    // EEPROM.put(0, selected);
    // asm volatile ("  jmp 0");
  }
}


void runAnt(int g, int b, int delayMS){
  
  for (int i = 0; i < NUM_LEDS; i++) {
    if((i - k)% g ==0) { 
      for (int j = 0; j < b; j++){
        if (i - j >= 0){
          leds[i - j] = CRGB::Black;
        }
      }
    }
    else{leds[i] = CHSV(0, sat, val);}

  } 
  FastLED.show();
  delay(delayMS);
  kReset(g);


}
void cautionAnt(int g, int b, int delayMS){
  
  for (int i = 0; i < NUM_LEDS; i++) {
    if((i - k)% g ==0) { 
      for (int j = 0; j < b; j++){ // set all the LEDs behind i but before b
        if (i - j >= 0){ // make sure we arent trying to set an LED that isnt on the strip
          leds[i - j].setHSV(25, sat, 150);
        }
      }
    }
    else{leds[i] = CHSV(0, sat, val);}
  }
  FastLED.show();
  delay(delayMS);
  kReset(g);
}
void cautionJump(int g, int b, int delayMS){

  for (int i = 0; i < NUM_LEDS; i++) {
    if((i - k)% g == 0) { 
      for (int j = 0; j < b; j++){ // set all the LEDs behind i but before b
        if (i - j >= 0){ // make sure we arent trying to set an LED that isnt on the strip
          leds[i - j].setHSV(25, sat, 150);
        }
      }
    }
    else{leds[i] = CHSV(0, 255, 255);}
  }
  FastLED.show();
  delay(delayMS);
 
  kFlip(b);
  
}
void rainbowAnt(int g, int b, int delayMS){
  
  int hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) { // loop through strand
      if((i - k)% g ==0) { // set the blank spaces
          for (int j = 0; j < b; j++){
            if (i - j >= 0){
              leds[i - j] = CRGB::Black;
            }
          }
      }
      else{leds[i] = CHSV(hue++, sat, val);} // set the colored spaces incrementing hue with each LED

  }
  FastLED.show();
  delay(delayMS);
  kReset(g);
}
void rainbowShotExplosion(int delayMS, int delayBlastMS, int delayBlackMS, int l, bool blast, bool useCustomColor){
  //Send a shot down the strip - one pixel changing hue
  int hue = -1;
  for (int i = 0; i < NUM_LEDS; i++){
    if (end) {break;}
    hue++;
    leds[i].setHSV(hue, sat, val);
    // l doing the length
    
    if (i == 0){ // for the first LED we must turn on all the ones in the shot ahead of it - soft bounce
      for (int j = i ; j <= l; j++){ // turn them on
        leds[j].setHSV(hue, sat, val); // fills in from i to l
        FastLED.show();
        delay(delayMS);
      }
    }
    else if (i + l < NUM_LEDS){ // else prevents one lone LED on during filling up to l 
      leds[i + l].setHSV(hue, sat, val); // set LED l ahead of i for ever value of i but 0 so that we dont get a random LED on at the start
    }

    FastLED.show();
    leds[i] = CRGB::Black;
    delay(delayMS);
  }
  hue = -1;

  //Set whole strip to rainbow and then show it at one time together - explode or show progressivly (depends on blast t/f)
  for (int i = NUM_LEDS - 1; i >= 0; i--){
    if (end){break;}
    if (useCustomColor){ // uses predetermined color or rainbow if chosen
      leds[i].setHSV(blastColor[0], blastColor[1], blastColor[2]);
    }
    else {
      hue++;
      leds[i].setHSV(hue, sat, val); //THIS FUCkiNG LiNE kEePs ChANgiNg selEctED TO NoT 4 - fixed it (i was 300 which is not a valid LED number)
    }
    if (not blast){FastLED.show(); delay(delayBlastMS);}
  }
  if (blast){FastLED.show();}
  
  
  //Send a shot of black down the strip to turn off the explosion in preparation for the next shot
  for (int i = 0; i < NUM_LEDS; i++){
    if (end) {break;}
    hue++;
      
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(delayBlackMS);
  }

}
void sizeableBounceableShot(int delayMS, int l, bool bounce){
  
  int hue = -1;
  for (int i = 0; i < NUM_LEDS ; i++){ // NUM_LEDS - l cause it makes the bounce work with  l > 5; removed -l cause we need to make i reach the last led to turn it all off progressivly - makes it flow
    if (end) {break;}
    hue++;
    leds[i].setHSV(hue, sat, val);
    // l doing the length
    
    if (i == 0){ // for the first LED we must turn on all the ones in the shot ahead of it - soft bounce
      for (int j = i ; j <= l; j++){ // turn them on
        leds[j].setHSV(hue, sat, val); // fills in from i to l
        FastLED.show();
        delay(delayMS);
      }
    }
    else if (i + l < NUM_LEDS){ // else prevents one lone LED on during filling up to l 
      leds[i + l].setHSV(hue, sat, val); // set LED l ahead of i for ever value of i but 0 so that we dont get a random LED on at the start
    }
  
    FastLED.show();
    leds[i] = CRGB::Black;
    delay(delayMS);    
  }

  if (bounce){
    for (int i = NUM_LEDS - 1; i >= 0; i--){ // just reversing most stuff
      if (end) {break;}
      hue--;
      leds[i].setHSV(hue, sat, val);
      // l doing the length but backwards ig; gets funky whack logic, weird bugs but theyre gone now i think
        
      if (i == NUM_LEDS - 1){ // doing same as previous section just backwards - soft bounce
        for (int j = i; j >= NUM_LEDS - l; j--){ 
          leds[j].setHSV(hue, sat, val); 
          FastLED.show();
        }
      }
      else if (i - l >= 0){ // make sure to not set below 0
        leds[i - l].setHSV(hue, sat, val); 
      }
      
      FastLED.show();
      leds[i] = CRGB::Black;
      delay(delayMS);
    }
  }
}
void runSolidRainbow(int delayMS, bool fill, bool rainbowFill, bool preventJump){
  
  if (fill && rainbowFill){
    if (count == 0){ // fills strip with a progressive rainbow
      int hue = -1;
      for (int i = 0; i < NUM_LEDS; i++){
        if (end) {break;}
        leds[i].setHSV(hue++, sat, val);
        FastLED.show();
      }
    }
    delay(delayMS);
  }
  
  
  // now set the whole strip one color iterating through hues
  for (int h = 0; h < 255; h++){
    for (int i = 0; i < NUM_LEDS; i++){
      if (end) {break;}
      leds[i].setHSV(h, sat, val);
      if (fill && rainbowFill && preventJump || fill && not rainbowFill ){
        if (count == 0){ // shows the strip filling with hue zero to remove a weird jaring jump
          FastLED.show();
        }
      }

    }
    count++;
    FastLED.show();
  }
}
void setSolid(int H, int S, int V){
  
  for (int i =0 ; i < NUM_LEDS; i++){
    leds[i].setHSV(H, S, V);
  }
  FastLED.show();
}

void kReset(int g){
  if (k == g){
    k = 0;
  }
  else {
    k++;
  }
}
void kFlip(int b){

  if (k == b){
    k = 0;
  }
  else {
    k = b;
  }  
}

