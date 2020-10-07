/*
    Modbus TCP slave.
*/

// Note: The Arduino IDE does not respect conditional included
// header files in the main sketch so you have to select your
// here.
//

#define ModbusDebug     //serial debug enable
#define MB_PORT 502  
#define MB_ID   1

#include <ESP8266WiFi.h>
WiFiServer ModbusServer(MB_PORT);

#define LED_PIN                         2
#define maxInputRegister                20
#define maxHoldingRegister              20

// MODBUS Function Codes
#define MB_FC_NONE                      0
#define MB_FC_READ_COILS                1   // not implemented
#define MB_FC_READ_DISCRETE_INPUT       2   // not implemented
#define MB_FC_READ_REGISTERS            3   // implemented
#define MB_FC_READ_INPUT_REGISTERS      4   // implemented
#define MB_FC_WRITE_COIL                5   // not implemented
#define MB_FC_WRITE_REGISTER            6   // implemented
#define MB_FC_WRITE_MULTIPLE_COILS      15
#define MB_FC_WRITE_MULTIPLE_REGISTERS  16  // implemented
// MODBUS Error Codes
#define MB_EC_NONE                      0
#define MB_EC_ILLEGAL_FUNCTION          1   // Function Code not Supported
#define MB_EC_ILLEGAL_DATA_ADDRESS      2   // Output Address not exists
#define MB_EC_ILLEGAL_DATA_VALUE        3   // Output Value not in Range
#define MB_EC_SLAVE_DEVICE_FAILURE      4   // Slave Deive Fails to process request
// MODBUS MBAP offsets
#define MB_TCP_TID                      0   // Transaction IDentifier
#define MB_TCP_PID                      2   // Protocol IDentifier
#define MB_TCP_LEN                      4   // Message LENgth
#define MB_TCP_UID                      6   // Unit IDentifier
#define MB_TCP_FUNC                     7   // The FUNCtion Code
#define MB_TCP_REGISTER_START           8
#define MB_TCP_REGISTER_NUMBER          10

byte ByteArray[260];
bool ledPinStatus = LOW;
unsigned int  MBInputRegister[maxInputRegister];
unsigned int  MBHoldingRegister[maxHoldingRegister];

void ModbusTCPBegin(const char *ssid, const char *key,uint8_t ip[4],uint8_t gateway[4],uint8_t subnet[4])
{
  //WiFiServer(MB_PORT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledPinStatus);
  ledPinStatus = LOW;
  
  #ifdef ModbusDebug
    Serial.begin(115200);
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  #endif

  WiFi.config(IPAddress(ip), IPAddress(gateway), IPAddress(subnet));
  WiFi.begin(ssid, key);
    
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledPinStatus); // Write LED high/low
    delay(100);
    
    #ifdef ModbusDebug
      Serial.print(".");
    #endif
  }
  // Start the server
  digitalWrite(LED_PIN,  !ledPinStatus);
  ModbusServer.begin();
  
  #ifdef ModbusDebug
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("WiFi signal");
    Serial.println(WiFi.RSSI());
    Serial.print(F("Listening on "));
    Serial.print(MB_PORT);
    Serial.println("...");
  #endif

  MBInputRegister[0] = 10;
  MBInputRegister[1] = 20;
  MBInputRegister[2] = 30;
  MBInputRegister[3] = 40;
  MBInputRegister[4] = 50;

  MBHoldingRegister[0] = 1;
  MBHoldingRegister[1] = 2;
  MBHoldingRegister[2] = 3;
  MBHoldingRegister[3] = 4;
  MBHoldingRegister[4] = 5;

  
}

void ModbusTCPRun()
{
  boolean flagClientConnected = 0;
  byte byteFN = MB_FC_NONE;
  int Start;
  int WordDataLength;
  int ByteDataLength;
  int MessageLength;
  
  //****************** Read from socket ****************
  WiFiClient client = ModbusServer.available();

  while (client.connected())
  //if (client.connected())
  {
    ModbusTCPInOut();
    
    if(client.available())
    {
      digitalWrite(LED_PIN, ledPinStatus);
      flagClientConnected = 1;
      int i = 0;
      while(client.available())
      {
        ByteArray[i] = client.read();
        i++;
      }
      client.flush();
      byteFN = ByteArray[MB_TCP_FUNC];
      Start = word(ByteArray[MB_TCP_REGISTER_START],ByteArray[MB_TCP_REGISTER_START+1]);
      WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
      
      #ifdef ModbusDebug
        Serial.println();
        Serial.print("RX: ");
        for (byte thisByte = 0; thisByte < 20; thisByte++)
        {
          Serial.print(ByteArray[thisByte], DEC);
          Serial.print("-");
        }
        Serial.println();
        Serial.print("Ricevuta Funzione: ");
        Serial.println(byteFN);
      #endif
      digitalWrite(LED_PIN, !ledPinStatus);
    }
  
    // Handle request
    if (MB_ID == ByteArray[6])
      switch(byteFN)
      {
        case MB_FC_NONE:
          break;

        case MB_FC_READ_REGISTERS:  // 03 Read Holding Registers
          ByteDataLength = WordDataLength * 2;
          ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
          ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).
          for(int i = 0; i < WordDataLength; i++)
          {
            ByteArray[ 9 + i * 2] = highByte(MBHoldingRegister[Start + i]);
            ByteArray[10 + i * 2] =  lowByte(MBHoldingRegister[Start + i]);
          }
          MessageLength = ByteDataLength + 9;
          client.write((const uint8_t *)ByteArray,MessageLength);
          byteFN = MB_FC_NONE;
          
          #ifdef ModbusDebug
            Serial.print("TX: ");
            for (byte thisByte = 0; thisByte <= MessageLength; thisByte++)
            {
              Serial.print(ByteArray[thisByte], DEC);
              Serial.print("-");
            }
            Serial.println();
          #endif
          
          break;
          
        case MB_FC_READ_INPUT_REGISTERS:  // 04 Read Input Registers
          Start = word(ByteArray[MB_TCP_REGISTER_START],ByteArray[MB_TCP_REGISTER_START+1]);
          WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
          ByteDataLength = WordDataLength * 2;
          ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
          ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).
          for(int i = 0; i < WordDataLength; i++)
          {
            ByteArray[ 9 + i * 2] = highByte(MBInputRegister[Start + i]);
            ByteArray[10 + i * 2] =  lowByte(MBInputRegister[Start + i]);
          }
          MessageLength = ByteDataLength + 9;
          client.write((const uint8_t *)ByteArray,MessageLength);
          byteFN = MB_FC_NONE;
          
          #ifdef ModbusDebug
            Serial.print("TX: ");
            for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
              Serial.print(ByteArray[thisByte], DEC);
              Serial.print("-");
            }
            Serial.println();
          #endif
          
          break;
            
        case MB_FC_WRITE_REGISTER:  // 06 Write Holding Register
          MBHoldingRegister[Start] = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
          ByteArray[5] = 6; //Number of bytes after this one.
          MessageLength = 12;
          client.write((const uint8_t *)ByteArray,MessageLength);
          byteFN = MB_FC_NONE;
          
          #ifdef ModbusDebug
            Serial.print("TX: ");
            for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
              Serial.print(ByteArray[thisByte], DEC);
              Serial.print("-");
            }
            Serial.println();
            Serial.print("Write Holding Register: ");
            Serial.print(Start);
            Serial.print("=");
            Serial.println(MBHoldingRegister[Start]);
          #endif
          
          break;
            
        case MB_FC_WRITE_MULTIPLE_REGISTERS:    //16 Write Holding Registers
          ByteDataLength = WordDataLength * 2;
          ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
          for(int i = 0; i < WordDataLength; i++)
          {
            MBHoldingRegister[Start + i] =  word(ByteArray[ 13 + i * 2],ByteArray[14 + i * 2]);
          }
          MessageLength = 12;
          client.write((const uint8_t *)ByteArray,MessageLength);
          byteFN = MB_FC_NONE;
          
          #ifdef ModbusDebug
            Serial.print("TX: ");
            for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
              Serial.print(ByteArray[thisByte], DEC);
              Serial.print("-");
            }
            Serial.println();
            Serial.print("Write Holding Registers from: ");
            Serial.print(Start);
            Serial.print("=");
            Serial.println(WordDataLength);
          #endif
  
          break;

        default:
          ByteArray[5] = 3; //Number of bytes after this one.
          ByteArray[7] = B10000000 | ByteArray[MB_TCP_FUNC];
          ByteArray[8] = MB_EC_ILLEGAL_FUNCTION;
          client.write((const uint8_t *)ByteArray,9);
          byteFN = MB_FC_NONE;
          
          break;
      }
  }
  client.stop();
  
  if (flagClientConnected == 1)
  {
    #ifdef ModbusDebug
      Serial.println("client disonnected");
    #endif
    
    flagClientConnected = 0;
  }
}

unsigned long timer;
unsigned int  i;
void ModbusTCPInOut()
{
  if (millis() - timer >= 5)
  {
    timer = millis();
    i++;
    MBInputRegister[0] = i;
  }
  digitalWrite(LED_PIN,MBHoldingRegister[0]);
}

