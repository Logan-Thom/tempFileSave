#ifndef TelemetryData_h
#define TelemetryData_h

// TelemetryData struct definition
struct TelemetryData {
  float pressure;
  float altitude;
  float maxAltitude;
  float accelerationX;
  float accelerationY;
  float accelerationZ;
  float rotationX;
  float rotationY;
  float rotationZ;
  String rocketState;
};

#endif

