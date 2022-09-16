/*
 * ESPROS EPC901 Line Imager Control Firmware
 * An Arduino MEGA 2560 is used due to the limited dynamic memory
 * (~2KB) of a normal Arduino UNO (ATMega328P)
 * Carlos Ramirez - 9/13/22
 */

#include <Wire.h>
#include "epc901_registers.h"
#include "pins.h"

#define EPC901_ADDR 0b0010101

// States
uint8_t state = 0;
uint8_t return_state = 0;
#define idle 0
#define SINGLE 1
#define CONTINUOUS 2
#define EPC_CONFIG 3
#define GAIN 4

// EPC901 Configuration Commands
uint8_t config_command = 0;
#define CHIP_REV_NO 0
#define EPC_RESET 1

volatile bool data_ready = false;
uint32_t prevMillis = 0;

uint16_t data[1027];
// Index 0 is left temperature, 1 is right temperature.
uint16_t epc901_temperature[2];
uint8_t register_data;

void setup() 
{
  pinMode(DATA_RDY, INPUT);
  pinMode(CLR_PIX, OUTPUT);
  pinMode(SHUTTER, OUTPUT);
  pinMode(VIDEO_P, INPUT);
  pinMode(READ, OUTPUT);

  digitalWrite(CLR_PIX, LOW);
  digitalWrite(SHUTTER, LOW);
  digitalWrite(READ, LOW);

  attachInterrupt(digitalPinToInterrupt(DATA_RDY), data_ready_ISR, RISING);

  analogReference(EXTERNAL);

  Wire.begin();

  Serial.begin(250000);
  Serial.println(F("BEGIN"));
}

void loop() 
{
  switch (state)
  {
    case SINGLE:
      epc901_read();
      state = idle;
    break;
    
    case CONTINUOUS:
      epc901_read();
    break;

    case EPC_CONFIG:
      switch (config_command)
      {
        case CHIP_REV_NO:
          register_data = read_register(EPC901_ADDR, CHIP_REV_NO_REG);
          print_register(register_data);
        break;

        case GAIN:
          bool success = false;
          
          success = write_register(EPC901_ADDR, ACQ_TX_CONF_I2C, RD_DIR_MASK, RD_DIR_RL);

          if (success)
            Serial.println("SUCCESS");
          else
            Serial.println("FAIL");
        break;

        case EPC_RESET:

        break;

        default:
          Serial.println(F("INVALID COMMAND"));
        break;
      }
      state = return_state;
    break;
    
    case idle:
    default:
      // If there's nothing to do, just print "WAIT" as a heartbeat signal
      if ((millis() - prevMillis) >= 1500) // Interval between prints in milliseconds
      {
        Serial.println(F("WAIT"));
        prevMillis = millis();
      }
    break;
  }
}

void serialEvent(void)
{
  String command = Serial.readStringUntil('\n'); // Read until newline char. '\n' is truncated.

  switch (command.charAt(0))
  {
    case '$':
      switch (command.charAt(1))
      {
        case '0':
          Serial.println(F("STOP READ"));
          state = idle;
        break;
        
        case '1':
          state = SINGLE;
        break;

        case '2':
          state = CONTINUOUS;
        break;
    
        default:
        break;
      }
    break;
    default:
      if (command == "CHIP_REV_NO")
      {
        return_state = state;
        config_command = CHIP_REV_NO;
        state = EPC_CONFIG;
      }
      else if (command == "EPC_RESET")
      {
        return_state = state;
        config_command = EPC_RESET;
        state = EPC_CONFIG;
      }
      else if (command == "GAIN")
      {
        return_state = state;
        config_command = GAIN;
        state = EPC_CONFIG;
      }
      else
        Serial.println(F("INVALID COMMAND"));
    break;
  }
}

void data_ready_ISR(void)
{
  data_ready = true;
}

void epc901_read(void)
{
    clear_pix();
    capture_image();
    
    while(!data_ready){};
    data_ready = false;
    
    digitalWrite(READ, HIGH);
    delayMicroseconds(2);
    digitalWrite(READ, LOW);
    delayMicroseconds(100);
    for (uint16_t i = 0; i <= 1026; i++)
    {
      digitalWrite(READ, HIGH);
      data[i] = analogRead(VIDEO_P);
      digitalWrite(READ, LOW);
    }

    Serial.print("D ");
    
    for (uint16_t i = 3; i <= 1026; i++)
        Serial.print(data[i] + String(","));
    
    Serial.println('\n');

    delayMicroseconds(15000);
}


//void send_data(uint16_t data)
//{
//  Serial.print("D ");
//    
//  for (uint16_t i = 3; i <= 1026; i++)
//    Serial.print(data[i] + String(","));
//  
//  Serial.println('\n');
//}

//void send_data(uint16_t data)
//{
//  Serial.write('D');
//
//  for (uint16_t i = 3; i <= 1026; i++)
//    Serial.write(data[i]);
//
//  Serial.write('\n');
//}

bool write_register(uint8_t devAddr, uint8_t registerAddr, uint8_t mask, uint8_t settings)
{
  uint8_t register_val = 0;

  /*
   * epc901 manual recommends reading contents of register first,
   * modify the required bits, then write back the modified 
   * register value.
   */

  /*
   * Read and Modify
   */
  register_val = read_register(devAddr, registerAddr);
  register_val = set_register(register_val, mask, settings);  
  print_register(register_val);

  /*
   * Write back modified register
   */
  Wire.beginTransmission(devAddr);
  Wire.write(byte(register_val));
  // true = Generate full stop condition
  Wire.endTransmission(true);


  /* 
   * Check epc901 I2C error flag to make sure the register 
   * operation was properly serviced.
   */
  bool error_flag = read_register(EPC901_ADDR, I2C_ERROR_IND);
  if (error_flag)
    return false; // Register operation failed
  else
    return true;  // Register operation success
}

uint8_t set_register(uint8_t register_contents, uint8_t mask, uint8_t new_settings)
{
  /*
   * Combine new settings with existing register contents.
   */
  new_settings &= mask;
  new_settings |= (register_contents & ~mask);

  return new_settings;
}

uint8_t read_register(uint8_t devAddr, uint8_t registerAddr)
{
  uint8_t data = 0;
  uint8_t len = 1;
  
  Wire.beginTransmission(devAddr);
  Wire.write(byte(registerAddr));
  // false = Generate repeated start condition
  Wire.endTransmission(false);
  Wire.requestFrom(devAddr, len);

  if (Wire.available())
    data = Wire.read();
  
  return data;
}

void print_register(uint8_t reg)
{
  Serial.print("R ");
  Serial.println(reg, BIN);
}

void clear_pix(void)
{
  digitalWrite(CLR_PIX, HIGH);
  delayMicroseconds(1);
  digitalWrite(CLR_PIX, LOW);
}

void capture_image(void)
{
  digitalWrite(SHUTTER, HIGH);
  delayMicroseconds(100);
  digitalWrite(SHUTTER, LOW);
}
