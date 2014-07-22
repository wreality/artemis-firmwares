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

const uint8_t elec_registers[12] = {
  0x41, 0x43, 0x45, 0x47, 0x49, 0x4B, 0x4D, 0x4F, 0x51, 0x53, 0x55, 0x57 };
//Configuration format: KeyModifier, KeyCode, TouchThreshold, ReleaseThreshold
const uint8_t keys[24][4] = { 
  { 0              , 0x0E, 0x00, 0xFF}, // Raise Shield
  { 0              , 0x0F, 0x00, 0xFF}, // Lower Shield
  { 0              , 0x05, 0x00, 0xFF}, // Auto Beams
  { 0              , 0x50, 0x00, 0xFF}, // Decr Freq
  { 0              , 0x4f, 0x00, 0xFF}, // Incr Freq
  { 0              , 0x17, 0x00, 0xFF}, // Zoom In
  { 0              , 0x1C, 0x00, 0xFF}, // Zoom Out
  { 0              , 0x36, 0x00, 0xFF}, // Weapons View
  { 0              , 0x37, 0x00, 0xFF}, // LRS View Button
  { 0              , 0x38, 0x00, 0xFF}, // Visual View Button
  { KEY_RIGHT_SHIFT, 0x18, 0x00, 0xFF}, // Torp to ENE
  { KEY_RIGHT_SHIFT, 0x0c, 0x00, 0xFF}, // ENE to Torp
  { 0              , 0x1e, 0x00, 0xFF}, // Homing
  { 0              , 0x1f, 0x00, 0xFF}, // Nuke
  { 0              , 0x20, 0x00, 0xFF}, // Mine
  { 0              , 0x21, 0x00, 0xFF}, // EMP
  { 0              , 0x24, 0x00, 0xFF}, // Load 1
  { KEY_RIGHT_SHIFT, 0x24, 0x00, 0xFF}, // Fire 1  
  { 0              , 0x25, 0x00, 0xFF}, // Load 2
  { KEY_RIGHT_SHIFT, 0x25, 0x00, 0xFF}, // Fire 2
  { 0              , 0x26, 0x00, 0xFF}, // Load 3
  { KEY_RIGHT_SHIFT, 0x26, 0x00, 0xFF}, // Fire 3
  { 0              , 0x27, 0x00, 0xFF}, // Load 4
  { KEY_RIGHT_SHIFT, 0x27, 0x00, 0xFF} // Fire 4
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

  if(!digitalRead(irqpin1)) {
    processInputs(0x5A);
  }
  if (!digitalRead(irqpin2))  {
    processInputs(0x5B);
  }
}

void processInputs(uint8_t address) {
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
  
  for (int i = 0; i<24; i++) {
    int addr = 0x5A;
    int elec = i;
    if (i > 11) { 
        addr = 0x5B;
        elec = i - 12;
    }
    setTouchValues(addr, elec, keys[i][2], keys[i][3]);
  }
  

  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(0x5A, FIL_CFG, 0x04);
  set_register(0x5B, FIL_CFG, 0x04);
  
  // Section E
  // Electrode Configuration
  // Set ELE_CFG to 0x00 to return to standby mode
  set_register(0x5A, ELE_CFG, 0x0C);  // Enables 12 inputs
  set_register(0x5B, ELE_CFG, 0x0C);  // Enables 12 inputs
  
  set_register(0x5A, ATO_CFG0, 0x0B);
  set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   
  set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5A, ATO_CFGT, 0xB5);  // Target = 0.9*USL = 0xB5 @3.3V


  set_register(0x5B, ATO_CFG0, 0x0B);
  set_register(0x5B, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   
  set_register(0x5B, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5B, ATO_CFGT, 0xB5);  // Target = 0.9*USL = 0xB5 @3.3V
  
  set_register(0x5A, ELE_CFG, 0x0C);  // Enables 12 inputs
  set_register(0x5B, ELE_CFG, 0x0C);  // Enables 12 inputs
}

void setTouchValues(uint8_t address, uint8_t electrode, uint8_t touch_thresh, uint8_t release_thresh) {
  if (touch_thresh == 0x00) { touch_thresh = TOU_THRESH; }
  if (release_thresh == 0xFF) { release_thresh = REL_THRESH; }
  set_register(address, elec_registers[electrode], touch_thresh);
  set_register(address, elec_registers[electrode]+0x01, release_thresh); 
}

void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}
