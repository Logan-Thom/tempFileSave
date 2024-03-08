#ifndef TelemetrySender_h
#define TelemetrySender_h

#include "Arduino.h"
#include "TelemetryData.h" // Data structure 

class TelemetrySender {
public:
    TelemetrySender();
    void begin(long baudRate = 115200);
    void sendTelemetry(const TelemetryData& data);
private:
    void configureLoRaModule();
};

#endif
