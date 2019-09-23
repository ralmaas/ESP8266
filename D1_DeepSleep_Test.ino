/*
 * Test program for Deep Sleep
 * Wakeup based on external circuit pulsing the RST input
 * 
 * 2019-09-23/ralm
 *
 * Example of MQTT output:
 *	TEST/sleeper/status Starting
 *	TEST/sleeper/buttonA 0
 *	TEST/sleeper/buttonB 1
 *	TEST/sleeper/voltage  4.01
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "your ssid";
const char* password = "your password";
const char* mqtt_server = "address of MQTT-server";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    // If user/pwd:
    // if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("TEST/sleeper/status", "Starting");
      // ... and resubscribe
      client.subscribe("TEST/sleeper/cmd");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

float getVoltage()
{
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (4.52 / 1024.0);
  return voltage;
}

int buttonA = 0;
int buttonB = 0;

void setup()
{
  // Start by sampling the input pins ASAP!!!
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  delay(5);
  // Get button state ASAP !!!!!
  buttonA = digitalRead(D1);
  buttonB = digitalRead(D2);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Receiver Module starting up");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

int status = 0;
char  msg[30];

void loop()
{
  // Waking up - first entry to this section
  /* */
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (status == 0) {
    // First time in loop (this could have been moved to setup function...)
    sprintf(msg, "%d", buttonA);
    Serial.print("buttonA: "); Serial.println(buttonA);
    client.publish("TEST/sleeper/buttonA", msg);
    sprintf(msg, "%d", buttonB);
    Serial.print("buttonB: "); Serial.println(buttonB);
    client.publish("TEST/sleeper/buttonB", msg);

    float voltage = getVoltage();
    sprintf(msg, "%5.2f", voltage);
    Serial.print("Voltage: "); Serial.println(voltage);
    client.publish("TEST/sleeper/voltage", msg);

    buttonA = digitalRead(D1);
    buttonB = digitalRead(D2);
    Serial.print("buttonA: "); Serial.println(buttonA);
    Serial.print("buttonB: "); Serial.println(buttonB);

    status = 1;
  }
  delay(1000);
  Serial.println("Going to sleep");
  ESP.deepSleep(0); 
}
