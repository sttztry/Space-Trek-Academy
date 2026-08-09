#include <Wire.h>
#include <Adafruit_MPL3115A2.h>

Adafruit_MPL3115A2 mpl = Adafruit_MPL3115A2();

// --- CALIBRATION ---
const float LAUNCH_ELEVATION_METERS = 3.0; // Kennedy Space Center elevation (~3m)
float altitudeOffset = 0.0;

// --- MOVING AVERAGE FILTER ---
const int FILTER_SAMPLES = 10;
float altitudeBuffer[FILTER_SAMPLES];
int bufferIndex = 0;
float bufferSum = 0.0;

void setup() {
  Serial.begin(115200);

  // Startup delay for battery power bootup
  delay(2000); 

  if (!mpl.begin()) {
    Serial.println("ERROR: MPL3115A2 not detected!");
  } else {
    Serial.println("MPL3115A2 Initialized Successfully.");
  }

  delay(1000);

  // Ground zero offset calibration
  float rawInitialAltitude = mpl.getAltitude();
  altitudeOffset = LAUNCH_ELEVATION_METERS - rawInitialAltitude;

  for (int i = 0; i < FILTER_SAMPLES; i++) {
    altitudeBuffer[i] = LAUNCH_ELEVATION_METERS;
    bufferSum += LAUNCH_ELEVATION_METERS;
  }

  // Updated CSV Header: Time in seconds
  Serial.println("Time_sec,Pressure_Pa,Temp_C,Calibrated_Alt_m");
}

void loop() {
  // Convert milliseconds to seconds (dividing by 1000.0 preserves decimal precision)
  float timeInSeconds = millis() / 1000.0;

  float rawPressure = mpl.getPressure();    // Raw Pressure in Pascals (Pa)
  float rawTemp = mpl.getTemperature();     // Temperature in Celsius (°C)
  float rawAlt = mpl.getAltitude() + altitudeOffset;

  // Rolling average smoothing
  bufferSum -= altitudeBuffer[bufferIndex];
  altitudeBuffer[bufferIndex] = rawAlt;
  bufferSum += altitudeBuffer[bufferIndex];
  bufferIndex = (bufferIndex + 1) % FILTER_SAMPLES;
  float smoothedAltitude = bufferSum / FILTER_SAMPLES;

  // Output formatted CSV data
  Serial.print(timeInSeconds, 2); // Prints seconds with 2 decimal places (e.g., 12.50)
  Serial.print(",");
  Serial.print(rawPressure, 2);
  Serial.print(",");
  Serial.print(rawTemp, 2);
  Serial.print(",");
  Serial.println(smoothedAltitude, 2);

  delay(500); // 2 Hz sample rate (0.5 second updates)
}