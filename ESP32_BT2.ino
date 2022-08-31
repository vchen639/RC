#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32MotorControl.h> 

// Motor R
int IN1 = 27; 
int IN2 = 26; 
int ENA = 14;

// Motor L
int IN3 = 15; 
int IN4 = 13; 
int ENB = 12; 

// Setting PWM properties
const int freq = 30000;
const int pwmChannel1 = 0;
const int pwmChannel2 = 1;
const int resolution = 8;
int dutyCycle = 200;

#define motorR 0
#define motorL 1

#define FULLSPEED 100
#define HALFSPEED 50

ESP32MotorControl motor;

// Main Loop vars
char *BLE_RXbuf;
char BLEcmd;
bool BLE_RXflag = false;

// BLE defs & vars
// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        BLE_RXbuf = (char *)rxValue.c_str();
        BLEcmd = BLE_RXbuf[0];
        BLE_RXflag = true; // set BLE RX flag
      }
    }
};

void move_forward(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);  
  Serial.println("Move Forward");
}

void move_backward(){
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Move Backward"); 
  
}

void move_left(){
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH); 
  Serial.println("Move Left");
 
}
void move_right(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW); 
  Serial.println("Move Right");
}

void no_move(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  Serial.println("Stop");
  
}
void setup() {
  Serial.begin(115200);
  Serial.println("Motor Pins assigned...");
  
  // Motor control setup
  motor.attachMotors(IN1, IN2, IN3, IN4);
  motor.motorStop(motorR);
  motor.motorStop(motorL);
    
  // Create the BLE Device
  BLEDevice::init("UWU");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(ENA,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  pinMode(ENB,OUTPUT);

  ledcSetup(pwmChannel1, freq, resolution);
  ledcSetup(pwmChannel2, freq, resolution);
  
  ledcAttachPin(ENA, pwmChannel1);
  ledcAttachPin(ENB, pwmChannel2);
}

void loop() {
  String tx;
  int Speed = FULLSPEED;

  // BLE disconnecting
  if (!deviceConnected && oldDeviceConnected) {
     delay(500); // give the bluetooth stack the chance to get things ready
     pServer->startAdvertising(); // restart advertising
     Serial.println("BLE start advertising");
     oldDeviceConnected = deviceConnected;
  }

  // BLE connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("BLE device connected");
  }

  // check BLE command
  if (BLE_RXflag) {
    Serial.print(BLEcmd); Serial.print(" "); Serial.println(BLE_RXbuf);
    
    switch (BLEcmd) {
      case 'F': move_forward();
                break;
      case 'B': move_backward();
                break;
      case 'R': move_right();           
                break;
      case 'L': move_left();
                break;
      case 'S': no_move();
                break;  
                
    }
    BLE_RXflag = false;
  }
  ledcWrite(pwmChannel1, dutyCycle);
  ledcWrite(pwmChannel2, dutyCycle);
}
