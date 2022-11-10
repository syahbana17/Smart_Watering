#include <PubSubClient.h>
#include <ESP8266WiFi.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   Connection parameter
   -Wifi -> internet connection foor data communication
   -RabbitMQ/MQTT -> protoocol data communication
*/
const char* wifiSsid              = "Smartwatering";
const char* wifiPassword          = "Wikramaku1212";
const char* mqttHost              = "cloudrmqserver.pptik.id";
const char* mqttUserName          = "/WikramaBogor:SmkWikramaBogor";
const char* mqttPassword          = "Wikrama|bogor@2022|";
//const char* mqttClient            = "IOT-Water-Pumpp";
const char* mqttQueueLog          = "Log";
const char* mqttQueueSensor       = "Sensor";
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*7
   Device parameter
   -Guid -> Id Device (unique) you can generate from here (https://www.uuidgenerator.net/version1)
   -status device -> save last state from the pump is on or off (1 = on , 0 = off)
   -pin microcontroller
   -mac device

*/
String deviceGuid                = "3bea088a-5666-11ed-9b6a-0242ac120002"; //You can change this guid with your guid
int devicePin                    = A0;


/*
   Wifi setup WiFi client and mac address
*/
WiFiClient espClient;
PubSubClient client(espClient);
byte mac[6]; //array temp mac address
String MACAddress;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   Set up WiFi connection
*/
void setup_wifi() {
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
   Function for Get Mac Address from microcontroller
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
   Function for Print Mac Address
*/
void printMACAddress() {
  WiFi.macAddress(mac);
  MACAddress = mac2String(mac);
  Serial.println(MACAddress);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
   Function for Get message payload from MQTT rabbit mq
*/
void callback(char* topic, byte* payload, unsigned int length) {


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   Function for Reconnecting to MQTT/RabbitMQ
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
   Function for Setup Pin Mode,wifi,mqtt,and serial
*/
void setup()
{
  pinMode(devicePin, INPUT);
  Serial.begin(115200);
  setup_wifi();
  printMACAddress();
  client.setServer(mqttHost, 1883);
  client.setCallback(callback);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
   This functioon for loop your program
*/
void loop() {
  //if you disconnected from wifi and mqttu
  if (!client.connected()) {
    reconnect();
  }

  const int valueSoil = analogRead(devicePin);
  const int dataSoil  = valueSoil;
  //Data to Mobile
  String dataSoilMobile = String(deviceGuid + "#" + dataSoil);
  char dataToMobile[50];
  dataSoilMobile.toCharArray(dataToMobile, sizeof(dataToMobile));
  if ( client.publish(mqttQueueLog, dataToMobile) == true) {
    Serial.println("Success sending message to Mobile apps");
  }
  else {
    Serial.println("Error sending message to Mobile");
    ESP.restart();

  }

  //Data to Pump water
  String dataSoilPump = String(dataSoil);
  char dataToPump[5];
  dataSoilPump.toCharArray(dataToPump, sizeof(dataToPump));
  if (client.publish(mqttQueueSensor, dataToPump) == true){
    Serial.println("Success sending message to Mobile apps");
}
else {
  Serial.println("Error sending message to Log");
  ESP.restart();

}

client.loop();
delay(10000);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
