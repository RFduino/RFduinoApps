#include <RFduinoBLE.h>

void setup() {
  Serial.begin(9600);
  
  RFduinoBLE.begin();
}

void loop() {
  if (Serial.available())
  {
    char ch = Serial.read();
    RFduinoBLE.send(ch == '1');
  }
}

void RFduinoBLE_onReceive(char *data, int len)
{
  Serial.write(data[0] ? "1" : "0");
}
