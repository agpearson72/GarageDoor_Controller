#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Bubbles";
const char* password = "Cincy17!!";

#define RELAY_PIN 22             // GPIO for relay
#define SENSOR_BOTTOM_PIN 33     // Bottom of the door track (closed position)
#define SENSOR_TOP_PIN 32        // Top of the door track (open position)

WebServer server(8080);

// Optional simple key for trigger protection
const String triggerSecret = "Password1";
// const String triggerSecret = "Password2";

// JSON response helper
void sendJSONResponse(int code, const String& key, const String& value) {
  String response = "{ \"" + key + "\": \"" + value + "\" }";
  server.send(code, "application/json", response);
}

// Evaluate the current door state using both sensors
String getDoorState() {
  bool bottomClosed = digitalRead(SENSOR_BOTTOM_PIN) == LOW; // LOW = magnet detected
  bool topClosed = digitalRead(SENSOR_TOP_PIN) == LOW;

  if (bottomClosed && !topClosed) {
    return "closed";
  } else if (!bottomClosed && topClosed) {
    return "open";
  } else {
    return "moving";
  }
}

// Handles garage door trigger
void handleTrigger() {
  if (server.hasArg("key") && server.arg("key") == triggerSecret) {
    digitalWrite(RELAY_PIN, HIGH);
    delay(500);
    digitalWrite(RELAY_PIN, LOW);
    sendJSONResponse(200, "status", "garage door right triggered");
    // sendJSONResponse(200, "status", "garage door left triggered");
  } else {
    sendJSONResponse(403, "error", "unauthorized");
  }
}

// Handles garage door sensor status
void handleStatus() {
  String state = getDoorState();
  sendJSONResponse(200, "right_door_status", state);
  // sendJSONResponse(200, "left_door_status", state);
}

// Optional root endpoint to confirm the API is alive
void handleRoot() {
  sendJSONResponse(200, "status", "right garage controller online");
  // sendJSONResponse(200, "status", "left garage controller online");
}

void setup() {
  Serial.begin(115200);

  // Set pin modes
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(SENSOR_BOTTOM_PIN, INPUT_PULLUP);
  pinMode(SENSOR_TOP_PIN, INPUT_PULLUP);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected. IP address: " + WiFi.localIP().toString());

  // Define web server routes
  server.on("/", handleRoot);
  server.on("/trigger", handleTrigger);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
}