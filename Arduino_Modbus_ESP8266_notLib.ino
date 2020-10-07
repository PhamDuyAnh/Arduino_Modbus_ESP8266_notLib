/*
 **************************************************************
 * Modbus TCP/IP test Function
 * Created by Pham Duy Anh - CKD - phamduyanh@gmail.com
 * Email:  phamduyanh@gmail.com
 * Mobi:   0908984010
 * Created: 27-08-2020
 * Updated: 
 *
 * Supported Register Function codes:
 *   - 04 - unsigned int --> Read Analog Input Registers                     
 *   - 03 - unsigned int --> Read Analog Output Holding Registers
 *   - 06 - unsigned int --> Write single Analog Output Holding Register
 *   - 16 - unsigned int --> Write multiple Analog Output Holding Registers
 **************************************************************

 Add ESP8266 library
 => Rreferences -> Additional Boards Manager URLs: http://arduino.esp8266.com/stable/package_esp8266com_index.json
 => Tools -> Boards -> Boards Manager -> esp8266 by ESP8266 Community version 2.0.0 (only use v2.0.0)
 */
//WIFI Settings
byte ip[]      = { 192, 168,   1, 6 };
byte gateway[] = { 192, 168,   1, 1 };
byte subnet[]  = { 255, 255, 255, 0 };

void setup()
{
  ModbusTCPBegin("Wonder light", "@wonderlight", ip, gateway, subnet);
}

void loop()
{
  //WiFiClient client = ModbusServer.available();
  ModbusTCPRun();
  ModbusTCPInOut();
}
