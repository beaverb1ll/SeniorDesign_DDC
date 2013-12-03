#define CUP_PRESENT_THRESHOLD 600   // this holds the voltage reading that a cup is considered present
#define SERIAL_READ_TIMEOUT 10000     // (ms)
#define CUP_PRESENT_TIMEOUT 10000     // (ms)
#define NUM_INGREDIENTS 6
#define CUP_SENSOR_PIN A0

void setup(void) {
   Serial.begin(9600);
   Serial.println("Board Reset");
   pinMode(CUP_SENSOR_PIN, INPUT);
 
 }

void loop(void) {
  
  Serial.println(analogRead(CUP_SENSOR_PIN));
}
