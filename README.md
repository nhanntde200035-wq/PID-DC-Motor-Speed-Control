

https://github.com/user-attachments/assets/0e32c798-5bf1-407b-a93f-2ccd672a9ba5



https://github.com/user-attachments/assets/82a52e04-46e5-4977-9ddf-90037af53364



# Lab 2 — LPIT Timer và FTM PWM trên S32K144

Dự án môn **EBS211 — Hệ thống nhúng**, xây dựng cụm điều khiển động cơ DC và xi-nhan trên board **MaaZ EDU (S32K144)**. Chương trình sử dụng ngắt LPIT định kỳ để đọc nút nhấn, cập nhật duty cycle PWM và điều khiển hai LED xi-nhan.

## Chức năng

- Tạo PWM bằng **FTM2, kênh 4**, xuất tại **PTD13** để điều khiển chân ENA của driver L298N.
- Giữ **BTN_UP** để tăng duty cycle; giữ **BTN_DOWN** để giảm duty cycle.
- Nhấn **SW0** để bật/tắt xi-nhan trái; nhấn **SW1** để bật/tắt xi-nhan phải. Bật một bên sẽ tắt bên còn lại.
- Khởi động với duty cycle **0%**, IN1 ở mức HIGH và IN2 ở mức LOW.

Duty cycle là giá trị điều khiển PWM, không phải tốc độ quay đo được của động cơ. Phiên bản hiện tại giữ cố định chiều quay, chưa có nút đảo chiều hoặc phản hồi tốc độ.

## Phần cứng và môi trường

- Board MaaZ EDU sử dụng MCU S32K144.
- Driver cầu H L298N, động cơ DC và nguồn phù hợp với động cơ/driver.
- Hai nút nhấn ngoài cho BTN_UP và BTN_DOWN.
- Hai nút SW0/SW1 và hai LED theo sơ đồ chân bên dưới.
- S32 Design Studio hỗ trợ dự án S32K144 và bộ công cụ biên dịch ARM tương ứng.
- Bộ nạp/debug tương thích với board.

## Sơ đồ chân

| Tín hiệu | Chân MCU | Kết nối và hoạt động |
|---|---|---|
| PWM | PTD13, ALT2 | FTM2_CH4 → ENA của L298N |
| IN1 | PTD12 | Đầu ra GPIO, HIGH khi khởi động |
| IN2 | PTD11 | Đầu ra GPIO, LOW khi khởi động |
| BTN_UP | PTD10 | Đầu vào kéo lên nội, nhấn nối GND |
| BTN_DOWN | PTD9 | Đầu vào kéo lên nội, nhấn nối GND |
| SW0 | PTC12 | Xi-nhan trái, nhấn ở mức LOW |
| SW1 | PTC13 | Xi-nhan phải, nhấn ở mức LOW |
| LED trái | PTD15 | Active-LOW, mức LOW bật LED |
| LED phải | PTD16 | Active-LOW, mức LOW bật LED |

### Nối driver động cơ

1. Nối PTD13 vào ENA; tháo jumper ENA nếu module dùng jumper để giữ chân này luôn bật.
2. Nối PTD12 vào IN1 và PTD11 vào IN2.
3. Nối động cơ vào OUT1/OUT2.
4. Cấp nguồn motor và nguồn logic theo hướng dẫn của module L298N đang dùng. Kiểm tra chức năng jumper nguồn 5V của module trước khi cấp nguồn logic ngoài.
5. Nối chung GND của board, driver và nguồn motor.

## Thông số trong mã nguồn

| Thông số | Giá trị |
|---|---|
| Clock ngoại vi được sử dụng | FIRC chia ra 48 MHz |
| LPIT | Kênh 0, ngắt mỗi 10 ms |
| LPIT TVAL | 479999 |
| PWM | Edge-aligned, FTM2_CH4 |
| Tần số PWM theo cấu hình | 20 kHz |
| Chu kỳ PWM | 50 µs |
| Prescaler FTM | Chia 1 |
| FTM MOD | 2399 |
| Dải duty cycle | 0–100% |
| Bước tăng/giảm | 1 điểm phần trăm mỗi 20 ms khi giữ nút |
| Thời gian tăng từ 0% lên 100% | Khoảng 2 giây khi giữ BTN_UP |
| Khoảng đổi trạng thái LED | 400 ms |
| Chu kỳ sáng/tắt đầy đủ | 800 ms, tương đương khoảng 1,25 Hz |

Các giá trị thời gian và tần số trên được suy ra từ cấu hình mã nguồn, chưa phải kết quả đo phần cứng.

## Nguyên lý hoạt động

Sau khi cấu hình các ngõ ra FIRCDIV1/FIRCDIV2, chương trình khởi tạo GPIO, PWM và LPIT, sau đó bật ngắt LPIT. Vòng lặp chính rỗng; việc điều khiển được thực hiện trong `LPIT0_Ch0_IRQHandler()`.

Mỗi lần ngắt 10 ms, chương trình:

1. Xóa cờ ngắt và đọc trạng thái bốn nút nhấn.
2. Phát hiện chuyển mức HIGH → LOW của SW0/SW1 để bật/tắt xi-nhan tương ứng.
3. Sau mỗi hai lần ngắt, tăng hoặc giảm duty cycle một điểm phần trăm nếu nút tốc độ đang được giữ.
4. Cập nhật giá trị PWM theo `g_speed`.
5. Sau mỗi 40 lần ngắt, đảo trạng thái nhấp nháy và cập nhật hai LED.

Nếu giữ đồng thời BTN_UP và BTN_DOWN, chương trình ưu tiên BTN_UP. Khi thả nút tốc độ, duty cycle giữ nguyên. SW0/SW1 dùng phát hiện cạnh với chu kỳ lấy mẫu 10 ms; chưa có bộ lọc chống dội riêng yêu cầu tín hiệu ổn định qua nhiều mẫu.

## Cấu trúc dự án

```text
lab2/
├── src/
│   ├── main.c             # GPIO, xử lý nút, xi-nhan và ISR LPIT
│   ├── lpit.c             # Khởi tạo LPIT và bật ngắt NVIC
│   └── ftm_pwm.c          # Khởi tạo FTM2 và cập nhật duty cycle
├── include/
│   ├── lpit.h             # Clock và giá trị nạp LPIT 10 ms
│   ├── ftm_pwm.h          # Khai báo hàm điều khiển PWM
│   └── ...                # Header thiết bị và hệ thống
├── Project_Settings/      # Startup và linker của dự án
├── .settings/             # Thiết lập IDE
├── .project               # Mô tả dự án Eclipse/S32DS
├── .cproject              # Cấu hình biên dịch
└── README.md
```

Thư mục `Debug_FLASH/` chứa sản phẩm biên dịch và có thể được tạo lại khi build.

## Mở và chạy dự án

1. Tải hoặc clone repository về máy.
2. Mở S32 Design Studio, chọn **File → Import → General → Existing Projects into Workspace**.
3. Chọn thư mục chứa `.project` và `.cproject`, sau đó import dự án `lab2`.
4. Chọn cấu hình **Debug_FLASH** và thực hiện **Build Project**. Nếu IDE báo thiếu toolchain, kiểm tra môi trường S32DS và đường dẫn công cụ trong thiết lập dự án.
5. Nối board với bộ nạp, kiểm tra dây và nguồn cho driver động cơ.
6. Tạo hoặc chọn cấu hình debug phù hợp với S32K144 và bộ nạp đang dùng, nạp chương trình rồi chạy.

## Kiểm tra chức năng

| Thao tác | Hành vi mong đợi theo code |
|---|---|
| Khởi động | Duty cycle 0%, hai LED xi-nhan tắt |
| Giữ BTN_UP | Duty cycle tăng dần đến 100% |
| Giữ BTN_DOWN | Duty cycle giảm dần đến 0% |
| Thả BTN_UP/BTN_DOWN | Giữ duty cycle hiện tại |
| Nhấn SW0 | Bật/tắt xi-nhan trái, tắt xi-nhan phải |
| Nhấn SW1 | Bật/tắt xi-nhan phải, tắt xi-nhan trái |
| Đo tại PTD13 | PWM dự kiến 20 kHz, duty cycle theo `g_speed` |

Có thể theo dõi `g_speed`, `g_left_on`, `g_right_on` và `g_blink` trong debugger. Để xác nhận đầu ra thực tế, đo PWM tại PTD13 và kiểm tra hoạt động của driver/động cơ trên phần cứng.

## Thay đổi tham số

- **Tần số PWM:** sửa `MOTOR_MOD` trong `src/ftm_pwm.c` theo `f_PWM = 48000000 / (MOD + 1)` khi giữ prescaler chia 1.
- **Tốc độ tăng/giảm duty:** sửa `RAMP_DIV` trong `src/main.c`. Khoảng cập nhật bằng `RAMP_DIV × 10 ms` với cấu hình LPIT hiện tại.
- **Nhịp xi-nhan:** sửa `BLINK_TICKS` trong `src/main.c`. Khoảng đổi trạng thái bằng `BLINK_TICKS × 10 ms`.
- **Chu kỳ ngắt:** sửa `LPIT_10MS_TVAL` trong `include/lpit.h`; đồng thời tính lại thời gian tăng/giảm duty và nhấp nháy.
