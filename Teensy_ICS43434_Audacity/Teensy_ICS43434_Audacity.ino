
#include <Audio.h>
#include <i2c_driver_wire.h>  // Use the Teensy4_I2C library instead of Wire
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=121,318
AudioInputUSB            usb1;           //xy=122,247
AudioMixer4              mixer2;         //xy=271,331
AudioMixer4              mixer1;         //xy=274,262
AudioOutputI2S           i2s1;           //xy=463,295
AudioOutputUSB           USBOut;
AudioConnection          patchCord1(i2s2, 0, mixer1, 1);
AudioConnection          patchCord2(i2s2, 1, mixer2, 1);
AudioConnection          patchCord3(usb1, 0, mixer1, 0);
AudioConnection          patchCord4(usb1, 1, mixer2, 0);
AudioConnection          patchCord5(mixer2, 0, i2s1, 1);
AudioConnection          patchCordUSBoutL ( mixer1, 0, USBOut , 0);
AudioConnection          patchCordUSBoutR (mixer2, 0, USBOut, 1);
AudioConnection          patchCord6(mixer1, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=273,457
// GUItool: end automatically generated code


void setup() {                
  AudioMemory(12);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(40);
  mixer1.gain(0, .8);
  mixer1.gain(1, .8);
  mixer2.gain(0, .8);
  mixer2.gain(1, .8);
}

void loop() {
}