# TFT-Library

> STM32F1 + ST7735S 1.8" TFT（128×160）单文件驱动库 · 软件模拟 SPI · 纯 C99 · 无动态内存分配 · 可直接移植

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-STM32F1xx-03234B.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32f1-series.html)
[![Language](https://img.shields.io/badge/language-C99-555.svg)]()

---

## 特性一览

| 项目 | 说明 |
| --- | --- |
| 屏幕 | ST7735S，1.8 英寸，128×160，RGB565（65K 色） |
| 接口 | 4 线软件模拟 SPI（SCL / SDA / DC / CS）+ RST + BLK |
| 驱动层次 | 底层寄存器 ↔ 图形绘制 ↔ 文字 ↔ 图片 ↔ 帧率统计，全部 `TFT_` 前缀 |
| 中文字库 | 16×16 与 24×24 双点阵，**中文直接写在字符串里**，无需转义 |
| 字符编码 | UTF-8 / GBK 均可，但**全工程必须统一**（见 [编码要求](#编码要求重要)） |
| 字库存储 | 编译期常量数组，存于片内 Flash，零 RAM 占用 |
| 性能优化 | 软件 SPI 位操作走 BSRR/BRR 无分支写法；驱动源文件单独 `-Os` |
| 图片 | RGB565 位图，支持带 8 字节图片头（Image2Lcd 格式）自动解析 |
| 帧率统计 | 内置 FPS 计数与屏幕显示，可测「屏幕实际刷新率」 |

---

## 效果展示

> 将截图放到 `docs/images/` 目录下，或替换为你的图片链接。

![效果 1](docs/images/effect-1.webp)
![效果 2](docs/images/effect-2.webp)
![效果 3](docs/images/effect-3.webp)
![效果 4](docs/images/effect-4.webp)

---

## 硬件连接

所有引脚默认接在 **GPIOB**，定义集中在 `TFT_Config.h`：

| 屏幕引脚 | 含义 | 默认引脚 | 说明 |
| --- | --- | --- | --- |
| SCL | 时钟 SCK | `PB4` | 软件模拟，输出 |
| SDA | 数据 MOSI | `PB5` | 软件模拟，输出 |
| RST | 复位 | `PB6` | 低电平复位 |
| DC | 命令/数据选择 | `PB7` | 原名 RS，高 = 数据，低 = 命令 |
| CS | 片选 | `PB8` | 低有效 |
| BLK | 背光 | `PB9` | 高电平点亮 |
| VCC | 电源 | 3.3V | **不要接 5V** |
| GND | 地 | GND | — |

> **注意**：`TFT_Init()` 内部只使能 `GPIOB` 时钟。若改用其他端口，需同步修改 `TFT_Config.h` 里的 `TFT_GPIO_PORT` 与 `TFT_Init()` 中的 `__HAL_RCC_GPIOx_CLK_ENABLE()`。

---

## 目录结构

```text
Library/TFT/
├── TFT.h          对外接口声明（唯一需要 #include 的头文件）
├── TFT.c          驱动实现（底层 SPI / 图形 / 文字 / 图片 / 帧率）
├── TFT_Config.h   硬件与显示配置（引脚、分辨率、偏移、字节序）
├── Font.h         字库数据（ASC16 / 汉字16 / 汉字24 / 数码管32）
├── Picture.c      图片数据（RGB565 位图）
├── Picture.h      图片数据声明
├── TFT_Test.c     综合测试程序（10 页演示全部功能）
└── TFT_Test.h     测试程序声明

tools/
├── font-maker.html   汉字/字符取模工具（浏览器打开）
└── image-maker.html  图片取模工具（浏览器打开）
```

> `CMakeLists.txt` 的包含路径只加了 `Library/`，所以引用时应写 `#include "TFT/TFT.h"` 或 `#include "TFT.h"`（取决于你的包含路径设置）。

---

## 快速开始

### 1. 获取代码

```bash
git clone https://github.com/k1271max/TFT-Library.git
```

也可从 [百度网盘](https://pan.baidu.com/s/1Xw8Rr6qGIEXF2fFv-ihTrQ?pwd=zurd) 下载（提取码 `zurd`）。

### 2. 开发环境

- 安装 [VSCode](https://code.visualstudio.com/)
- 安装以下扩展（`DeepSeek V4 for Copilot Chat` 不用装）：

  ![扩展列表](docs/images/vscode-extensions.webp)

- 工具链：CMake ≥ 3.22、Ninja、`arm-none-eabi-gcc`（本工程验证于 `14.3.1+st.2`）

### 3. 把库加入工程

把 `Library/` 目录加入工程的包含路径，并把 `TFT.c`、`Picture.c`（可选）加入编译源文件。

### 4. 初始化并显示内容

```c
#include "TFT/TFT.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* 只需调用一次：内部完成 GPIO 初始化 + 硬件复位 + 寄存器配置 */
    TFT_Init();
    TFT_SetBacklight(1);

    /* 全屏填充 + 写字 */
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 8, TFT_WHITE, TFT_BLACK, "Hello, STM32!");
    TFT_DrawString16(4, 28, TFT_GREEN, TFT_BLACK, "你好，世界");

    while (1)
    {
    }
}
```

### 5. 一次性验证全部功能

```c
#include "TFT/TFT_Test.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    TFT_TestAll();          /* 内部会自动 TFT_Init() */
    while (1) { }
}
```

### 6. 编译

```bash
# 配置（Debug / Release 二选一）
cmake --preset Debug

# 编译
cmake --build build/Debug
```

构建产物在 `build/Debug/`：

| 文件 | 说明 |
| --- | --- |
| `TFT-Library.elf` | 带调试信息，用于仿真 / 调试 |
| `TFT-Library.hex` | Intel HEX，用于烧录 |
| `TFT-Library.bin` | 纯二进制镜像 |

在 VSCode 中用调试模式自动配置项目，配置完成后点击生成文件：

![生成文件](docs/images/build-output.webp)

### 7. 烧录

```bash
# 方式一：st-flash
st-flash write build/Debug/TFT-Library.bin 0x08000000

# 方式二：STM32CubeProgrammer
STM32_Programmer_CLI -c port=SWD -w build/Debug/TFT-Library.hex -v -rst

# 方式三：OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program build/Debug/TFT-Library.hex verify reset exit"
```

也可通过 VSCode 调试下载：

![VSCode 调试下载](docs/images/debug-download.webp)

Hex 文件在 `Build` 文件夹下，可直接用其他工具烧录。

---

## 配置项说明

全部集中在 `TFT_Config.h`，改硬件只需动这一个文件。

```c
/* 分辨率 */
#define TFT_WIDTH               128U
#define TFT_HEIGHT              160U

/* 显存窗口偏移：整屏图像出现固定偏移时调整（常见 0/0、1/0、2/1） */
#define TFT_COL_OFFSET          0U
#define TFT_ROW_OFFSET          0U

/* 图像数据字节序：1 = 低字节在前（Image2Lcd 默认），0 = 高字节在前 */
#define TFT_IMAGE_LITTLE_ENDIAN   1

/* 引脚 */
#define TFT_GPIO_PORT           GPIOB
#define TFT_PIN_SCL             GPIO_PIN_4
#define TFT_PIN_SDA             GPIO_PIN_5
#define TFT_PIN_RST             GPIO_PIN_6
#define TFT_PIN_DC              GPIO_PIN_7
#define TFT_PIN_CS              GPIO_PIN_8
#define TFT_PIN_BLK             GPIO_PIN_9
```

| 配置项 | 取值 | 何时需要修改 |
| --- | --- | --- |
| `TFT_COL_OFFSET` / `TFT_ROW_OFFSET` | 通常 `0` / `0` | 画面整体固定偏移、边缘有彩条时 |
| `TFT_IMAGE_LITTLE_ENDIAN` | `1` | 图片颜色异常（红蓝互换 / 花屏）时切换 |
| `TFT_PIN_*` | `GPIO_PIN_x` | 换板子或改接线时 |

---

## API 速查

共 **33** 个对外接口，全部以 `TFT_` 开头。

### 底层驱动（9 个）

| 函数 | 说明 |
| --- | --- |
| `void TFT_Init(void)` | 初始化 GPIO 与屏幕，**使用前必须调用一次** |
| `void TFT_Reset(void)` | 硬件复位；复位后必须重新 `TFT_Init()` |
| `void TFT_SetBacklight(uint8_t on)` | 背光开关，非 0 点亮 |
| `void TFT_WriteCommand(uint8_t cmd)` | 发送命令 |
| `void TFT_WriteData(uint8_t data)` | 发送 8 位数据 |
| `void TFT_WriteData16(uint16_t data)` | 发送 16 位数据（高字节在前） |
| `void TFT_WriteRegister(uint8_t cmd, uint8_t data)` | 命令 + 单字节数据 |
| `void TFT_SetWindow(x0, y0, x1, y1)` | 设置显存窗口（左上角、右下角），**不自动裁剪** |
| `void TFT_SetCursor(x, y)` | 等价于 `TFT_SetWindow(x, y, x, y)` |

> `TFT_SetWindow` / `TFT_SetCursor` 是裸接口，坐标越界不会被钳制，越界行为由屏幕控制器决定。高层绘图函数（`FillRect` / `DrawPixel` / `DrawImage`）**内部已做裁剪**，可放心使用。

### 图形绘制（13 个）

| 函数 | 说明 |
| --- | --- |
| `void TFT_FillScreen(uint16_t color)` | 全屏填充 |
| `void TFT_DrawPixel(x, y, color)` | 画点（自动裁剪） |
| `void TFT_FillRect(x, y, w, h, color)` | 填充矩形（一次开窗批量写入，很快） |
| `void TFT_DrawRect(x, y, w, h, color)` | 矩形边框 |
| `void TFT_DrawLine(x0, y0, x1, y1, color)` | 任意直线（Bresenham，水平 / 垂直自动优化） |
| `void TFT_DrawCircle(x0, y0, r, color)` | 圆环（Bresenham） |
| `void TFT_FillCircle(x0, y0, r, color)` | 实心圆 |
| `void TFT_DrawBox(x, y, w, h, bc)` | 凸起立体方框 |
| `void TFT_Draw3DBox(x, y, w, h, mode)` | 立体方框，`mode`：0 凸起 / 1 凹下 / 2 白色高亮 |
| `void TFT_DrawButtonRaised(x1, y1, x2, y2)` | 凸起按钮（未按下） |
| `void TFT_DrawButtonSunken(x1, y1, x2, y2)` | 凹下按钮（按下） |
| `void TFT_DrawButton(x1, y1, x2, y2, pressed)` | 按状态自动选择凸起 / 凹下 |
| `uint16_t TFT_BGR2RGB(uint16_t c)` | RGB ↔ BGR 颜色互换 |

> 按钮类接口用的是**左上角 + 右下角坐标**，不是宽高。

### 文字显示（3 个）

| 函数 | 说明 |
| --- | --- |
| `void TFT_DrawString16(x, y, fc, bc, str)` | 16 点阵；ASCII 8×16、汉字 16×16；支持 `'\n'` 换行 |
| `void TFT_DrawString24(x, y, fc, bc, str)` | 24 点阵；汉字 24×24、ASCII 仍用 8×16；支持 `'\n'` 换行 |
| `void TFT_DrawNumber32(x, y, fc, bc, num)` | 32 点阵数码管单字符 |

参数含义：

| 参数 | 说明 |
| --- | --- |
| `x, y` | 左上角坐标 |
| `fc` | 前景色 |
| `bc` | 背景色；**`fc == bc` 时透明**（不写背景，叠加在已有画面之上） |
| `str` | 字符串，中文直接写，无需转义 |
| `num` | `0~9` 为数字；`10` `.`、`11` `:`、`12` `%`、`13` `°`、`14` `-` |

字距与行高：

| 函数 | ASCII 字距 | 汉字字距 | `'\n'` 行高 |
| --- | --- | --- | --- |
| `TFT_DrawString16` | 8 | 16 | 16 |
| `TFT_DrawString24` | 8 | 24 | 24 |

> **注意**：文字函数不做边界裁剪。坐标或整行宽度超出屏幕时可能回卷，请自行保证显示区域在 `128×160` 之内。

### 图片显示（2 个）

| 函数 | 说明 |
| --- | --- |
| `void TFT_DrawImage(x, y, w, h, pixels)` | 显示纯像素数组（**不含**图片头） |
| `int TFT_DrawImageHdr(x, y, image)` | 显示带 8 字节图片头的整图，**直接传数组名** |

```c
/* 推荐：Image2Lcd 取模，数组自带图片头 */
TFT_DrawImageHdr(14, 22, gImage_k1271cn);

/* 纯像素数组，或只显示图片的局部区域 */
TFT_DrawImage(14, 22, 100, 100, gImage_k1271cn + 8);
```

`TFT_DrawImageHdr` 返回值：

| 返回值 | 含义 |
| --- | --- |
| `0` | 成功 |
| `-1` | 指针为空 |
| `-2` | 图片头不是 16 位色（`byte1 != 0x10`） |
| `-3` | 图片头中的宽或高为 0 |

### 帧率统计（6 个）

| 函数 | 说明 |
| --- | --- |
| `void TFT_FpsInit(uint16_t period_ms)` | 初始化，`period_ms` 为统计周期，传 0 用默认 500ms |
| `void TFT_FpsReset(void)` | 清零计数与当前帧率 |
| `uint8_t TFT_FpsTick(void)` | 每帧调用一次；返回 1 表示本周期结束且数值已更新 |
| `uint16_t TFT_FpsGet(void)` | 取当前帧率 **×10**（返回 `123` 即 12.3 FPS） |
| `void TFT_FpsShow(x, y, fc, bc)` | 在屏幕显示 `FPS:xx.x`，固定宽度、内部带缓存 |
| `void TFT_FpsInvalidate(void)` | 使 `TFT_FpsShow` 缓存失效，强制下次重绘 |

`TFT_FpsShow` 每帧调用开销极小（数值与坐标都没变时直接跳过），但注意：

> **调用了 `TFT_FillScreen()` 之后必须调用一次 `TFT_FpsInvalidate()`**，否则数值未变时会被缓存命中而跳过重绘，屏幕上的读数就消失了。

典型用法：

```c
TFT_FpsInit(500);
TFT_FpsReset();

while (1)
{
    /* ... 绘制这一帧 ... */

    TFT_FpsTick();
    TFT_FpsShow(4, 2, TFT_GREEN, TFT_BLACK);
}
```

---

## 颜色与宏

帧缓冲格式是 **RGB565**，一个像素 2 字节。

```c
/* 由 RGB888 生成 RGB565 */
#define TFT_RGB565(r, g, b)  \
    ((uint16_t)((((r) & 0xF8u) << 8) | (((g) & 0xFCu) << 3) | ((b) >> 3)))
```

预定义颜色：

| 宏 | 值 | 颜色 |
| --- | --- | --- |
| `TFT_RED` | `0xF800` | 红 |
| `TFT_GREEN` | `0x07E0` | 绿 |
| `TFT_BLUE` | `0x001F` | 蓝 |
| `TFT_WHITE` | `0xFFFF` | 白 |
| `TFT_BLACK` | `0x0000` | 黑 |
| `TFT_YELLOW` | `0xFFE0` | 黄 |
| `TFT_CYAN` | `0x07FF` | 青 |
| `TFT_MAGENTA` | `0xF81F` | 品红 |
| `TFT_GRAY0` | `0xEF7D` | 浅灰 |
| `TFT_GRAY1` | `0x8410` | 中灰 |
| `TFT_GRAY2` | `0x4208` | 深灰 |

如果发现屏幕上的**红蓝互换**，说明面板的像素顺序与驱动假设相反，用 `TFT_BGR2RGB()` 或修改初始化里的 MADCTL 即可。

---

## 文字与中文字库

### 字库结构

字库全部在 `Font.h`，编译期即写入 Flash，不占 RAM。

| 数组 | 点阵 | 字形数 | 单字字节 | 说明 |
| --- | --- | --- | --- | --- |
| `asc16[]` | 8×16 | 95 | 16 | ASCII `0x20`~`0x7E` |
| `hz16[]` | 16×16 | 目前 5 | 36 | 4 B 键 + 32 B 点阵 |
| `hz24[]` | 24×24 | 目前 4 | 76 | 4 B 键 + 72 B 点阵 |
| `sz32[]` | 24×32 | 15 | 128 | 数码管 `0-9 . : % ° -` |

汉字字模的结构体定义：

```c
typedef struct
{
    union { uint32_t Key; };    /* 单成员匿名联合体 */
    unsigned char Msk[32];      /* 16×16 = 16 行 × 2 字节 */
} TFT_HZ16;
```

数组末尾有一项哨兵 `{{0}, {0}}`，查找时靠 `Key < 0x80` 跳过。

### 为什么可以用 `'你'` 当索引

```c
const TFT_HZ16 hz16[] = {
    {{'你'}, {0x08,0x80, ...}},
    ...
};
```

两个关键点：

1. **多字节字符常量 = 源字节按大端打包**。
   GBK 的「你」是 `C4 E3`，则 `'你'` 的值为 `0xC4E3`。
   驱动里的 `tft_str_key()` 也把字符串开头的字节按大端打包，两边算法一致，所以能直接比较。
2. **单成员匿名联合体**是必需的。
   直接写 `{{'你'}, {...}}` 去初始化 `uint32_t Key` 会触发
   *"braces around scalar initializer"*，而这个警告**没有对应的 `-W` 开关可以关闭**，所以用匿名联合体绕过。

多字节字符常量会触发 `-Wmultichar`，已在 `Font.h` 中用 `#pragma GCC diagnostic push/ignored/pop` 就地屏蔽。

### 编码要求（重要）

字库匹配是**按字节**进行的，所以：

> **`Font.h` 与所有调用它的 `.c` 文件必须使用同一种编码。**

本工程统一使用 **GBK**。如果某个文件改成了 UTF-8，那么同一个汉字在字库里的键和字符串里的字节就不一样了，结果是**这个字查不到、不显示**。

如果你确实需要混用编码，就必须把字库里的 `'字'` 改成对应的多字节写法，或者统一转码。**推荐做法：全工程保持 GBK。**

### 新增一个汉字

以「啊」的 24×24 字模为例：

1. 浏览器打开 `tools/font-maker.html`（注意这个文件是 GBK 编码，页面已声明 `<meta charset="gbk">`）；
2. 勾选目标字号 **24×24**，在输入框里输入要取模的汉字（可多个）；
3. 调整参数：
   - **超采样**：默认 `3x`，保证细笔画不丢失（`1x` 时「一」这类横线容易被丢掉）
   - **阈值**：默认 `90`
   - **形态学**：`none` / `+1` / `+2` / `-1`，用于加粗或减细
   - 工具会对**笔画过密**（`transitions/row` 超阈值）或**墨量过少**的字给出警告
4. 确认预览图（可直接点击像素手工修改）；
5. 点「复制全部」，把形如 `{{'啊'}, {0x..,0x..,...}},` 的行粘贴到 `Font.h` 的对应数组里；
6. **务必粘贴在 `#pragma GCC diagnostic push` 与 `pop` 之间**，否则会有 `-Wmultichar` 警告；
7. 编译验证，可用测试程序的文字页查看效果。

> **经验**：16×16 能容纳的笔画是有限的。「啊」有 16 画，在 16×16 下必然糊成一团 —— 真实字库里这种字是设计师**手工简化**过的。笔画多的字请直接用 24×24。

### 缺字时的行为

| 函数 | 字库里没有这个字时 |
| --- | --- |
| `TFT_DrawString16` | 跳过该字（按 UTF-8/GBK 长度前进 1 个字位），不显示 |
| `TFT_DrawString24` | **自动退化为 16 点阵并居中显示**（偏移 `+4, +4`），字距仍按 24 前进 |

所以 `TFT_DrawString24` 即使字库不全也不会出现空洞，只是字形变小。

---

## 图片显示

### 数据格式

图片以 `const unsigned char` 数组形式存放在 `Picture.c`，`Picture.h` 里做 `extern` 声明。

带图片头时（Image2Lcd 格式）：

| 偏移 | 长度 | 含义 |
| --- | --- | --- |
| `0` | 1 | 扫描方式，`0x00` = 从左到右、从上到下 |
| `1` | 1 | 位深，`0x10` = 16 位色（RGB565） |
| `2~3` | 2 | 宽度，**小端** |
| `4~5` | 2 | 高度，**小端** |
| `6~7` | 2 | 保留（`0x01 0x1B`） |
| `8~` | `w*h*2` | RGB565 像素数据，行优先 |

本工程示例：`gImage_k1271cn[20008]` = 8 字节头 + 100×100×2 像素，头部为
`00 10 64 00 64 00 01 1B`（宽 100、高 100）。

### 字节序

Image2Lcd 默认输出**每像素低字节在前**，因此 `TFT_Config.h` 中：

```c
#define TFT_IMAGE_LITTLE_ENDIAN   1
```

无论哪种字节序，驱动都会按「高字节先发」写入屏幕。如果显示出来颜色是花的但轮廓可见，就是这里配反了。

### 命名一致性（易错点）

`Picture.c` 里的数组名和 `Picture.h` 里的 `extern` 声明**必须一致**：

```c
/* Picture.c */
const unsigned char gImage_k1271cn[20008] = { ... };

/* Picture.h */
extern const unsigned char gImage_k1271cn[20008];
```

换图时只改 `.c` 忘了改 `.h`，编译就会报
`'gImage_xxx' undeclared (first use in this function)`。

---

## 取模工具

两个纯前端工具，双击用浏览器打开即可，无需联网、无需安装。

### `tools/font-maker.html` —— 字符取模

- 支持字号：8 / 12 / 16 / 24 / 32
- 超采样 1~4 倍（默认 3x）、阈值、形态学（膨胀 / 腐蚀）
- 自动居中、点击像素手工编辑
- 输出格式：`{{'字'}, {0x.., ...}},`，可直接粘贴进 `Font.h`
- 内置质量预警：墨量过低（< 5%）、笔画过密

### `tools/image-maker.html` —— 图片取模

- 拖放 / 粘贴 / 选择图片
- 缩放方式：最近邻 / 双线性 / 高质量
- 输出模式：RGB565（对应 `TFT_DrawImage`）/ 单色 1 bit / 字模格式
- 可调亮度、对比度、反色、阈值、4×4 Bayer 抖动、自动居中
- 可选「加 8 字节图片头」，勾选后用 `TFT_DrawImageHdr()` 显示

> 两个 HTML 文件都用 **GBK** 编码保存并声明 `<meta charset="gbk">`。如果你用编辑器重存成 UTF-8，请同步修改页面里的 `charset` 声明，否则中文会乱码。

---

## 综合测试程序

调用 `TFT_TestAll()` 会依次播放 10 页演示，每页停留 `TFT_TEST_PAGE_DELAY`（默认 1000 ms）：

| 页 | 内容 | 覆盖的接口 |
| --- | --- | --- |
| 0 | 底层驱动 | `Init` / `Reset` / `WriteCommand` / `WriteData` / `WriteRegister` / `SetBacklight` |
| 1 | 开机动画 | `FillScreen` / `FillRect` / `RGB565` |
| 2 | 颜色 | 色块 / 灰阶 / `BGR2RGB` / `SetWindow` / `WriteData16` / `SetCursor` / `DrawPixel` |
| 3 | 图形 | `DrawLine` / `DrawCircle` / `FillCircle` / `DrawRect` / `FillRect` |
| 4 | 立体框与按钮 | `DrawBox` / `Draw3DBox` / `DrawButtonRaised` / `DrawButtonSunken` / `DrawButton` |
| 5 | 文字 | `DrawString16` / `DrawString24`（含中文、透明背景） |
| 6 | 数字 | `DrawNumber32` 全部 15 个字形 |
| 7 | 图片 | `DrawImageHdr` |
| 8 | 屏幕刷新率 | 10 秒整屏重画测速 + 条形图（满格 20 FPS） |
| 9 | 动画与结束 | 进度条 / 弹跳小球 / 背光闪烁 |

调整节奏只需改 `TFT_Test.c` 顶部的宏：

```c
#define TFT_TEST_PAGE_DELAY   1000u
```

### 关于动画不闪烁的做法

第 9 页的弹跳小球用的是**增量更新**：每帧只重画「旧球消失」与「新球新增」的月牙区域，而不是「擦掉整个球再画新球」。核心是辅助函数：

```c
static void tft_test_row_diff(int32_t a0, int32_t a1,
                              int32_t b0, int32_t b1,
                              uint16_t y, uint16_t color);
```

它在某一行上画出区间 `[a0,a1]` 中**不被 `[b0,b1]` 覆盖**的部分。

> **踩坑提醒**：绘制首帧时必须和增量擦除使用**同一套跨度表**。如果首帧用 `TFT_FillCircle`（Bresenham 跨度，略大几个像素）、擦除却用自算的 `hw[]` 表，小球初始位置就会残留几个像素。

---

## 性能说明

| 项目 | 说明 |
| --- | --- |
| SPI | **软件模拟**，每一位都要 GPIO 翻转；走 `BSRR`/`BRR` 无分支写法 |
| 单像素开销 | `-Os` 下位循环约 12 条指令，无栈操作 |
| 整屏刷新 | 128×160×2 = 40960 字节 ≈ 32.8 万次时钟翻转，是主要瓶颈 |
| 实测方法 | 测试程序第 8 页：10 秒内尽可能多次整屏 `FillScreen`，统计平均帧率 |

想进一步提升速度的方向（按收益排序）：

1. **改用硬件 SPI + DMA**：吞吐量可提升一个数量级，是根治方案；
2. 提高 SPI 时钟（软件模拟下可减小延时或直接去掉）；
3. 大幅区域刷新改用 `TFT_SetWindow` + 连续写像素，避免逐像素 `DrawPixel`；
4. 动画只做**局部增量更新**（见《关于动画不闪烁的做法》）。

### 优化选项说明

软件模拟 SPI 对速度极其敏感，`-O0` 下位操作循环会膨胀一倍以上。因此 `CMakeLists.txt` 里对**驱动源文件单独开启 `-Os`**：

```cmake
set_source_files_properties(${TFT_LIBRARY_SRC} PROPERTIES COMPILE_OPTIONS "-Os")
```

这样工程其余部分仍可用 `-O0` 单步调试，而屏幕刷新不受影响。

---

## 常见问题

### 屏幕全白或全黑

1. 确认 `TFT_SetBacklight(1)` 已调用（BLK 接高）；
2. 确认 `TFT_Init()` 被调用过，且 `Reset()` 之后**再次**调用了 `Init()`；
3. 检查 CS / DC / SCL / SDA 接线是否与 `TFT_Config.h` 一致；
4. 检查 `GPIOB` 时钟是否使能、屏幕供电是否为 3.3V。

### 颜色不对：红蓝互换

面板像素顺序与驱动假设相反。用 `TFT_BGR2RGB()` 转换，或修改初始化中的 MADCTL 寄存器。

### 显示图片时颜色是花的，但轮廓能看出来

`TFT_IMAGE_LITTLE_ENDIAN` 配反了，改成 `0` 试试。

### 整屏图像固定偏移、边缘有彩条

调整 `TFT_COL_OFFSET` / `TFT_ROW_OFFSET`，常见取值 `0/0`、`1/0`、`2/1`。

### 中文不显示（英文正常）

1. 该字**不在字库里** —— 用 `tools/font-maker.html` 生成后加入 `Font.h`；
2. **编码不一致** —— 确认 `Font.h` 和调用文件都是 GBK（见《编码要求》）；
3. 新增字模没有放在 `#pragma push/pop` 之间。

### 编译报 `'gImage_xxx' undeclared`

`Picture.h` 里的 `extern` 名字和 `Picture.c` 里的定义不一致，改成一样即可。

### 编译报 `braces around scalar initializer`

汉字字模的 `Key` 没有用单成员匿名联合体包裹，参考 `Font.h` 中 `TFT_HZ16` 的写法。

### 动作动画时有残影或闪烁

用增量更新替代「擦除 + 重画」，并确保首帧与擦除使用同一套跨度计算（见《关于动画不闪烁的做法》）。

### `TFT_FpsShow` 显示的数值消失了

在 `TFT_FillScreen()` 之后补一句 `TFT_FpsInvalidate()`，强制下一次重绘。

### 刷新太慢

见《性能说明》。软件 SPI 是根本瓶颈，建议改硬件 SPI + DMA。

---

## 附：接口命名约定

| 前缀 | 含义 |
| --- | --- |
| `TFT_Init` / `TFT_Reset` / `TFT_Set*` | 初始化与设置 |
| `TFT_Draw*` | 画线条类图形（描边） |
| `TFT_Fill*` | 填充类图形（实心） |
| `TFT_*Box` / `TFT_*Button` | 立体装饰元素 |
| `TFT_DrawString*` / `TFT_DrawNumber*` | 文字与数字 |
| `TFT_DrawImage*` | 图片 |
| `TFT_Fps*` | 帧率统计 |

---

## 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

---

## 贡献

欢迎提交 Issue 和 Pull Request。如果这个库对你有帮助，给个 Star ⭐ 支持一下。
