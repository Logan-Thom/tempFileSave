/*
  * Main FLight Computer for Strath AIS Spaceport America Cup 2024
  * Current Functionality: Working Barometer, Accelerometer, flight control logic
  * Needed Functionality: Radio, SD Card, wiring to ignition circcuits
  * Created For: Strath AIS
  * Created By: Logan Thom
  * Created On: 16/12/23
  * Last Updated: 12/02/24
  * Updated By: Logan Thom
*/

#include <Barometer.h>
#include <Accelerometer.h>
#include <TelemetryData.h>
#include <TelemetrySender.h>
#include <Wire.h>
#include <DataLoggerSD.h>


//declare timing variables/constants
unsigned long startMillis;
unsigned long currentMillis;
unsigned long time_main_deployed;
unsigned long time_drogue_deployed;

Barometer bmp;
Accelerometer mpu;
TelemetrySender telemetrySender;
TelemetryData data;

const int chipSelect = 32;
DataLoggerSD dlsd;

//start a flight status check for safety reasons and logic control
enum status { WeeBoyIdle,
              PoweredFlight,
              Coasting,
              Drogue,
              Main,
              Landed };
status flight_status;

//initialise flight data variables
float acc;
float alt;
float pressure;
float rotational_velocity;
float maxAlt;




//important output pins for parachutes,
const int drogue_deploy_pin = 50;
const int main_deploy_pin = 52;

String statusToString(status flightStatus) {
  switch (flightStatus) {
    case WeeBoyIdle: return "WeeBoyIdle";
    case PoweredFlight: return "PoweredFlight";
    case Coasting: return "Coasting";
    case Drogue: return "Drogue";
    case Main: return "Main";
    case Landed: return "Landed";
    default: return "Unknown";
  }
}



void setup() {

  Serial.begin(115200);
  Serial3.begin(115200);


  Serial.println("Program Start");
  //dlsd.speak();
  
  
  //begin program timing
  startMillis = millis();

  //ignition circuits setup
  pinMode(drogue_deploy_pin, OUTPUT);
  pinMode(main_deploy_pin, OUTPUT);

  //=======================================================
  //Barometer Setup, looks awful but sadly its necessary
  //for it to actually work properly, dont worry too much
  //about how it works exactly.
  //Starts I2C on the chip, sets sample rates
  //=======================================================

  while (!Serial);

  if(!bmp.begin_I2C()) {  // hardware I2C mode, can pass in address & alt Wire
                           //if (! bmp.begin_SPI(BMP_CS)) {  // hardware SPI mode
                           //if (! bmp.begin_SPI(BMP_CS, BMP_SCK, BMP_MISO, BMP_MOSI)) {  // software SPI mode
    Serial.println("Could not find a valid BMP3 sensor, check wiring");
    while(1);
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_2X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_32X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_DISABLE);  //recommended setting for drop detection, makes testing slightly unreliable as is not suited to static environments with people moving aronud
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);


  bmp.Calibrate();

  //=======================================================
  //Accelerometer setup, mainly implementing ranges of
  //recorded data and constraints
  //=======================================================
  if (!mpu.begin()) {
    Serial.println("Could not find a valid mpu sensor, check wiring");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU_6050 located");
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);

  
  //=======================================================
  //SD card setup
  //=======================================================

  dlsd.Initialise();

  dlsd.removeOld("offsets.txt");
  dlsd.removeOld("datalog.txt");

  float offsets[] = {0.0,0.0,0.0,0.0,0.0,bmp.zeroHeight}; //currently only using one offset
  dlsd.newOffsetlog("offsets.txt",7,"XAOffset,YAOffset,ZAOffset,XSOffset,YSOffset,ZSOffset,zeroHeight",offsets);
  dlsd.newDatalog("datalog.txt","acc,spin,height,pressure");


  //=======================================================
  //Radio and telemetry setup
  //=======================================================


  //set flight status
  flight_status = WeeBoyIdle;  //thanks, Jack.

  Serial.println("Setup complete");
}

void loop() {


  //update timer variable
  currentMillis = millis();


  //update, store and transmit data
  mpu.UpdateSensorData();
  acc = mpu.GetAccelerationMagnitude();
  pressure = bmp.GetCurrentPressure();
  alt = bmp.GetCurrentHeight();
  maxAlt = bmp.highestAltitude - bmp.zeroHeight;
  rotational_velocity = mpu.GetRotationMagnitude();



  

  //perform flight logic
  switch (flight_status) {
    case 0:
      if (acc > 50.0) {
        //Big Boy Launch

        flight_status = PoweredFlight;
      }
      break;

    case 1:
      if (acc < 15) {
        flight_status = Coasting;
      }
      break;

    case 2:
      if (bmp.IsDescending() == true) {
        digitalWrite(drogue_deploy_pin, HIGH);
        //telemetrySender.sendTelemetry("AT+SEND=3,18,DROGUEDEPLOYED\r\n");
        time_drogue_deployed = millis();
        flight_status = Drogue;
      }
      break;

    case 3:
      if (alt < 450 && currentMillis - time_drogue_deployed > 3000) {
        digitalWrite(main_deploy_pin, HIGH);
        //telemetrySender.sendTelemetry("AT+SEND=3,16,MAINDEPLOYED\r\n");
        time_main_deployed = millis();
        flight_status = Main;
      }
      break;

    case 4:
      if (currentMillis - time_main_deployed > 240000) {
        flight_status = Landed;
      }
      break;

    case 5:
      //(hopefully) flight successful
      //do final things to save data
      Serial.println("Landed");
      digitalWrite(drogue_deploy_pin, LOW);  //just in case
      digitalWrite(main_deploy_pin, LOW);
      for (;;)
        ;
      break;
  }

  //data transfer and storage

  //telemetry data collection
  data.pressure = pressure;
  data.altitude = alt;
  data.maxAltitude = maxAlt;
  data.accelerationX = mpu.XAcc;
  data.accelerationY = mpu.YAcc;
  data.accelerationZ = mpu.ZAcc;
  data.rotationX = mpu.XSpin;
  data.rotationY = mpu.YSpin;
  data.rotationZ = mpu.ZSpin;
  data.rocketState = statusToString(flight_status);

  telemetrySender.sendTelemetry(data);

  Serial3.write("AT+SEND=3,4,TEST\r\n");
  Serial3.flush();

  //sd card data logging
  dlsd.newEntry(currentMillis);
  dlsd.appendData(acc);
  dlsd.appendData(data.accelerationX);
  dlsd.appendData(data.accelerationY);
  dlsd.appendData(data.accelerationZ);
  dlsd.appendData(rotational_velocity);
  dlsd.appendData(data.rotationX);
  dlsd.appendData(data.rotationY);
  dlsd.appendData(data.rotationZ);
  dlsd.appendData(alt);
  dlsd.appendData(pressure);
  dlsd.saveEntry("datalog.txt");

}
