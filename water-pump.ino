#include <PubSubClient.h>
#include <ESP8266WiFi.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Connection parameter
 * -Wifi -> internet connection foor data communication
 * -RabbitMQ/MQTT -> protoocol data communication
 */
const char* wifiSsid              = "Smartwatering";
const char* wifiPassword          = "Wikramaku1212";
const char* mqttHost              = "cloudrmqserver.pptik.id";
const char* mqttUserName          = "/WikramaBogor:SmkWikramaBogor";
const char* mqttPassword          = "Wikrama|bogor@2022|";
//const char* mqttClient            = "IOT-Water-Pumpp";
const char* mqttQueueLog          = "Log";
const char* mqttQueueAktuator     = "Aktuator";
const char* mqttQueueSensor       = "Sensor";
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*7
 * Device parameter
 * -Guid -> Id Device (unique) you can generate from here (https://www.uuidgenerator.net/version1) 
 * -status device -> save last state from the pump is on or off (1 = on , 0 = off) 
 * -pin microcontroller 
 * -mac device
 * 
 */
String deviceGuid                = "1acad8f0-5666-11ed-9b6a-0242ac120002"; //You can change this guid with your guid 
int devicePin                    = D1;


/*
 * Wifi setup WiFi client and mac address 
 */
WiFiClient espClient;
PubSubClient client(espClient);
byte mac[6]; //array temp mac address
String MACAddress;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Set up WiFi connection
 */
 void setup_wifi(){
  delay(10);
  //We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to :");
  Serial.println(wifiSsid);
  WiFi.begin(wifiSsid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Get Mac Address from microcontroller
 */
 
String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%2X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Print Mac Address 
 */
 void printMACAddress() {
  WiFi.macAddress(mac);
  MACAddress = mac2String(mac);
  Serial.println(MACAddress);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
 * Function for Get message payload from MQTT rabbit mq
 */
void callback(char* topic, byte* payload, unsigned int length){
  char message[5]; //variable for temp payload message
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Messagge :");
  for(int i = 0;i < length;i++){
    Serial.print((char)payload[i]);
    message[i] = (char)payload[i]; //initiate value from payload to message variable
    
  }
  Serial.println();
  Serial.println("-------------------------------");

  
  String dataSoil = String(message); //convert message to string from char*
  int triggerData = dataSoil.toInt(); //convert datasoil to int for trigger pump water relay


  /*
   * Status pump water
   * Condition -> you must convert string to char because format publish must with char type format
   */
   
  String pumpOff                   = String(deviceGuid + "#" + "0");
  char statusPumpOff [50];
  pumpOff.toCharArray(statusPumpOff, sizeof(statusPumpOff));
  
  String pumpOn                    = String(deviceGuid + "#" + "1");
  char statusPumpOn [50];
  pumpOn.toCharArray(statusPumpOn, sizeof(statusPumpOn));

  
  /*SOIL MOISTURE PARAMETER
   * DRY / KERING   >700 
   * WET / LEMBAB   <350
   * NORMAL         >350 AND <700
   * This condition will trigger pump water relay is on or off and send status to MQTT
   */


  if (triggerData > 700){
    client.publish(mqttQueueAktuator,statusPumpOn ); //format  publish ( char,char)
    digitalWrite(devicePin,LOW);
    delay(5000); 
    
    client.publish(mqttQueueAktuator,statusPumpOff);
    digitalWrite(devicePin,HIGH);
    Serial.println(pumpOff);
     Serial.println("Success Send Message");
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Reconnecting to MQTT/RabbitMQ 
 */
void reconnect() {
  // Loop until we're reconnected
  printMACAddress();
  const char* CL;
  CL = MACAddress.c_str();
  Serial.println(CL);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(CL, mqttUserName, mqttPassword)) {
      Serial.println("connected");
      client.subscribe(mqttQueueSensor);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      ESP.restart();
      delay(5000);

    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * Function for Setup Pin Mode,wifi,mqtt,and serial
 */
void setup()
{
  pinMode(devicePin, OUTPUT);

  digitalWrite(devicePin, HIGH);

  //pinMode(input, INPUT);
  Serial.begin(115200);
  setup_wifi();
  printMACAddress();
  client.setServer(mqttHost, 1883);
  client.setCallback(callback);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
 * This functioon for loop your program
 */
void loop() {
  //if you disconnected from wifi and mqtt
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
