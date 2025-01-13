#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRtext.h>
#include <IRutils.h>

// Wi-Fi 네트워크 정보
const char* ssid = "IPL_2.4G";
const char* password = "ipl612112";

// Spring Boot 서버 URL
const char* springBootUrl = "http://192.168.0.77:8080/et/feature/receiveIr";

// ESP8266 웹 서버 생성 (포트 8088)
ESP8266WebServer server(8088);

// IR 송신 및 수신 핀 설정 및 변수
const uint16_t kRecvPin = D5;
const uint16_t IRsendPin = D6;
IRrecv irrecv(kRecvPin);
IRsend irsend(IRsendPin);
decode_results results;

// RGB LED 핀 설정
int RED = D1;
int GREEN = D2;
int BLUE = D3;

// 상태 변수
boolean booleanValue = false;  // Spring Boot로부터 받은 boolean 값
String irValue = "";  // 수신된 IR 값

// LED 상태 함수
void setLED(int r, int g, int b) {
  analogWrite(RED, r);
  analogWrite(GREEN, g);
  analogWrite(BLUE, b);
}

// Wi-Fi 연결 함수
void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  setLED(255, 0, 255);  // Wi-Fi 연결중 (RB)
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Wi-Fi 연결 실패 상태
    if (attempts > 20) {  
      setLED(255, 0, 0);  // Wi-Fi 연결 실패 (R)
      return;
    }
  }
  
  // Wi-Fi 연결 성공 상태
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  setLED(0, 0, 255);  // Wi-Fi 연결 성공 (B)
}

// Spring Boot로 데이터를 전송하는 함수
void sendToSpringBoot(String irValue) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, springBootUrl);
    http.addHeader("Content-Type", "application/json");

    String postData = "{\"irValue\":\"" + irValue + "\"}";
    
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from Spring Boot: " + response);
      setLED(0, 0, 255);  // 전송 성공 (B)
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
      setLED(255, 0, 0);  // 전송 실패 (R)
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
    setLED(255, 0, 0);  // Wi-Fi 연결 실패 상태 (R)
  }
}

// Spring Boot에서 boolean 값을 처리하는 핸들러
void handleSendBoolean() {
  if (server.hasArg("plain")) {
    String requestParam = server.arg("plain");
    Serial.print("Received boolean value from Spring Boot: ");
    Serial.println(requestParam);

    if (requestParam == "true") {
      booleanValue = true;
      irrecv.enableIRIn();  // IR 수신 시작
      server.send(200, "application/json", "{\"message\":\"IR receiving started\", \"status\":200}");
      Serial.println("IR receiving started");
      setLED(0, 255, 255);  // 스프링부트로부터 1값 수신 (GB)
    } else {
      booleanValue = false;
      irrecv.disableIRIn();  // IR 수신 중지
      server.send(200, "application/json", "{\"message\":\"IR receiving stopped\", \"status\":200}");
      Serial.println("IR receiving stopped");
      setLED(255, 0, 0);  // 상태 중지 (R)
    }
  } else {
    server.send(400, "application/json", "{\"message\":\"No boolean value received\", \"status\":400}");
    Serial.println("No boolean value received from Spring Boot");
    setLED(255, 0, 0);  // 수신 실패 상태 (R)
  }
}

// IR 값을 수신하고 Spring Boot로 전송하는 함수
void receiveIR() {
  if (booleanValue && irrecv.decode(&results)) {
    irValue = "0x" + String(results.value, HEX);
    irValue.toUpperCase();  // 대문자로 변환

    Serial.printf("Received IR Code: %s (%d Bits)\n", irValue.c_str(), results.bits);

    sendToSpringBoot(irValue);  // Spring Boot로 전송
    booleanValue = false;  // 값을 한 번 전송한 후 전송 중지
    irrecv.resume();  // 다음 IR 수신 준비
  }
}

// Spring Boot에서 IR 송신을 요청하는 핸들러
void handleSendIR() {
  if (server.hasArg("plain")) {
    String irCodeStr = server.arg("plain");
    Serial.print("Received IR code from Spring Boot for transmission: ");
    Serial.println(irCodeStr);

    uint32_t irCode = (uint32_t)strtol(irCodeStr.c_str(), NULL, 16);

    if (irCode != 0) {
      irsend.sendNEC(irCode);
      Serial.printf("Sent IR Code: 0x%X\n", irCode);
      sendToSpringBoot("success");  // 전송 성공 응답
      setLED(255, 0, 255);  // IR 송신 성공 (RB)
      delay(2000);
      setLED(0, 0, 255);
    } else {
      Serial.println("Invalid IR code format");
      sendToSpringBoot("fail");  // 전송 실패 응답
      setLED(255, 0, 0);  // IR 송신 실패 (R)
    }
    server.send(200, "application/json", "{\"message\":\"IR transmission complete\", \"status\":200}");
  } else {
    server.send(400, "application/json", "{\"message\":\"No IR code received\", \"status\":400}");
    Serial.println("No IR code received from Spring Boot");
    setLED(255, 0, 0);  // IR 코드 수신 실패 (R)
  }
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  irsend.begin();

  server.on("/sendBoolean", HTTP_POST, handleSendBoolean);
  server.on("/sendIR", HTTP_POST, handleSendIR);

  server.begin();
  Serial.println("HTTP server started and waiting for client connections");
}

void loop() {
  server.handleClient();
  receiveIR();
}
