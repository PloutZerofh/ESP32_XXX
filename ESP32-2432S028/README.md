# ESP32-2432S028R 纯显示工程

已从 `D:\ESP32\build\2.8inch__ESP32-2432S028R` 对齐引脚，ESP-IDF 工程可直接编译烧录。上电后循环刷色验证屏幕。

## 硬件引脚

| 信号 | GPIO |
|------|------|
| SCLK | 14 |
| MOSI | 13 |
| MISO | 12 |
| CS | 15 |
| DC | 2 |
| RST | 板载复位（-1） |
| 背光 | 21（高电平点亮） |
| 分辨率 | 240×320 ILI9341 |

厂商 Arduino 配置备份：`docs/User_Setup.h`

## 编译 / 烧录

在 **CMD** 中：

```bat
cd /d D:\ESP32\ESP32-2432S028
export_idf.bat
idf.py build
idf.py -p COMx flash monitor
```

把 `COMx` 换成设备管理器里的串口。也可在 Cursor/VS Code 的 ESP-IDF 扩展里对本目录执行 Build / Flash。

## 预期现象

红 → 绿 → 蓝 → 白 → 黑 → 黄 → 青 → 品红，每秒切换。

## 工程文件

- `main/main.c`：ILI9341 初始化 + 刷色
- `main/board_pins.h`：板级引脚
- `sdkconfig.defaults`：ESP32 / 4MB Flash
- `export_idf.bat`：激活本机 ESP-IDF 6.0 工具链
