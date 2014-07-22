/*Copyright (c) 2010 bildr community

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "mpr121.h"
#include <Wire.h>

#define KEY_LEFT_CTRL	0x01
#define KEY_LEFT_SHIFT	0x02
#define KEY_RIGHT_CTRL	0x10
#define KEY_RIGHT_SHIFT	0x20
uint8_t buf[8] = { 0 };

uint8_t keys[24][2] = { 
  { 0, 0x0E}, // Raise Shield
  { 0, 0x0F}, // Lower Shield
  { 0, 0x05}, // Auto Beams
  { 0, 0x50}, // Decr Freq
  { 0, 0x4f}, // Incr Freq
  { 0, 0x17}, // Zoom In
  { 0, 0x1C}, // Zoom Out
  { 0, 0x36}, // Weapons View
  { 0, 0x37}, // LRS View Button
  { 0, 0x38}, // Visual View Button
  { KEY_RIGHT_SHIFT, 0x18}, // Torp to ENE
  { KEY_RIGHT_SHIFT, 0x0c}, // ENE to Torp
  { 0, 0x1e}, // Homing
  { 0, 0x1f}, // Nuke
  { 0, 0x20}, // Mine
  { 0, 0x21}, // EMP
  { 0, 0x24}, // Load 1
  { KEY_RIGHT_SHIFT, 0x24}, // Fire 1  
  { 0, 0x25}, // Load 2
  { KEY_RIGHT_SHIFT, 0x25}, // Fire 2
  { 0, 0x26}, // Load 3
  { KEY_RIGHT_SHIFT, 0x26}, // Fire 3
  { 0, 0x27}, // Load 4
  { KEY_RIGHT_SHIFT, 0x27} // Fire 4
};


int irqpin1 = 2;  // Digital 2
int irqpin2 = 3;
boolean touchStates[24]; //to keep track of the previous touch states

void setup(){
  pinMode(irqpin1, INPUT_PULLUP);
  pinMode(irqpin2, INPUT_PULLUP);

  
  Serial.begin(9600);
  Wire.begin();

  mpr121_setup();
}

void loop(){
  readTouchInputs();
}


void readTouchInputs(){
  uint8_t address = checkInterrupt();
  if(address){
    int incr = 0;
    if (address == 0x5B){incr = 12;}
    //read the touch state from the MPR121
    Wire.requestFrom((int)address,2); 
    
    byte LSB = Wire.read();
    byte MSB = Wire.read();
    
    uint16_t touched = ((MSB << 8) | LSB); //16bits that make up the touch states

    
    for (int i=0; i < 12; i++){  // Check what electrodes were pressed
      if(touched & (1<<i)){
        if(touchStates[i+incr] == 0){
          //pin i was just touched
          sendKey(keys[i+incr][0], keys[i+incr][1]);
        }  
        touchStates[i+incr] = 1;      
      }else{
        touchStates[i+incr] = 0;
      }
    
    }
    
  }
}

void sendKey(uint8_t modifier, uint8_t key) {
  buf[0] = modifier;
  buf[2] = key;
  Serial.write(buf, 8);
  buf[0] = 0;
  buf[2] = 0;
  Serial.write(buf, 8);
}

void mpr121_setup(void){

  set_register(0x5A, ELE_CFG, 0x00); 
  
  // Section A - Controls filtering when data is > baseline.
  set_register(0x5A, MHD_R, 0x01);
  set_register(0x5A, NHD_R, 0x01);
  set_register(0x5A, NCL_R, 0x00);
  set_register(0x5A, FDL_R, 0x00);
  set_register(0x5B, MHD_R, 0x01);
  set_register(0x5B, NHD_R, 0x01);
  set_register(0x5B, NCL_R, 0x00);
  set_register(0x5B, FDL_R, 0x00);

  // Section B - Controls filtering when data is < baseline.
  set_register(0x5A, MHD_F, 0x01);
  set_register(0x5A, NHD_F, 0x01);
  set_register(0x5A, NCL_F, 0xFF);
  set_register(0x5A, FDL_F, 0x02);
  set_register(0x5B, MHD_F, 0x01);
  set_register(0x5B, NHD_F, 0x01);
  set_register(0x5B, NCL_F, 0xFF);
  set_register(0x5B, FDL_F, 0x02);
  
  //All Stop Button
  set_register(0x5A, ELE0_T, TOU_THRESH);
  set_register(0x5A, ELE0_R, REL_THRESH);
 
  //Reverse Button
  set_register(0x5A, ELE1_T, TOU_THRESH);
  set_register(0x5A, ELE1_R, REL_THRESH);
  
  //Request Dock Button
  set_register(0x5A, ELE2_T, TOU_THRESH);
  set_register(0x5A, ELE2_R, REL_THRESH);
  
  //Zoom In Button
  set_register(0x5A, ELE3_T, TOU_THRESH);
  set_register(0x5A, ELE3_R, REL_THRESH);
  
  //Zoom Out Button
  set_register(0x5A, ELE4_T, TOU_THRESH);
  set_register(0x5A, ELE4_R, REL_THRESH);
  
  //Helm Button
  set_register(0x5A, ELE5_T, TOU_THRESH);
  set_register(0x5A, ELE5_R, REL_THRESH);
  
  //LRS Button
  set_register(0x5A, ELE6_T, TOU_THRESH);
  set_register(0x5A, ELE6_R, REL_THRESH);
  
  //Visual Button
  set_register(0x5A, ELE7_T, TOU_THRESH);
  set_register(0x5A, ELE7_R, REL_THRESH);
 
  //Warp 1 Shield Button
  set_register(0x5A, ELE8_T, TOU_THRESH);
  set_register(0x5A, ELE8_R, REL_THRESH);
 
  //Warp 2 Button
  set_register(0x5A, ELE9_T, TOU_THRESH);
  set_register(0x5A, ELE9_R, REL_THRESH);
  
  
  //Warp 3 Button
  set_register(0x5A, ELE10_T, TOU_THRESH);
  set_register(0x5A, ELE10_R, REL_THRESH);
  
  
  //Warp 4 Button
  set_register(0x5A, ELE11_T, TOU_THRESH);
  set_register(0x5A, ELE11_R, REL_THRESH);

//0x5B

  //All Stop Button
  set_register(0x5B, ELE0_T, TOU_THRESH);
  set_register(0x5B, ELE0_R, REL_THRESH);
 
  //Reverse Button
  set_register(0x5B, ELE1_T, TOU_THRESH);
  set_register(0x5B, ELE1_R, REL_THRESH);
  
  //Request Dock Button
  set_register(0x5B, ELE2_T, TOU_THRESH);
  set_register(0x5B, ELE2_R, REL_THRESH);
  
  //Zoom In Button
  set_register(0x5B, ELE3_T, TOU_THRESH);
  set_register(0x5B, ELE3_R, REL_THRESH);
  
  //Zoom Out Button
  set_register(0x5B, ELE4_T, TOU_THRESH);
  set_register(0x5B, ELE4_R, REL_THRESH);
  
  //Helm Button
  set_register(0x5B, ELE5_T, TOU_THRESH);
  set_register(0x5B, ELE5_R, REL_THRESH);
  
  //LRS Button
  set_register(0x5B, ELE6_T, TOU_THRESH);
  set_register(0x5B, ELE6_R, REL_THRESH);
  
  //Visual Button
  set_register(0x5B, ELE7_T, TOU_THRESH);
  set_register(0x5B, ELE7_R, REL_THRESH);
 
  //Warp 1 Shield Button
  set_register(0x5B, ELE8_T, TOU_THRESH);
  set_register(0x5B, ELE8_R, REL_THRESH);
 
  //Warp 2 Button
  set_register(0x5B, ELE9_T, TOU_THRESH);
  set_register(0x5B, ELE9_R, REL_THRESH);
  
  
  //Warp 3 Button
  set_register(0x5B, ELE10_T, TOU_THRESH);
  set_register(0x5B, ELE10_R, REL_THRESH);
  
  
  //Warp 4 Button
  set_register(0x5B, ELE11_T, TOU_THRESH);
  set_register(0x5B, ELE11_R, REL_THRESH);

  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(0x5A, FIL_CFG, 0x04);
  
  // Section E
  // Electrode Configuration
  // Set ELE_CFG to 0x00 to return to standby mode
  set_register(0x5A, ELE_CFG, 0x0C);  // Enables 12 inputs
  set_register(0x5B, ELE_CFG, 0x0C);
}


uint8_t checkInterrupt(void){
  if (!digitalRead(irqpin1)) {
    return 0x5A;
  }
  if (!digitalRead(irqpin2)) {
    return 0x5B;
  }
}


void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}
