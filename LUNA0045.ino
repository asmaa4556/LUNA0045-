
#include <Arduino.h>
#include <DHT.h>
#include <Servo.h>

// ---------- SENSOR PINS ----------
#define DHT_PIN D4
#define DHT_TYPE DHT11

#define SOIL_MUX_PIN D0
#define LIGHT_MUX_PIN D3

// ---------- OUTPUT PINS ----------
#define PUMP_PIN D1
#define LIGHT_PIN D2

#define MOTOR_IN1 D5
#define MOTOR_IN2 D6

#define SERVO_PIN D7

// ---------- OBJECTS ----------
DHT dht(DHT_PIN, DHT_TYPE);
Servo soilServo;

// ---------- THRESHOLDS ----------
const int SOIL_DRY_LIMIT = 50;
const int LIGHT_DARK_LIMIT = 70;

// ---------- TIMING ----------
unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 10000;

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  pinMode(SOIL_MUX_PIN, OUTPUT);
  pinMode(LIGHT_MUX_PIN, OUTPUT);

  // Keep pump and light OFF at startup
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);

  soilServo.attach(SERVO_PIN);
  soilServo.write(90);

  Serial.println("LUNA0045 Started");
}

// ---------- SOIL SENSOR ----------
int readSoil() {
  digitalWrite(SOIL_MUX_PIN, LOW);
  digitalWrite(LIGHT_MUX_PIN, HIGH);

  delay(10);

  int raw = analogRead(A0);

  // Adjust calibration values after testing
  int soilPercent = map(raw, 800, 300, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  return soilPercent;
}

// ---------- LIGHT SENSOR ----------
int readLight() {
  digitalWrite(SOIL_MUX_PIN, HIGH);
  digitalWrite(LIGHT_MUX_PIN, LOW);

  delay(10);

  int raw = analogRead(A0);

  // Adjust calibration values after testing
  int lightPercent = map(raw, 300, 800, 0, 100);
  lightPercent = constrain(lightPercent, 0, 100);

  return lightPercent;
}

// ---------- MAIN LOOP ----------
void loop() {
  if (millis() - lastRead >= READ_INTERVAL) {
    lastRead = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    int soil = readSoil();
    int light = readLight();

    Serial.println("------ LUNA0045 ------");

    if (!isnan(temperature)) {
      Serial.print("Temperature: ");
      Serial.println(temperature);
    }

    if (!isnan(humidity)) {
      Serial.print("Humidity: ");
      Serial.println(humidity);
    }

    Serial.print("Soil Moisture: ");
    Serial.print(soil);
    Serial.println("%");

    Serial.print("Light Intensity: ");
    Serial.print(light);
    Serial.println("%");

    // WATERING CONDITION
    if (soil < SOIL_DRY_LIMIT) {
      Serial.println("SOIL DRY -> PUMP ON");
      digitalWrite(PUMP_PIN, HIGH);
    } else {
      Serial.println("SOIL OK -> PUMP OFF");
      digitalWrite(PUMP_PIN, LOW);
    }

    // AUXILIARY LIGHT CONDITION
    if (light < LIGHT_DARK_LIMIT) {
      Serial.println("LOW LIGHT -> AUXILIARY LIGHT ON");
      digitalWrite(LIGHT_PIN, HIGH);
    } else {
      Serial.println("LIGHT OK -> AUXILIARY LIGHT OFF");
      digitalWrite(LIGHT_PIN, LOW);
    }
  }
}
