#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

// Bao gồm các thư viện cần thiết
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ESP8266Ping.h>

//#define DEBUG
#ifdef DEBUG
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
//#define ENABLE_DEBUG_PING
#endif

#define BLYNK_NO_BUILTIN  // Vô hiệu hóa các thao tác analog và digital tích hợp sẵn.
//#define BLYNK_NO_INFO  // Vô hiệu hóa việc cung cấp thông tin về thiết bị cho server (tiết kiệm thời gian).

// Định nghĩa màu sắc cho Blynk
#define BLYNK_GREEN "#23C48E"
#define BLYNK_BLUE "#04C0F8"
#define BLYNK_YELLOW "#ED9D00"
#define BLYNK_RED "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"
#define LED_BUILTIN 2  // Định nghĩa chân LED tích hợp sẵn

// Cấu hình WiFi
const char auth[] = "";
const char ssid[] = "";
const char pass[] = "";

const IPAddress ip(192, 168, 0, 123);  // Địa chỉ IP tĩnh của thiết bị
const IPAddress gateway(192, 168, 0, 1);  // Địa chỉ gateway
const IPAddress bcastAddr(192, 168, 0, 255);  // Địa chỉ broadcast
const IPAddress subnet(255, 255, 255, 0);  // Subnet mask
const IPAddress dns(1, 1, 1, 1);  // Địa chỉ DNS

// Cấu hình thiết bị WOL
const IPAddress device_ip(192, 168, 0, 234);  // Địa chỉ IP của server
byte macAddr[6] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };  // Địa chỉ MAC của thiết bị

const char device_name[] = "";  // Tên thiết bị
const uint16_t boot_time = 10;  // Thời gian khởi động ước lượng (1 số tương đương khoảng 2 giây)

// WOL
#define MAGIC_PACKET_LENGTH 102
#define PORT_WAKEONLAN 9
byte magicPacket[MAGIC_PACKET_LENGTH];  // Gói tin magic packet để đánh thức thiết bị
unsigned int localPort = 9;  // Cổng UDP

WiFiUDP udp;  // Đối tượng UDP để gửi gói tin

// Định nghĩa các chân (pins)
#define STATE_PIN V0
#define BUTTON_PIN V1
#define PING_TIME_PIN V2
#define RSSI_PIN V3

// Trạng thái của server WOL
struct WOLServerState {
  bool IsOnline;  // Trạng thái online/offline
  uint16_t boot_time;  // Thời gian khởi động
  bool boot_error;  // Lỗi khởi động
  uint16_t ping;  // Thời gian phản hồi ping
  uint32_t previousMillis;  // Thời gian trước đó
  uint32_t interval;  // Khoảng thời gian kiểm tra
};
WOLServerState currentState = { false, 0, false, 0, 0, 5000UL };  // Khởi tạo trạng thái ban đầu

// Timer
BlynkTimer timer;

void timerFunct() {
  // Ghi giá trị RSSI và thời gian ping lên Blynk
  Blynk.virtualWrite(RSSI_PIN, WiFi.RSSI());
  Blynk.virtualWrite(PING_TIME_PIN, currentState.ping);

  if (currentState.IsOnline) {
    // Nếu thiết bị đang online
    Blynk.setProperty(STATE_PIN, "color", BLYNK_GREEN);
    Blynk.virtualWrite(STATE_PIN, String(device_name) + " is Online");

    Blynk.setProperty(BUTTON_PIN, "color", BLYNK_DARK_BLUE);
    Blynk.setProperty(BUTTON_PIN, "offLabel", String(device_name) + " running...");
    Blynk.setProperty(BUTTON_PIN, "onLabel", String(device_name) + " running...");
  } else if (!currentState.IsOnline && currentState.boot_time > 0) {
    // Nếu thiết bị đang khởi động
    Blynk.setProperty(STATE_PIN, "color", BLYNK_BLUE);
    Blynk.virtualWrite(STATE_PIN, "Waiting for ping...");

    Blynk.setProperty(BUTTON_PIN, "color", BLYNK_YELLOW);
    Blynk.setProperty(BUTTON_PIN, "offLabel", currentState.boot_time);
    Blynk.setProperty(BUTTON_PIN, "onLabel", currentState.boot_time);
  } else if (!currentState.IsOnline && currentState.boot_time == 0 && currentState.boot_error) {
    // Nếu xảy ra lỗi khởi động
    Blynk.setProperty(STATE_PIN, "color", BLYNK_RED);
    Blynk.virtualWrite(STATE_PIN, "Oops! Something happened, Try It Again!");

    Blynk.setProperty(BUTTON_PIN, "color", BLYNK_YELLOW);
    Blynk.setProperty(BUTTON_PIN, "offLabel", "Try It Again");
    Blynk.setProperty(BUTTON_PIN, "onLabel", "Magic Packet has been sent");
  } else {
    // Nếu thiết bị offline
    Blynk.setProperty(STATE_PIN, "color", BLYNK_RED);
    Blynk.virtualWrite(STATE_PIN, String(device_name) + " is Offline");

    Blynk.setProperty(BUTTON_PIN, "color", BLYNK_BLUE);
    Blynk.setProperty(BUTTON_PIN, "offLabel", "Turn On");
    Blynk.setProperty(BUTTON_PIN, "onLabel", "Magic Packet has been sent");
  }
}

void connectWiFi() {
  // Kết nối WiFi
  WiFi.mode(WIFI_STA);
  WiFi.hostname("WOL server");
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(ssid, pass);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);

    count++;
    if (count > 20) {
      Serial.println("Error: Unable to connect to WiFi after multiple attempts.");
      delay(500);
      ESP.restart();
    } else {
      Serial.printf("Connecting to WiFi... Attempt %d\n", count);
      yield();
    }
  }

  Serial.println("WiFi connected successfully.");
  BLYNK_LOG("WiFi connected, IP: %s", WiFi.localIP().toString());
}

void connectBlynk() {
  // Kết nối Blynk
  Blynk.config(auth);
  Blynk.disconnect();

  int count = 0;
  while (Blynk.connect(10000) == false) {
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);

    count++;
    if (count > 20) {
      Serial.println("Error: Unable to connect to Blynk after multiple attempts.");
      delay(500);
      ESP.restart();
    } else {
      Serial.printf("Connecting to Blynk... Attempt %d\n", count);
      yield();
    }
  }

  Serial.println("Blynk connected successfully.");
  BLYNK_LOG("Blynk connected");
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);  // Bật Serial trong chế độ debug
#endif

  timer.setInterval(1000L, timerFunct);  // Đặt timer cho hàm timerFunct

  connectWiFi();  // Kết nối WiFi
  connectBlynk();  // Kết nối Blynk

  // Khởi động UDP và tạo magic packet
  if (udp.begin(localPort) == 1) {
    BLYNK_LOG("udp begin OK");
    buildMagicPacket();
  } else {
    delay(500);
    ESP.restart();
  }
}

void buildMagicPacket() {
  // Tạo magic packet để đánh thức thiết bị
  memset(magicPacket, 0xFF, 6);  // 6 byte đầu tiên là 0xFF

  // Thêm MAC address vào magic packet
  for (int i = 0; i < 16; i++) {
    int ofs = i * sizeof(macAddr) + 6;  // Vị trí bắt đầu của MAC address trong magic packet
    memcpy(&magicPacket[ofs], macAddr, sizeof(macAddr));  // Sao chép MAC address vào magic packet
  }
}

void loop() {
  // Kiểm tra kết nối WiFi
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    return;
  }

  // Kiểm tra kết nối với Blynk Cloud
  if (!Blynk.connected()) {
    connectBlynk();
    return;
  }

  uint32_t currentMillis = millis();
  if (currentMillis - currentState.previousMillis >= currentState.interval) {
    currentState.previousMillis = currentMillis;

    if (currentState.boot_time == 0) {
      currentState.interval = 5000UL;
    } else {
      currentState.boot_time--;
      if (currentState.boot_time == 0) {
        currentState.boot_error = true;
        Blynk.logEvent("server_error");
      }
    }

    if (Ping.ping(device_ip, 1)) {
      currentState.IsOnline = true;
      currentState.boot_error = false;
      currentState.boot_time = 0;
      currentState.ping = Ping.averageTime();
    } else {
      currentState.IsOnline = false;
      currentState.ping = 0;
    }
  }

  Blynk.run();  // Chạy Blynk
  timer.run();  // Chạy timer
}

// BOOT PC button handler của ứng dụng
BLYNK_WRITE(BUTTON_PIN) {
  if (!currentState.IsOnline && currentState.boot_time == 0) {
    BLYNK_LOG("AppButtonWakeOnLan: value=%d", param.asInt());
    udp.beginPacket(bcastAddr, PORT_WAKEONLAN);
    udp.write(magicPacket, MAGIC_PACKET_LENGTH);
    udp.endPacket();

    currentState.boot_time = boot_time;
    currentState.interval = 1000UL;
  }
  delay(2000);
  Blynk.virtualWrite(BUTTON_PIN, 0);
}
