#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Wi-Fi 정보
const char* ssid = "U+NetB225";          // Wi-Fi SSID
const char* password = "8265858BM!";                   // Wi-Fi Password (공개 Wi-Fi인 경우 빈 값)

// Firebase 정보
#define FIREBASE_HOST "https://jsha-iot-default-rtdb.firebaseio.com"  // Firebase Realtime Database URL
#define FIREBASE_AUTH "AIzaSyBRzJVr_N--c3hez5bVTgjufGQcSYkARqM"       // Firebase Database Secret

// 자리 번호 및 LED 핀 매핑
const int seatCount = 3;                     // 자리 개수
const int ledPins[seatCount] = {D1, D2, D3}; // 자리별 LED 핀 배열

FirebaseConfig config; // Firebase 설정 객체
FirebaseAuth auth;     // Firebase 인증 객체
FirebaseData firebaseData;

void setup() {
  Serial.begin(115200); // 시리얼 모니터 시작

  // LED 핀 초기화
  for (int i = 0; i < seatCount; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // 초기 상태는 OFF
  }

  // Wi-Fi 연결
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  // Firebase 설정
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);

  // 데이터베이스 초기화
  initializeDatabase();
}

void loop() {
  // 각 자리의 상태 확인 및 LED 제어
  for (int i = 0; i < seatCount; i++) {
    String seatPath = "/seats/" + String(i + 1); // Firebase 경로: /seats/1, /seats/2, /seats/3

    if (Firebase.getBool(firebaseData, seatPath)) {
      if (firebaseData.dataType() == "boolean") {
        bool seatStatus = firebaseData.boolData();

        // 자리 상태에 따라 LED 제어
        if (seatStatus) {
          digitalWrite(ledPins[i], HIGH); // 예약 상태: LED 켜기
          Serial.printf("Seat %d is reserved. LED ON.\n", i + 1);
        } else {
          digitalWrite(ledPins[i], LOW); // 미예약 상태: LED 끄기
          Serial.printf("Seat %d is not reserved. LED OFF.\n", i + 1);
        }
      }
    } else {
      Serial.printf("Failed to get data for seat %d: %s\n", i + 1, firebaseData.errorReason().c_str());
    }

    delay(100); // 자리별 요청 간 딜레이
  }

  delay(5000); // 전체 자리 상태 확인 후 5초 대기
}

// 데이터베이스 초기화 함수
void initializeDatabase() {
  if (!Firebase.getJSON(firebaseData, "/seats")) {
    Serial.println("No data found in the database. Initializing...");

    // FirebaseJson 객체로 기본 데이터 설정
    FirebaseJson json;
    json.set("1", false);
    json.set("2", false);
    json.set("3", false);

    if (Firebase.set(firebaseData, "/seats", json)) {
      Serial.println("Database initialized with default data.");
    } else {
      Serial.println("Failed to initialize database: " + firebaseData.errorReason());
    }
  } else {
    Serial.println("Database already initialized.");
  }
}
