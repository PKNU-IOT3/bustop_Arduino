# 센서 제어를 위한 Arduino 구현
2023.06.30 제작

## 개발목적 
- MySQL과 통신하여 버스 탑승 대기 인원에 따른 센서 제어를 위함
  - LED 센서 
  - 터치 센서

## 기술스택
- C
- MySQL
- nodeMCU(ESP8266)
- GPIO

## 회로 구성
![Arduino Design](https://raw.githubusercontent.com/PKNU-IOT3/bustop_Arduino/main/images/arduino_design.png)

## 로직
- DB에 저장된 정보 실시간 반영
- 승객 탑승 시 DB에 반영(탑승대기인원 감소 및 현재 탑승인원 증가 / 터치 센서 사용)
- 버스의 정차 여부 및 인원 변화에 따라 LED / 부저로 사용자에게 알림

## 참고 자료
- MySQL 연결 헤더<br>
 :link: [MySQL](https://github.com/ChuckBell/MySQL_Connector_Arduino)<br>
- wifi / MySQL 연결<br>
 :link: [wifi](https://nan-sso-gong.tistory.com/9?category=958220)<br>
 :link: [MySQL](https://nan-sso-gong.tistory.com/10?category=958220)<br>


## 22.06.30 소스코드 구현
- ESP8266 모듈을 이용한 wifi 연결
![DB Connection](https://raw.githubusercontent.com/PKNU-IOT3/bustop_Arduino/main/images/DBconnection.png)

- MySQL 연결
- DB 정보 가져오기(SELECT)
```c
MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  // DB내용 가져오기
  sprintf(query, SELECT_SQL); 
  cur_mem->execute(query);
  column_names *cols = cur_mem->get_columns();
  
  // DB데이터 변수에 저장(bus_cnt , bus_NowIn)
  row_values *row = NULL;
  
  do {
    row = cur_mem->get_next_row();
    if (row != NULL) {
      for (int f = 0; f < cols->num_fields; f++) {
        Serial.print(row->values[f]);
        if (f < cols->num_fields-1) {
          Serial.print(',');
        }
      }
      Serial.println();
    }
  } while (row != NULL);
```
![DBSelect01](https://raw.githubusercontent.com/PKNU-IOT3/bustop_Arduino/main/images/DBselect01.png)


## 22.07.03 프로젝트 수정
- DB 업데이트(UPDATE)
```c
if (state == HIGH) {                        // state의 값이 HIGH라면 (터치센서 O)
    if(bus_cnt > 0){
      buzz();                                 // 부저
      bus_cnt--;                              // 대기인원 감소

      // bus_cnt UPDATE
      sprintf(query, UPDATE_SQL,bus_cnt); 
      cur_mem->execute(query);
    }
  }
```
- SELECT 항목 수정
```c
MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  // DB내용 가져오기
  sprintf(query, SELECT_SQL); 
  cur_mem->execute(query);
  column_names *cols = cur_mem->get_columns();

  // 터치센서 값 읽어오기
  state = digitalRead(13);
  
  // DB데이터 변수에 저장(bus_cnt , bus_NowIn)
  row_values *row = NULL;
  do {
    row = cur_mem->get_next_row();
    if (row != NULL) {
      for (int f = 0; f < cols->num_fields; f++) {
        if(f == 0)
        {
          bus_cnt = atoi(row->values[f]);
          Serial.println(bus_cnt);
        }
        else if(f == 1)
        {
          now_cnt = atoi(row->values[f]);
          Serial.println(now_cnt);
        }
      }
    }
  } while (row != NULL);
```
![DBSelect02](https://raw.githubusercontent.com/PKNU-IOT3/bustop_Arduino/main/images/DBselect02.png)

- RGB LED 연결 및 상황에 따른 제어 추가
  - nodeMCU GPIO 핀 사용 문제 발생
  - ![arduinoPin](https://raw.githubusercontent.com/PKNU-IOT3/bustop_Arduino/main/images/arduino_pin.png)
  - 다음과 같은 구성에서 GPIO4/GPIO5/GPIO12/GPIO13/GPIO14 5개의 PIN만 범용으로 사용 가능하여 해당 PIN 만 사용하도록 수정하여 해결
```c
if (bus_cnt > 0)                            // 탑승인원 0명 이상인경우 (승객 탑승o)
  {
    digitalWrite(4, HIGH);                    // RED
    digitalWrite(5, HIGH);
    digitalWrite(12, LOW);
  }
  else                                        // 탑승인원 0명 (승객 탑승x)
  {
    digitalWrite(4, LOW);                     // BLUE
    digitalWrite(5, HIGH);
    digitalWrite(12, HIGH);
  }
```

## 22.07.04 프로젝트 수정
- 부저 연결
- DB UPDATE(bus_NowIn)
```c
if (state == HIGH) {                        // state의 값이 HIGH라면 (터치센서 O)
    if(bus_cnt > 0){
      buzz();                                 // 부저
      bus_cnt--;                              // 대기인원 감소
      now_cnt++;                              // 현재 탑승인원 증가 
      // bus_cnt UPDATE
      sprintf(query, UPDATE_SQL,bus_cnt); 
      cur_mem->execute(query);
      // now_cnt UPDATE
      sprintf(query, UPDATE_NOW_SQL,now_cnt); 
      cur_mem->execute(query);
    }
  }
```
![DBupdate](https://raw.githubusercontent.com/PKNU-IOT3/bustop_Arduino/main/images/DBupdate.png)



## 22.07.25 프로젝트 수정
- DB 연결부 loop문으로 이동
  - setup문에 있을경우 연결 실패시 발생하는 예외처리
    
## 23.07.28 버스 모형에 nodeMCU 부착 및 동작확인
- 버스 제작 및 동작확인
  - 터치 센서를 누르면 탑승 대기 인원 감소 /  현재 탑승 인원 증가
  - 탑승 대기 인원 0명 될 경우 LED 파란색으로 변경
  - 별도의 전원 연결시 3대 모두 각각의 DB정보를 받아 정상작동 확인
  - DB 실시간 변경 확인

![구현화면](https://github.com/PKNU-IOT3/bustop_Arduino/blob/main/images/Bus_Arduino.gif)
