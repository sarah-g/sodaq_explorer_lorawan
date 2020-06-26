#include <Sodaq_RN2483.h>

#define debugSerial SerialUSB
#define loraSerial Serial2

#define NIBBLE_TO_HEX_CHAR(i) ((i <= 9) ? ('0' + i) : ('A' - 10 + i))
#define HIGH_NIBBLE(i) ((i >> 4) & 0x0F)
#define LOW_NIBBLE(i) (i & 0x0F)

//Use OTAA, set to false to use ABP
bool OTAA = true;

// ABP
// USE YOUR OWN KEYS!
const uint8_t devAddr[4] =
{
    0x00, 0x00, 0x00, 0x00
};

// USE YOUR OWN KEYS!
const uint8_t appSKey[16] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// USE YOUR OWN KEYS!
const uint8_t nwkSKey[16] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// OTAA
// With using the GetHWEUI() function the HWEUI will be used
static uint8_t DevEUI[8]
{
    //00415A3C339B3D3D
    0x00, 0x41, 0x5A, 0x3C, 0x33, 0x9B, 0x3D, 0x3D
};


const uint8_t AppEUI[8] =
{
    //70B3D57ED00310AA
    0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x03, 0x10, 0xAA
};

const uint8_t AppKey[16] =
{
    //82E63ABB049CA8C76CFD80ADA5AB867E
    0x82, 0xE6, 0x3A, 0xBB, 0x04, 0x9C, 0xA8, 0xC7, 0x6C, 0xFD, 0x80, 0xAD, 0xA5, 0xAB, 0x86, 0x7E
};

void setup()
{
  delay(1000);

  while ((!debugSerial) && (millis() < 10000)){
    // Wait 10 seconds for debugSerial to open
  }

  debugSerial.println("Start");

  // Start streams
  debugSerial.begin(57600);
  loraSerial.begin(LoRaBee.getDefaultBaudRate());

  LoRaBee.setDiag(debugSerial); // to use debug remove //DEBUG inside library
  LoRaBee.init(loraSerial, LORA_RESET);

  //Use the Hardware EUI
  getHWEUI();

  // Print the Hardware EUI
  debugSerial.print("LoRa HWEUI: ");
  for (uint8_t i = 0; i < sizeof(DevEUI); i++) {
      debugSerial.print((char)NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(DevEUI[i])));
      debugSerial.print((char)NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(DevEUI[i])));
  }
  debugSerial.println();  

  setupLoRa();
}

void setupLoRa(){
  if(!OTAA){
    // ABP
    setupLoRaABP();
  } else {
    //OTAA
    setupLoRaOTAA();
  }
  // Uncomment this line to for the RN2903 with the Actility Network
  // For OTAA update the DEFAULT_FSB in the library
  // LoRaBee.setFsbChannels(1);

  LoRaBee.setSpreadingFactor(9);
}

void setupLoRaABP(){  
  if (LoRaBee.initABP(loraSerial, devAddr, appSKey, nwkSKey, true))
  {
    debugSerial.println("Communication to LoRaBEE successful.");
  }
  else
  {
    debugSerial.println("Communication to LoRaBEE failed!");
  }
}

void setupLoRaOTAA(){

  if (LoRaBee.initOTA(loraSerial, DevEUI, AppEUI, AppKey, true))
  {
    debugSerial.println("Network connection successful.");
  }
  else
  {
    debugSerial.println("Network connection failed!");
  }
}

void loop()
{
   int reading1 = getMilliVolts(TEMP_SENSOR);
   int reading2 = getMilliVolts(TEMP_SENSOR);
   debugSerial.print("reading1 = ");
   debugSerial.print(reading1);
   debugSerial.print(", reading2 = ");
   debugSerial.println(reading2);

   //convert readings to byte array to send across LoRaWAN network
   //define payload as byte array using 4 bytes for the payload - 2 bytes per reading
   //2 bytes can hold unsigned values from 0-65535 (so can handle range 0-3300 or 0-5000)
   //1 byte can only hold unsigned values from 0-255
   uint8_t payload[4]; 

   //add first reading as 2 bytes to payload byte array
   //e.g. 3124 = 00001100 00110100 as 2 byte binary
   payload[0] = (reading1 & 0xFF00) >> 8;  //mask lower byte (00110100) and shift bits 8 to the right to write higher byte (00001100)
   payload[1] = (reading1 & 0x00FF);  //mask higher byte and write lower byte (00110100)

   //add second reading as 2 bytes to payload byte array
   payload[2] = (reading2 & 0xFF00) >> 8;  //mask lower byte and shift bits 8 to the right to write higher byte
   payload[3] = (reading2 & 0x00FF);  //mask higher byte and write lower byte

   //print payload - print each byte in payload as HEX value
   debugSerial.print("payload = ");
   char hexBuffer[1];  //buffer to hold next hex value for printing
   for(int i=0; i<sizeof(payload); i++){
     sprintf(hexBuffer, "%02X", payload[i]);
     debugSerial.print(hexBuffer);
   }
   debugSerial.println();

    //switch (LoRaBee.send(1, (uint8_t*)reading.c_str(), reading.length()))
    switch (LoRaBee.send(1, payload, sizeof(payload)))
    {
    case NoError:
      debugSerial.println("Successful transmission.");
      break;
    case NoResponse:
      debugSerial.println("There was no response from the device.");
      break;
    case Timeout:
      debugSerial.println("Connection timed-out. Check your serial connection to the device! Sleeping for 20sec.");
      delay(20000);
      break;
    case PayloadSizeError:
      debugSerial.println("The size of the payload is greater than allowed. Transmission failed!");
      break;
    case InternalError:
      debugSerial.println("Oh No! This shouldn't happen. Something is really wrong! The program will reset the RN module.");
      setupLoRa();
      break;
    case Busy:
      debugSerial.println("The device is busy. Sleeping for 10 extra seconds.");
      delay(10000);
      break;
    case NetworkFatalError:
      debugSerial.println("There is a non-recoverable error with the network connection. The program will reset the RN module.");
      setupLoRa();
      break;
    case NotConnected:
      debugSerial.println("The device is not connected to the network. The program will reset the RN module.");
      setupLoRa();
      break;
    case NoAcknowledgment:
      debugSerial.println("There was no acknowledgment sent back!");
      break;
    default:
      break;
    }
    // Delay between readings
    // 60 000 = 1 minute
    delay(10000); 
}

int getMilliVolts(int pin)
{
  //Voltage at pin in milliVolts = (reading from ADC) * (3300/1024)
  //This formula converts the number 0-1023 from the ADC into 0-3300mV (= 3.3V)
  //for 5v boards change 3300 to 5000 to give 0-5000mV (= 5V)
  float mVolts = (float)analogRead(pin) * 3300.0 / 1023.0;
  return (int)round(mVolts);
}

/**
* Gets and stores the LoRa module's HWEUI/
*/
static void getHWEUI()
{
    uint8_t len = LoRaBee.getHWEUI(DevEUI, sizeof(DevEUI));
}
