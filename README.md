# ESP8266_Blynk_WOL_Server

Ứng dụng này được viết cho máy tính cá nhân của tôi và chỉ hỗ trợ một thiết bị duy nhất! Nó yêu cầu một địa chỉ IP tĩnh cho máy chủ WOL (được cấu hình trong mã nguồn) và cho thiết bị (bạn cần cấu hình riêng).

Bạn cần một thiết bị ESP-8266/ESP-8285/NodeMCU ESP8266 (tôi sử dụng NodeMCU ESP8266), ứng dụng [Blynk](https://www.blynk.cc/getting-started/), và thiết bị được cấu hình cho Wake on LAN.
Để tải phần mềm, sử dụng [Arduino IDE](https://www.arduino.cc/en/main/software) với [ESP8266 plug-in](https://github.com/esp8266/Arduino) và thư viện [Ping Library](https://github.com/dancol90/ESP8266Ping) đã được cài đặt.

--------------------

### Các biến cần thay đổi:

```cpp
//Cấu hình WiFi
const char auth[] = "Blynk_AuthToken";
const char ssid[] = "WiFi_SSID";
const char pass[] = "WiFi_Password";

const IPAddress ip(192, 168, 0, 123);
const IPAddress gateway(192, 168, 0, 1);
const IPAddress bcastAddr(192, 168, 0, 255);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress dns(1, 1, 1, 1);

//Cấu hình thiết bị WOL
const IPAddress device_ip(192, 168, 0, 234);
byte macAddr[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

//Cấu hình thông báo
const char email[] = "example@example.com";
const char device_name[] = "NAS";
const uint16_t boot_time = 10; // Số để đếm ngược (Nó không đại diện cho giây, nhưng gần, xem các vấn đề đã biết!)
```

Tên biến|Giá trị mặc định|Bình luận
-----|----------|-------------------------
auth|Blynk_AuthToken|Mã thông báo Blynk (Nhận được sau khi dự án được tạo trong Blynk)
ssid|WiFi_SSID|Tên mạng WiFi của bạn
pass|WiFi_Password|Mật khẩu mạng WiFi của bạn
ip|192.168.0.123|Địa chỉ IP tĩnh cho máy chủ WOL
gateway|192.168.0.1|Địa chỉ IP Gateway của bạn
bcastAddr|192.168.0.255|Địa chỉ IP Broadcast của bạn
subnet|255.255.255.0|Địa chỉ Subnet Mask của bạn
dns|1.1.1.1|Địa chỉ IP của máy chủ DNS ưa thích của bạn
device_ip|192.168.0.234|Địa chỉ IP tĩnh của thiết bị cần bật (được sử dụng để ping)
macAddr|aa:bb:cc:dd:ee|Địa chỉ MAC của thiết bị cần bật (quan trọng cho gói tin magic packet)
device_name|NAS|Tên ngắn của thiết bị mà bạn muốn bật
boot_time|10|Thời gian tối đa để chờ thiết bị bật (được sử dụng cho thông báo, xem các vấn đề đã biết!)

--------------------
