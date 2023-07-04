#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <ESP8266WiFi.h>
#include <stdio.h>

// wifi 연결셋팅부
const char* ssid = "iotzone02";
const char* password = "1234567890";

//sql 연결셋팅부
IPAddress server_addr(210, 119, 12, 69); 
char user[] = "root";                    
char password_[] = "12345";

// wifi 및 DB 연결
WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

// SQL문 
char SELECT_SQL[] = "SELECT bus_cnt, bus_NowIn FROM bus.bus_table WHERE bus_num='100-1'";
char UPDATE_SQL[] = "UPDATE bus.bus_table SET bus_cnt = %d WHERE bus_num = '100-1'";
char UPDATE_NOW_SQL[] = "UPDATE bus.bus_table SET bus_NowIn = %d WHERE bus_num = '100-1'";
char query[128];

// 전역변수 지정
int bus_cnt = 0;
int now_cnt = 0;
int btn = 0;
int state = 0;
int flag = 1;

void setup() {
  Serial.begin(115200);
  // pinmode 지정
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(13, INPUT);
  
  // wifi 연결
  Wifi_connect();

  Serial.print("Connecting to SQL...  ");
  
  // DB연결
  if (conn.connect(server_addr, 3306, user, password_)) {
    Serial.println("OK.");
  }
  else {
    Serial.println("FAILED.");
  }
  cursor = new MySQL_Cursor(&conn);
}

void loop() {
  delay(500);
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

  // 사람 탑승
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

  // LED
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

  if (flag == 1){                             // 상태변화하는 경우 부저울림
  buzz();
  }
  delete cur_mem;                             // 초기화
}

// wifi 연결
void Wifi_connect() {
  Serial.println("---------------------------------------");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wifi connected!");
  Serial.println("\nConnected to network");
  Serial.print("My IP address is: ");
  Serial.println(WiFi.localIP());
}

// 부저
void buzz(){
  Serial.println("ON");
  tone(14, 277);
  delay(100);
  noTone(14);
  flag = 0;
}