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

// ### Dust Sensor
// Tuning params for Sharp Dust Sensor
#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000 //The red wire, 5000 = the 5V pin

// IO Params for the Sharp Dust Sensor
const int iled = 7; // The yellow wire
const int vout = 0; // The... what color is that? wire (AOUT on the board)

// ### MQ-135 "air quality" sensor

int mq135_pin = 1; // The analog output of MQ-135 goes here
// NOTE: You must set the calibration value RZERO in MQ135.h!

// ### Comms and misc settings
// The i2c address of this board
#define MY_I2C_ADDR 69

#define _7SEG (0x38)   /* I2C address for 7-Segment */
#define THERM (0x49)   /* I2C address for digital thermometer */
#define EEP (0x50)     /* I2C address for EEPROM */
#define RED (3)        /* Red color pin of RGB LED */
#define GREEN (5)      /* Green color pin of RGB LED */
#define BLUE (6)       /* Blue color pin of RGB LED */


// Geo info
double lat = 34.4140067; // Seclab
double lon = -119.8427564;

//Score Cutoffs

#define SCORE_GOOD 69
#define SCORE_BAD 420

// ################ END CONFIGURATION ###############

#include <Wire.h> // Man, I feel really pro using I2C for a project now...
#include <MQ135.h> // https://github.com/GeorgK/MQ135

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

void dust_sensor_read() {
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
}
// ---------- End Dust Sensor Stuff

// ---------- Begin 7SEG Stuff (From Gravitech) ---------

// The number lookup table
const byte NumberLookup[16] =   {0x3F,0x06,0x5B,0x4F,0x66,
                                 0x6D,0x7D,0x07,0x7F,0x6F, 
                                 0x77,0x7C,0x39,0x5E,0x79,0x71};


/***************************************************************************
 Function Name: Cal_temp

 Purpose: 
   Calculate temperature from raw data.
****************************************************************************/
void Cal_temp (int& Decimal, byte& High, byte& Low, bool& sign)
{
  if ((High&B10000000)==0x80)    /* Check for negative temperature. */
    sign = 0;
  else
    sign = 1;
    
  High = High & B01111111;      /* Remove sign bit */
  Low = Low & B11110000;        /* Remove last 4 bits */
  Low = Low >> 4; 
  Decimal = Low;
  Decimal = Decimal * 625;      /* Each bit = 0.0625 degree C */
  
  if (sign == 0)                /* if temperature is negative */
  {
    High = High ^ B01111111;    /* Complement all of the bits, except the MSB */
    Decimal = Decimal ^ 0xFF;   /* Complement all of the bits */
  }  
}

void seg7_init() {
  /* Configure 7-Segment to 12mA segment output current, Dynamic mode, 
     and Digits 1, 2, 3 AND 4 are NOT blanked */
     
  Wire.beginTransmission(_7SEG);    
  Wire.write(0);
  Wire.write(B01000111);
  Wire.endTransmission();
  
  /* Setup configuration register 12-bit */
     
  Wire.beginTransmission(THERM);    
  Wire.write(1);
  Wire.write(B01100000);
  Wire.endTransmission();
  
  /* Setup Digital THERMometer pointer register to 0 */
     
  Wire.beginTransmission(THERM);    
  Wire.write(0);
  Wire.endTransmission();
  
  /* Test 7-Segment */
  for (int counter=0; counter<8; counter++)
  {
    Wire.beginTransmission(_7SEG);
    Wire.write(1);
    for (int counter2=0; counter2<4; counter2++)
    {
      Wire.write(1<<counter);
    }
    Wire.endTransmission();
    delay (250);
  }
  
}

/***************************************************************************
 Function Name: Dis_7SEG

 Purpose: 
   Display number on the 7-segment display.
****************************************************************************/
void Dis_7SEG (int Decimal, byte High, byte Low, bool sign)
{
  byte Digit = 4;                 /* Number of 7-Segment digit */
  byte Number;                    /* Temporary variable hold the number to display */
  
  if (sign == 0)                  /* When the temperature is negative */
  {
    Send7SEG(Digit,0x40);         /* Display "-" sign */
    Digit--;                      /* Decrement number of digit */
  }
  
  if (High > 99)                  /* When the temperature is three digits long */
  {
    Number = High / 100;          /* Get the hundredth digit */
    Send7SEG (Digit,NumberLookup[Number]);     /* Display on the 7-Segment */
    High = High % 100;            /* Remove the hundredth digit from the TempHi */
    Digit--;                      /* Subtract 1 digit */    
  }
  
  if (High > 9)
  {
    Number = High / 10;           /* Get the tenth digit */
    Send7SEG (Digit,NumberLookup[Number]);     /* Display on the 7-Segment */
    High = High % 10;            /* Remove the tenth digit from the TempHi */
    Digit--;                      /* Subtract 1 digit */
  }
  
  Number = High;                  /* Display the last digit */
  Number = NumberLookup [Number]; 
  if (Digit > 1)                  /* Display "." if it is not the last digit on 7-SEG */
  {
    Number = Number | B10000000;
  }
  Send7SEG (Digit,Number);  
  Digit--;                        /* Subtract 1 digit */
  
  if (Digit > 0)                  /* Display decimal point if there is more space on 7-SEG */
  {
    Number = Decimal / 1000;
    Send7SEG (Digit,NumberLookup[Number]);
    Digit--;
  }

  if (Digit > 0)                 /* Display "c" if there is more space on 7-SEG */
  {
    Send7SEG (Digit,0x58);
    Digit--;
  }
  
  if (Digit > 0)                 /* Clear the rest of the digit */
  {
    Send7SEG (Digit,0x00);    
  }  
}

/***************************************************************************
 Function Name: Send7SEG

 Purpose: 
   Send I2C commands to drive 7-segment display.
****************************************************************************/

void Send7SEG (byte Digit, byte Number)
{
  Wire.beginTransmission(_7SEG);
  Wire.write(Digit);
  Wire.write(Number);
  Wire.endTransmission();
}

/***************************************************************************
 Function Name: UpdateRGB

 Purpose: 
   Update RGB LED according to define HOT and COLD temperature. 
****************************************************************************/

void UpdateRGB (byte score)
{
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);        /* Turn off all LEDs. */
  
  if (score <= SCORE_GOOD)
  {
    digitalWrite(BLUE, HIGH);
  }
  else if (score >= SCORE_BAD)
  {
    digitalWrite(RED, HIGH);
  }
  else 
  {
    digitalWrite(GREEN, HIGH);
  }
}


// ---------- MQ-135 stuff

MQ135 gasSensor = MQ135(mq135_pin);


// -------------- Main Init Function
void setup() {
  Serial.begin(9600);
  Serial.println("### SensorThingy v0 BOOT:");

  Serial.println("Dust sensor init...");
  dust_sensor_init();

  Serial.println("Gas sensor init...");
  gasSensor = MQ135(mq135_pin);

  // Bring up I2C. Do this last
  Serial.println("I2C init... Address " + String(MY_I2C_ADDR));
  Wire.begin();
  seg7_init();
  Serial.println("Waiting for Master...");
}

void loop() {
  dust_sensor_read();
  printSensors();
  delay(10000);
}

void get_sensor_json(char* buf) {
}

void printSensors() {
  Serial.println(density);
}


