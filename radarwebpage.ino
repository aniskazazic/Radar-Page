#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Servo.h>

#define WIFI_SSID "wifi"
#define WIFI_PASSWORD "password"
#define DATABASE_URL "database_url"  
#define API_KEY "Api_key" 
#define trigPin D6
#define echoPin D5

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

Servo myServo; 

unsigned long lastDistanceCheck = 0;
unsigned long lastServoMove = 0; 

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Attach servo to a pin
  myServo.attach(D1);
  myServo.write(90); 

  // Wi-Fi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL; 


  auth.user.email = "email";
  auth.user.password = "password";

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Waiting for Firebase to initialize...");
  delay(2000);

  if (Firebase.ready()) {
    Serial.println("Firebase is ready!");
  } else {
    Serial.print("Firebase initialization failed: ");
    Serial.println(firebaseData.errorReason());
  }
}

void loadDistance() {
  float distance, duration;

  // Trigger ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0344 / 2;

  // Print distance
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Send data to Firebase
  if (Firebase.ready()) {
    Serial.println("Sending data to Firebase...");
    String path = "/Distance"; // Path in the Firebase Realtime Database
    if (Firebase.RTDB.pushFloat(&firebaseData, path.c_str(), distance)) {
      Serial.println("Data saved to Firebase successfully!");
    } else {
      Serial.print("Error sending data: ");
      Serial.println(firebaseData.errorReason());
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Move servo to simulate "opening" and "closing"
  if (currentMillis - lastServoMove >= 3000) { 
    lastServoMove = currentMillis;

    myServo.write(60); // Move to 60 degrees
    delay(3000);       // Wait for 1 second
    myServo.write(90); // Move back to 90 degrees (neutral position)
    delay(2000);       // Wait for 2 seconds
    myServo.write(120); // Move to 120 degrees
    delay(1000);       // Wait for 1 second
    myServo.write(90); // Move back to 90 degrees (neutral position)

    Serial.println("Servo completed one open-close cycle.");
  }

  // Measure and send distance every 2 seconds
  if (currentMillis - lastDistanceCheck >= 2000) { 
    lastDistanceCheck = currentMillis;
    loadDistance();
  }
}
