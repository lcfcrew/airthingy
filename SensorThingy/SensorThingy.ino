/*
 * SnesorThingy -- Sensors/i2c streaming platoform
 * 
 * By edg and the LCF Crew
 * 
 * This sketch implements a sensor platform, that streams various sensor data
 * through I2C to a network device, which can send it to the cloud
 * 
 * This is the "slave" i2c device.  For the master, see the "AirThingy"
 * in this repo.
 */

//############## CONFIGURATION -- EDIT ME ##################

// Tuning params for Sharp Dust Sensor
#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000 //The red wire, 5000 = the 5V pin

// IO Params for the Sharp Dust Sensor
const int iled = 7; // The yellow wire
const int vout = 0; // The... what color is that? wire (AOUT on the board)

// The i2c address of this board
#define MY_I2C_ADDR 69

// Geo info
double lat = 34.4140067; // Seclab
double lon = -119.8427564;

// ################ END CONFIGURATION ###############

#include <ArduinoJson.h> // Oh yeah, we're doing that.  Get ready...
#include <Wire.h> // Man, I feel really pro using I2C for a project now...

//---------- This part borrowed from WaveShare's implementation
// For the dust sensor:
/*
variable
*/
float density, voltage;
int   adcvalue;

/*
private function
*/
int Dust_Sensor_Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

void dust_sensor_init() {
  pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);
}

float dust_sensor_read() {
  /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Dust_Sensor_Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;

  return density;
}
// ---------- End Dust Sensor Stuff

// Main Init Function
void setup() {
  Serial.begin(9600);
  Serial.println("### SensorThingy v0 BOOT:");
  dust_sensor_init();
  
  // Bring up I2C. Do this last
  Wire.begin(MY_I2C_ADDR);
  Serial.println("I2C Address: " + String(MY_I2C_ADDR));
  Wire.onRequest(requestEvent); // register event
  Serial.println("Waiting for Master...");
}

void loop() {
  printSensors();
  delay(10000);
}

void get_sensor_json(char* buf) {
  StaticJsonBuffer<200> jsonBuffer; // Woah, these dudes do malloc-less json?? Hax
  JsonObject& data = jsonBuffer.createObject();
  data["lat"] = lat;
  data["lon"] = lon;
  data["dust"] = dust_sensor_read();
  data["air_quality"] = 0.6969; // TODO: FAKE
  data.prettyPrintTo(buf, 200);
}

void printSensors() {
  char toSend[200];
  memset(toSend,0,sizeof(toSend));
  get_sensor_json(toSend);
  Serial.println(toSend);
}

// Callback for I2C -- Reads sensors and returns a JSON
void requestEvent() {
  Serial.println("Got request from Master");
  char toSend[200];
  memset(toSend,0,sizeof(toSend));
  get_sensor_json(toSend);
  Wire.write(toSend,sizeof(toSend));
  
}
