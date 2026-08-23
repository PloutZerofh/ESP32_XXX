# ESP32-2432S028R LVGL 工程

ESP-IDF 工程，面向 2.8" Cheap Yellow Display（ILI9341 + XPT2046 触摸）。上电后显示 LVGL 主界面，支持触摸按钮计数。

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

厂商 Arduino 配置备份（不参与编译）：`docs/User_Setup.h`

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

屏幕显示 Logo、标题、可点击按钮和计数标签；触摸按钮后计数递增。

## 工程结构

```
components/
  bsp/
    CMakeLists.txt
    src/           # lcd.c, touch.c
    include/       # lcd.h, touch.h, board_pins.h
  lvgl_port/
    CMakeLists.txt
    lv_port.c, lv_port_disp.c, lv_port_indev.c
    include/
  lvgl/            # LVGL 库
  lv_conf.h        # LVGL 配置
main/
  main.c           # 启动编排
  tasks/           # RTOS 任务（LVGL 调度）
  ui/
    screens/       # 页面
    assets/images/ # 图片资源
tools/             # 图片转 LVGL C 数组脚本
```

## 资源工具

```bat
python tools/img2lvgl.py logo.png main/ui/assets/images/img_logo.c img_logo
python tools/gen_demo_logo.py
```
