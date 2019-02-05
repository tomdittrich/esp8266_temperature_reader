#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "RestClient.h"

const char* ssid     = "YOUR_WLAN";
const char* password = "YOUR_WLAN_PASSWORD";
const char* host = "YOUR_WEATHERSTATION_IP";

struct SensorData {
  double temperature;
  double humidity;
  double particulateMatter25; // PM2.5
  double particulateMatter10; // PM10
};

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  SensorData sensorDataOutside;
  String rawData = getRawDataFromHost();
  deserializeRawData(sensorDataOutside, rawData);

  Serial.println(sensorDataOutside.temperature);
  Serial.println(sensorDataOutside.humidity);

  Serial.println(ESP.getFreeHeap());
  
  delay(500);
}

String getRawDataFromHost(){
  RestClient client = RestClient(host);
  String response = "";
  int statusCode = client.get("/data.json", &response);
  
  Serial.print("Status code from server: ");
  Serial.println(statusCode);

  return response;
}

bool deserializeRawData(SensorData& data, String response){
  int responseLength = response.length() + 1;
  char responseAsArray[responseLength];
  response.toCharArray(responseAsArray, responseLength);
  DynamicJsonBuffer jsonBuffer(950); // https://arduinojson.org/v5/assistant/

  Serial.print("Response body from server: ");
  Serial.println(responseAsArray);
  
  JsonObject& jsonObject = jsonBuffer.parseObject(responseAsArray);
  JsonArray& sensorDataValues = jsonObject["sensordatavalues"];

  data.temperature = sensorDataValues[2]["value"];
  data.humidity = sensorDataValues[3]["value"];

  // just to be on the safe-side
  jsonBuffer.clear();
  
  return (jsonObject.success() && sensorDataValues.success());
}
