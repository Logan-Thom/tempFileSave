#include "TelemetrySender.h"

TelemetrySender::TelemetrySender() {
    // Constructor doesn't do anything
}

void TelemetrySender::begin(long baudRate) {
    //Serial1.begin(baudRate);    will be done in main code to prevent errors in serial start-up time
    configureLoRaModule();
}

void TelemetrySender::sendTelemetry(const TelemetryData& data) {
    // Convert telemetry data to string
    String telemetryString = String(data.pressure) + "," +
                             String(data.altitude) + "," +
                             String(data.maxAltitude) + "," +
                             String(data.accelerationX) + "," +
                             String(data.accelerationY) + "," +
                             String(data.accelerationZ) + "," +
                             String(data.rotationX) + "," +
                             String(data.rotationY) + "," +
                             String(data.rotationZ) + "," +
                             String(data.rocketState); 
    
    // Calculate the length of the telemetry string
    int dataLength = telemetryString.length();

    // Prepare the AT command with appropriate address, length and data
    String atCommand = "AT+SEND=3," + String(dataLength) + "," + telemetryString + "\r\n";

    // Send the AT command via LoRa
    Serial3.println(atCommand);

    // Debug output to the Serial monitor, not used in FC
    //Serial.println("Telemetry sent: " + telemetryString);
}


void TelemetrySender::configureLoRaModule() {
    // AT commands to configure the LoRa module
	// Set ADDRESS
    Serial3.println("AT+ADDRESS=3\r\n");  // Set address same on both modules
    

    // Set NETWORKID
    Serial3.println("AT+NETWORKID=5\r\n");  // Set network ID same on both modules

    // Set BAND
    Serial3.println("AT+BAND=433E6\r\n");  // 433 MHz check????

    // Set PARAMETER
    Serial3.println("AT+PARAMETER=10,7,1,7\r\n");  // Parameter settings for over 3km
    //Serial.print("LoRa module configured");
}
