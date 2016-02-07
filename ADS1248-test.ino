#include "ads12xx.h"
#include <SPI.h>
#include <math.h>

const uint16_t MAX_SAMPLE_SIZE = 100;
int32_t data[MAX_SAMPLE_SIZE];

uint8_t PULSE_LEN;
uint8_t SAMPLE_SIZE;

byte DRDY = 3;
byte CS = 4;
byte PULSE = 5;
byte RELAY = 6;

// Define which ADC to use in the ads12xx.h file
ads12xx ADS;

void setup() {
  Serial.begin(230400);
  while (!Serial) {
  }
  Serial.println("Serial online");
  ADS.begin(CS, RELAY, DRDY); // initialize ADS as object of the ads12xx class

  ADS.Reset();

  delay(10);

  // init default pins for MUX and DRATE
  sendDefaults();

  Serial.println("Commands for testing:");
  Serial.println("'r' to read Register");
  Serial.println("'w' to write Register");
  Serial.println("'R' to get a Conversion Result");
  Serial.println("'x' to SDATAC, 'd' for SDATA");
  Serial.println("'o' to write Pre Predefinde Registers");
  Serial.println("'f' to write a command");
  Serial.println("'c' to select calibration methode");
  Serial.println("'s' to set sample size and pulse length");
  Serial.println("'t' transmitted pulse length in us");
  Serial.println("'h' for this help");
}

void sendDefaults() {
  ADS.SendCMD(SDATAC);
  ADS.SetRegisterValue(MUX, P_AIN0 | N_AINCOM);
  ADS.SetRegisterValue(DRATE, DR_30000);
  // ADS.SendCMD(SYNC);
  Serial.println("Writing sucessful");
  SAMPLE_SIZE = 100;
  PULSE_LEN = 3;
}

void readc() {
  char check = 'y';
  while (check == 'y') {
    if (Serial.available()) {
      check = Serial.read();
    }
    uint32_t data = ADS.GetConversion();
    int timer1 = micros();
    if (long minus = data >> 23 == 1) {
      long data = data - 16777216;
    }
    Serial.println(data);

    // double voltage = (4.9986 / 8388608)*data;
    // Serial.println(voltage);
  }
}

void loop() {
  // transmit();
  if (Serial.available()) {
    char cin = Serial.read();

    uint8_t cmd;
    uint8_t cin1;
    switch (cin) {
    case 'r':
      Serial.println("Which Register to read?");
      while (!Serial.available())
        ;
      cin1 = Serial.parseInt();
      Serial.println(cin1);
      Serial.println(ADS.GetRegisterValue(cin1), BIN);
      break;
    case 'w':
      Serial.println("Which Register to write?");
      while (!Serial.available())
        ;
      cin1 = Serial.parseInt();
      Serial.println("Which Value to write?");
      while (!Serial.available())
        ;
      ADS.SetRegisterValue(cin1, Serial.parseInt());
      break;
    case 'R':
      Serial.println("Conversion Result");
      Serial.println(ADS.GetConversion());
      break;
    case 'x':
      Serial.println("Stop SDATAC");
      ADS.Reset();
      break;
    case 'o':
      Serial.println("Writing predefind Registers");
      sendDefaults();
      break;
    case 'd':
      readc();
      break;
    case 'f':
      Serial.println("Which command to write");
      while (!Serial.available())
        ;
      cmd = Serial.parseInt();
      Serial.print(cmd, HEX);
      ADS.SendCMD(cmd);
      break;
    case 'c':
      calibrate();
      break;
    case 't':
      transmit();
      break;
    case 's':
      saveConfig();
    case 'h':
      Serial.println("Commands for testing:");
      Serial.println("'r' to read Register");
      Serial.println("'w' to write Register");
      Serial.println("'R' to get a Conversion Result");
      Serial.println("'x' to SDATAC, 'd' for SDATA");
      Serial.println("'o' to write Pre Predefinde Registers");
      Serial.println("'f' to write a command");
      Serial.println("'c' to select calibration method");
      Serial.println("'s' to set sample size and pulse length");
      Serial.println("'t' transmitted pulse length in us");
      Serial.println("'h' for this help");
      break;
    default:
      break;
    }
  }
}

void saveConfig() {
  Serial.println("Pulse length (in us): ");
  while (!Serial.available()) {
  }
  PULSE_LEN = Serial.parseInt();

  Serial.println("Sample size (max 255): ");
  while (!Serial.available()) {
  }
  SAMPLE_SIZE = Serial.parseInt();

  if (SAMPLE_SIZE > MAX_SAMPLE_SIZE) {
    Serial.println("Error: Sample size is too large");
    SAMPLE_SIZE = MAX_SAMPLE_SIZE;
    return;
  }
}

void transmit() {
  // ADS.SendCMD(RDATAC);

  char check = 'y';
  while (check == 'y') {
    if (Serial.available()) {
      check = Serial.read();
    }
    // clear all previous data
    memset(data, 0, SAMPLE_SIZE * sizeof(data[0]));

    digitalWrite(PULSE, LOW);
    delayMicroseconds((uint32_t)PULSE_LEN * 100);
    digitalWrite(PULSE, HIGH);

    ADS.GetContConversion(data, SAMPLE_SIZE);

    for (uint8_t i = 0; i < SAMPLE_SIZE; ++i) {
      Serial.println(data[i]);
    }
  }

  // stop cont. data ! otherwsie cannot read / write to registers.
  // ADS.SendCMD(SDATAC);
}

void calibrate() {
  uint8_t cmd;
  Serial.println("Which Calibration to run?");
  Serial.println("'1' for SELFCAL");
  Serial.println("'2' for SYSOGCAL\n'3' for SYSGCAL");
  while (!Serial.available())
    ;
  cmd = Serial.parseInt();
  switch (cmd) {
  case 1:
    Serial.println("Preforming Self Gain and Offset Callibration");
    ADS.SendCMD(SELFCAL);
    delay(5);
    Serial.print("OFC0: ");
    Serial.println(ADS.GetRegisterValue(OFC0));
    Serial.print("OFC1: ");
    Serial.println(ADS.GetRegisterValue(OFC1));
    Serial.print("OFC2: ");
    Serial.println(ADS.GetRegisterValue(OFC2));
    Serial.print("FSC0: ");
    Serial.println(ADS.GetRegisterValue(FSC0));
    Serial.print("FSC1: ");
    Serial.println(ADS.GetRegisterValue(FSC1));
    Serial.print("FSC2: ");
    Serial.println(ADS.GetRegisterValue(FSC2));
    break;
  case 2:
    Serial.println("Preforming System Offset Callibration");
    ADS.SendCMD(SYSOCAL);
    delay(5);
    Serial.print("OFC0: ");
    Serial.println(ADS.GetRegisterValue(OFC0));
    Serial.print("OFC1: ");
    Serial.println(ADS.GetRegisterValue(OFC1));
    Serial.print("OFC2: ");
    Serial.println(ADS.GetRegisterValue(OFC2));
    break;
  case 3:
    Serial.println("Preforming System Gain Callibration");
    ADS.SendCMD(SYSGCAL);
    delay(5);
    Serial.print("FSC0: ");
    Serial.println(ADS.GetRegisterValue(FSC0));
    Serial.print("FSC1: ");
    Serial.println(ADS.GetRegisterValue(FSC1));
    Serial.print("FSC2: ");
    Serial.println(ADS.GetRegisterValue(FSC2));
    break;
  default:
    break;
  }
}
