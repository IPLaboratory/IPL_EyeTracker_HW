#include <IRremote.h>

// 센서 핀 정의
const int RECV_PIN = 5;
// IR 리시버 객체 정의
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  // 시리얼 모니터 설정 (9600 baud)
  Serial.begin(9600);
  // IR 리시버 활성화
  irrecv.begin(RECV_PIN);
}

void loop() {
  if (irrecv.decode()) {
        Serial.print("Received: 0x");
        Serial.println(irrecv.decodedIRData.decodedRawData, HEX);

        // 프로토콜 출력
        switch (irrecv.decodedIRData.protocol) {
            case NEC: 
              Serial.println("Protocol: NEC"); 
              break;
            case SONY: 
              Serial.println("Protocol: SONY"); 
              break;
            case RC5: 
              Serial.println("Protocol: RC5"); 
              break;
            case RC6: 
              Serial.println("Protocol: RC6"); 
              break;
            case SHARP: 
              Serial.println("Protocol: SHARP"); 
              break;
            case JVC: 
              Serial.println("Protocol: JVC"); 
              break;
            case SAMSUNG: 
              Serial.println("Protocol: SAMSUNG"); 
              break;
            case LG: 
              Serial.println("Protocol: LG"); 
              break;
            case WHYNTER: 
              Serial.println("Protocol: WHYNTER"); 
              break;
            case PANASONIC: 
              Serial.println("Protocol: PANASONIC"); 
              break;
            case DENON: 
              Serial.println("Protocol: DENON"); 
              break;
            default: 
              Serial.println("Protocol: UNKNOWN"); 
              break;
        }

        // rawData 출력
        Serial.print("RawData: ");
        for (uint8_t i = 0; i < irrecv.decodedIRData.rawDataPtr->rawlen; i++) {
          Serial.print(irrecv.decodedIRData.rawDataPtr->rawbuf[i] * USECPERTICK);
          Serial.print(" ");
        }
        Serial.println();
        
        irrecv.resume(); // 다음 신호 수신 대기
  }
}