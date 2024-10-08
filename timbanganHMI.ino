#include <SPI.h>
#include <Ethernet.h>
#include "MgsModbus.h"

#include "HX711.h"
HX711 scale;

MgsModbus Mb;
int inByte = 0; // incoming serial byte

// Ethernet settings (depending on MAC and Local network)
byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0x94, 0xB5 };
IPAddress ip(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 112);
IPAddress subnet(255, 255, 255, 0);

int checkBerat = 0;
int timbang = 0;
float calibration_factor = 420.0; //Nilai awal perkiraan
int gram;
int lastGram;
int kirimData;

boolean data = false;
boolean zero0 = false;

void timbangan() {

  scale.set_scale(calibration_factor);
  checkBerat = Mb.MbData[7];
  Serial.print("Trigger tare : ");
  Serial.println(Mb.MbData[7]);

  if ((checkBerat == 1) && (zero0 == false))
  {
    zero();
    zero0 = true;
  }

  gram = scale.get_units();

  if (gram < 0)
  {
    gram = 0.00;
  }

  if (gram != lastGram) {

    kirimData = gram * 4;
    Serial.print("berat : ");
    Serial.println(gram * 4);
    lastGram = gram;
  }

  if ((checkBerat == 0) && (zero0 == true))
  {
    zero0 = false;
  }
}


void zero() {
  long zero_factor = scale.read_average();
  scale.set_scale();
  scale.tare();
}


void setup()
{
  // serial setup
  Serial.begin(9600);
  Serial.println("Serial interface started");

  // initialize the ethernet device
  Ethernet.begin(mac, ip, gateway, subnet);   // start etehrnet interface
  Serial.println("Ethernet interface started");

  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  //Loadcell
  scale.begin(6, 7);
  zero();

}
boolean trigInc = false;
boolean trigDec = false;

void calFactor() {
  int inc = Mb.MbData[8];
  int dec = Mb.MbData[9];
  
  
  if (inc == 1 && trigInc == false) {
    calibration_factor++;
    trigInc = true;
  }
  else{
    trigInc = false;
  }

  if (dec == 1 && trigDec == false) {
    calibration_factor--;
    trigDec = true;
  }
  else {
    trigDec = false;
  }
}

void loop()
{
  timbangan();
  calFactor();

  Mb.MbData[10] = int(calibration_factor);
  Mb.MbData[5] = kirimData;
  Serial.print("Berat : ");
  Serial.println(kirimData);

  Mb.MbsRun();
}
