#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h> //OLED libraries
#include <Adafruit_SSD1306.h>
#include "algorithm_by_RF.h"
#include "max30102.h"

//#define DEBUG // uncomment to print to serial monitor

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)

// Interrupt pin
const byte oxiInt = 3; // pin connected to MAX30102 INT

uint32_t aun_ir_buffer[BUFFER_SIZE]; //infrared LED sensor data
uint32_t aun_red_buffer[BUFFER_SIZE];  //red LED sensor data
float old_n_spo2;  // Previous SPO2 value
uint8_t uch_dummy,k;

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.display();
  delay(3000);
  
  Wire.begin();
  maxim_max30102_reset(); //resets the MAX30102
  delay(1000);
  maxim_max30102_read_reg(REG_INTR_STATUS_1,&uch_dummy);  //Reads/clears the interrupt status register
  maxim_max30102_init();  //initialize the MAX30102
  old_n_spo2=0.0;
}

void loop() {
  float n_spo2,ratio,correl;  //SPO2 value
  int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
  int32_t n_heart_rate; //heart rate value
  int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
  int32_t i;

  for(i=0;i<100;i++)
  {
//    while(digitalRead(oxiInt)==1);  //wait until the interrupt pin asserts
//    maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));  //read from MAX30102 FIFO
    maxim_max30102_read_fifo((aun_ir_buffer+i), (aun_red_buffer+i));  //read from MAX30102 FIFO
  }

  rf_heart_rate_and_oxygen_saturation(aun_ir_buffer, BUFFER_SIZE, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid, &ratio, &correl); 

#ifdef DEBUG
  Serial.println("--RF--");
  Serial.print("\t");
  Serial.print(n_spo2);
  Serial.print("\t");
  Serial.print(n_heart_rate, DEC);
  Serial.print("\t");
  Serial.println("------");
#endif

  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  if(ch_hr_valid && ch_spo2_valid) {
    oled.print(F("HR: ")); oled.println(n_heart_rate, DEC);
    oled.print(F("SPO2: ")); oled.println(n_spo2, DEC);
    oled.display();
    old_n_spo2=n_spo2;
  }
  else {
    oled.print(F("Measuring"));
    oled.display();
  }
}
