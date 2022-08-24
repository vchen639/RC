#include <BLEDevice.h> 
#include <BLEServer.h> 
#include <BLEUtils.h> 
#include <BLE2902.h> 
#include <String.h> 

// Motor 1
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;

// Motor 2
int motor2Pin1 = 32; 
int motor2Pin2 = 33; 
int enable2Pin = 25; 

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t txValue = 0;
long lastMsg = 0;
String order="Test\n";

/*為BLE供應商定義指定的 UUID 編號*/
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/*為BLE服務器編寫一個Callback函式來管理BLE的連接。*/
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

/*具有BLE功能的Callback函式。 
 *調用時，移動終端向 ESP32 發送數據時，會將其存儲到 reload 中。
 */
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        order="";
        for (int i = 0; i < rxValue.length(); i++){
          order +=(char)rxValue[i];
        }
      }
    }
};

/*創建BLE server流程：
 *1. Create a BLE Server
 *2. Create a BLE Service
 *3. Create a BLE Characteristic on the Service
 *4. Create a BLE Descriptor on the characteristic 
 *5. Start the service
 *6. Start advertising
 */
void setupBLE(String BLEName){
  const char *ble_name=BLEName.c_str();
  BLEDevice::init(ble_name);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID); 
  pCharacteristic= pService->createCharacteristic(CHARACTERISTIC_UUID_TX,BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks()); 
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void move_forward(){
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);  
  delay(1000); 
}

void move_backward(){
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW); 
  delay(1000);
}

void move_left(){
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH); 
  delay(1000);
}
void move_right(){
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW); 
  delay(1000);
}

void no_move(){
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  setupBLE("Yahee"); //初始化BLE並為其命名，藍牙顯示名稱可自行更改
  
  // sets the pins as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);
  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(enable2Pin, pwmChannel);
  
}


/*當手機通過BLE藍牙向ESP32發送訊息時，會通過ESP32的序列視窗顯示出來； 
 *經由ESP32的序列視窗發送訊息後，會通過BLE藍牙發送到手機。
 */
void loop() {
  long now = millis();
  if (now - lastMsg > 100) {
    if (deviceConnected&&order.length()>0) {
        Serial.println(order);

        if (order == "F"){
          move_forward();
          Serial.println("f");
         }

        else if (order == "B"){
           move_backward();
           Serial.println("b");
         }

        else if (order == "L"){
          move_left();
          Serial.println("l");
         }
         
        else if (order = "R"){
          move_right();
           Serial.println("r");
        }

        else{
          no_move();
         }
        
        order="";
    }
   
    lastMsg = now;
  }
}
