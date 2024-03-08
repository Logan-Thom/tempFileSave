#include "Arduino.h"
#include "Accelerometer.h"
#include <math.h>

Accelerometer::Accelerometer()
{
    //constructor doesn't do anything
}



void Accelerometer::UpdateSensorData()
{
    
    bool tempBool = Accelerometer::getEvent(&a, &g, &temp);
    Accelerometer::XAcc = a.acceleration.x;
    Accelerometer::YAcc = a.acceleration.y;
    Accelerometer::ZAcc = a.acceleration.z;
    Accelerometer::XSpin = g.gyro.x;
    Accelerometer::YSpin = g.gyro.y;
    Accelerometer::ZSpin = g.gyro.z;
}

void Accelerometer::Calibrate()
{
    float XATemp;
    float YATemp;
    float XSTemp;
    float YSTemp;
    float ZSTemp; 
    for (int i=0;i<10;i++)
    {
        Accelerometer::UpdateSensorData();
        XATemp += XAcc;
        YATemp += YAcc;
        XSTemp += XSpin;
        YSTemp += YSpin;
        ZSTemp += ZSpin;
    }
    XAOffset = XATemp / 10;
    YAOffset = YATemp / 10;
    XSOffset = XSTemp / 10;
    YSOffset = YSTemp / 10;
    ZAOffset = ZSTemp / 10;
}

float Accelerometer::GetAccelerationMagnitude()
{
    float tempNum = (XAcc - XAOffset)*(XAcc - XAOffset) + (YAcc - YAOffset)*(YAcc - YAOffset) + ZAcc*ZAcc;
    return sqrt(tempNum);
}

float Accelerometer::GetRotationMagnitude()
{
    float tempNum = (XSpin - XSOffset)*(XSpin - XSOffset) + (YSpin - YSOffset)*(YSpin - YSOffset) + (ZSpin - ZSOffset)*(ZSpin - ZSOffset);
    return sqrt(tempNum);
}


