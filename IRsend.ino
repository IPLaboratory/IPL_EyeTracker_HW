/*

  IR 신호값 송수신을 위한 코드
  IRRemote 라이브러리 기반으로 제작
  특정 리모컨 주파수를 입력받을 경우 변수로 저장되어있는 raw data 값을 해당 프로토콜에 맞춰 송신하는 코드 (해당 코드에서는 임의로 NEC프로토콜만을 가정)
  IR송신모듈같은 경우, 이해할 수 없는 오작동으로 인해 작동이 되지 않음 (왠만하면 센서를 사용했으면 좋겠음)

  <이 코드에서 추가해야할 내용>
   1. 수신에 관련된 코드를 작성해야 함 (어떤 raw data인지 입력을 받는 코드를 작성해야함)
   2. LED 사용해서 직관적인 송수신 오류 또는 완료에 관한 상태를 알려주는 코드를 작성해야 함 (하드웨어 설계도 같이..)

  아래는 각 프로토콜마다 사용해야하는 주파수임. raw data를 보낼때 해당 주파수에 맞추어서 보내야함
  <Protocol kHz>
    NEC 프로토콜: 38 kHz
    Sony SIRC 프로토콜: 40 kHz
    RC5 프로토콜 (Philips): 36 kHz
    RC6 프로토콜 (Philips): 36 kHz
    Panasonic 프로토콜: 35-37 kHz
    Sharp 프로토콜: 38 kHz
    Samsung 프로토콜: 38 kHz
    Mitsubishi 프로토콜: 38 kHz

  <PinMap>
    Arduino UNO
    IRsendLED / + : D4, - : GND
    IRreceiver(HX-MI 21) / DAT : D5, VCC : 5V, GND : GND

    ---- 작동하지 않았던 IRSender의 PinMap ----
    IRsender / S : D4, VCC : 5V, GND : GND

*/

#include <Arduino.h>
#include <IRremote.hpp>

// 핀 정의
#define IR_RECEIVE_PIN 5   // IR 수신 핀 설정
#define IR_SEND_PIN 4      // IR 송신 핀 설정 

// NEC 프로토콜 정의
#define NEC 38000  // NEC 프로토콜은 38kHz 사용

void setup() {
    Serial.begin(115200);
    
    // IR 송신기 설정
    IrSender.begin(IR_SEND_PIN);  // 송신 핀만 설정
    Serial.println(F("송신 준비 완료"));

    // IR 수신기 설정
    IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
    Serial.println(F("수신 준비 완료"));
}

void loop() { 
    if (IrReceiver.decode()) {
        uint32_t receivedValue = IrReceiver.decodedIRData.decodedRawData;  // 수신된 데이터를 가져옴

        Serial.print(F("Received IR signal: 0x"));
        Serial.println(receivedValue, HEX);

        // 수신된 값이 타겟 코드와 일치하는지 확인
        if (receivedValue == 0xE916FF00) {
            Serial.println(F("IR 보내는 중..."));
            
            // Raw 데이터로 송신 (raw 데이터를 받을때 앞에 0 값이 나오면 이건 제거해야함)
            uint16_t rawData[] = {8900, 4400, 550, 550, 600, 500, 600, 500, 600, 550, 550, 550, 600, 500, 600, 500, 600, 550, 550, 1650, 600, 1650, 550, 1650, 600, 1600, 600, 1650, 600, 1600, 600, 1650, 600, 1600, 600, 550, 550, 1650, 600, 1600, 600, 550, 550, 550, 600, 500, 600, 1650, 550, 550, 600, 1600, 600, 550, 550, 550, 600, 1600, 600, 1650, 550, 1650, 600, 550, 550, 1650, 600};
            IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC);
            Serial.println(F("IR raw data 보냄."));
        }
        
        // 다음 신호를 수신할 수 있도록 수신기 재설정
        IrReceiver.resume();
    }
}