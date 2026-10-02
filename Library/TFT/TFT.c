/**
 ******************************************************************************
 * @file    TFT.c
 * @brief   ST7735S（1.8寸TFT显示屏 128x160）软件 SPI 驱动 + 图形/文字/图片 一体化实现
 * @note    
 * 作者：K1271
 * QQ：2395779533
 * 邮箱：k1271max@qq.com/gmail.com
 ******************************************************************************
 */

#include "TFT.h"
#include "Font.h"
#include <stddef.h>

/* 数码管字形数量（每个字形 32x4 = 128 字节） */
#define sz32_num   (sizeof(sz32) / 128u)

/*====================== 内部低层函数 ======================*/

/**
 * @brief 软件 SPI 发送一个字节（不控制 CS / DC）
 * @note  数据在 SCL 上升沿被屏幕锁存。
 *        BSRR 低 16 位为置位、高 16 位为复位，因此 SDA 的设置与清除
 *        可以合并成一次写入，并用移位代替分支，-O0 下也能保持紧凑。
 */
static void tft_spi_write_byte(uint8_t data)
{
    volatile uint32_t *const bsrr = &TFT_GPIO_PORT->BSRR;
    volatile uint32_t *const brr  = &TFT_GPIO_PORT->BRR;
    const uint32_t sda = (uint32_t)TFT_PIN_SDA;
    const uint32_t scl = (uint32_t)TFT_PIN_SCL;
    uint8_t i;

    for (i = 0; i < 8u; i++)
    {
        /* 取最高位：为 1 时写 sda（置位），为 0 时写 sda << 16（复位） */
        uint32_t sh = (1u - ((uint32_t)(data >> 7) & 1u)) << 4;

        *bsrr = (sda << sh);
        *brr  = scl;                    /* SCL = 0 */
        *bsrr = scl;                    /* SCL = 1 */

        data = (uint8_t)(data << 1);
    }
}

/**
 * @brief 连续写入 count 个相同颜色（调用前需先 TFT_SetWindow 开窗）
 */
static void tft_write_color_n(uint16_t color, uint32_t count)
{
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFFu);

    TFT_CS_LOW();
    TFT_DC_HIGH();
    while (count--)
    {
        tft_spi_write_byte(hi);
        tft_spi_write_byte(lo);
    }
    TFT_CS_HIGH();
}

/*====================== 底层驱动 ======================*/

void TFT_WriteCommand(uint8_t cmd)
{
    TFT_CS_LOW();
    TFT_DC_LOW();
    tft_spi_write_byte(cmd);
    TFT_CS_HIGH();
}

void TFT_WriteData(uint8_t data)
{
    TFT_CS_LOW();
    TFT_DC_HIGH();
    tft_spi_write_byte(data);
    TFT_CS_HIGH();
}

void TFT_WriteData16(uint16_t data)
{
    TFT_CS_LOW();
    TFT_DC_HIGH();
    tft_spi_write_byte((uint8_t)(data >> 8));
    tft_spi_write_byte((uint8_t)(data & 0xFFu));
    TFT_CS_HIGH();
}

void TFT_WriteRegister(uint8_t cmd, uint8_t data)
{
    TFT_WriteCommand(cmd);
    TFT_WriteData(data);
}

void TFT_Reset(void)
{
    TFT_RST_LOW();
    HAL_Delay(100);
    TFT_RST_HIGH();
    HAL_Delay(50);
}

void TFT_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    x0 += TFT_COL_OFFSET;
    x1 += TFT_COL_OFFSET;
    y0 += TFT_ROW_OFFSET;
    y1 += TFT_ROW_OFFSET;

    TFT_WriteCommand(0x2Au);           /* 列地址 */
    TFT_WriteData((uint8_t)(x0 >> 8));
    TFT_WriteData((uint8_t)(x0 & 0xFFu));
    TFT_WriteData((uint8_t)(x1 >> 8));
    TFT_WriteData((uint8_t)(x1 & 0xFFu));

    TFT_WriteCommand(0x2Bu);           /* 行地址 */
    TFT_WriteData((uint8_t)(y0 >> 8));
    TFT_WriteData((uint8_t)(y0 & 0xFFu));
    TFT_WriteData((uint8_t)(y1 >> 8));
    TFT_WriteData((uint8_t)(y1 & 0xFFu));

    TFT_WriteCommand(0x2Cu);           /* 开始写显存 */
}

void TFT_SetCursor(uint16_t x, uint16_t y)
{
    TFT_SetWindow(x, y, x, y);
}

void TFT_SetBacklight(uint8_t on)
{
    if (on) TFT_BLK_HIGH();
    else    TFT_BLK_LOW();
}

void TFT_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio.Pin   = TFT_PIN_SCL | TFT_PIN_SDA | TFT_PIN_RST |
                 TFT_PIN_DC  | TFT_PIN_CS  | TFT_PIN_BLK;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TFT_GPIO_PORT, &gpio);

    TFT_CS_HIGH();
    TFT_SCL_HIGH();
    TFT_SDA_HIGH();
    TFT_DC_HIGH();
    TFT_RST_HIGH();
    TFT_BLK_HIGH();

    TFT_Reset();

    TFT_WriteCommand(0x11);            /* 退出睡眠 */
    HAL_Delay(120);

    TFT_WriteCommand(0xB1);            /* 帧率控制 */
    TFT_WriteData(0x01);
    TFT_WriteData(0x2C);
    TFT_WriteData(0x2D);

    TFT_WriteCommand(0xB2);
    TFT_WriteData(0x01);
    TFT_WriteData(0x2C);
    TFT_WriteData(0x2D);

    TFT_WriteCommand(0xB3);
    TFT_WriteData(0x01);
    TFT_WriteData(0x2C);
    TFT_WriteData(0x2D);
    TFT_WriteData(0x01);
    TFT_WriteData(0x2C);
    TFT_WriteData(0x2D);

    TFT_WriteCommand(0xB4);            /* 列反转 */
    TFT_WriteData(0x07);

    /* 电源序列 */
    TFT_WriteCommand(0xC0);
    TFT_WriteData(0xA2);
    TFT_WriteData(0x02);
    TFT_WriteData(0x84);

    TFT_WriteCommand(0xC1);
    TFT_WriteData(0xC5);

    TFT_WriteCommand(0xC2);
    TFT_WriteData(0x0A);
    TFT_WriteData(0x00);

    TFT_WriteCommand(0xC3);
    TFT_WriteData(0x8A);
    TFT_WriteData(0x2A);

    TFT_WriteCommand(0xC4);
    TFT_WriteData(0x8A);
    TFT_WriteData(0xEE);

    TFT_WriteCommand(0xC5);
    TFT_WriteData(0x0E);

    TFT_WriteCommand(0x36);            /* 显存访问控制：MX=1 MY=1 */
    TFT_WriteData(0xC0);

    /* 伽马校正 */
    TFT_WriteCommand(0xE0);
    TFT_WriteData(0x0F);
    TFT_WriteData(0x1A);
    TFT_WriteData(0x0F);
    TFT_WriteData(0x18);
    TFT_WriteData(0x2F);
    TFT_WriteData(0x28);
    TFT_WriteData(0x20);
    TFT_WriteData(0x22);
    TFT_WriteData(0x1F);
    TFT_WriteData(0x1B);
    TFT_WriteData(0x23);
    TFT_WriteData(0x37);
    TFT_WriteData(0x00);
    TFT_WriteData(0x07);
    TFT_WriteData(0x02);
    TFT_WriteData(0x10);

    TFT_WriteCommand(0xE1);
    TFT_WriteData(0x0F);
    TFT_WriteData(0x1B);
    TFT_WriteData(0x0F);
    TFT_WriteData(0x17);
    TFT_WriteData(0x33);
    TFT_WriteData(0x2C);
    TFT_WriteData(0x29);
    TFT_WriteData(0x2E);
    TFT_WriteData(0x30);
    TFT_WriteData(0x30);
    TFT_WriteData(0x39);
    TFT_WriteData(0x3F);
    TFT_WriteData(0x00);
    TFT_WriteData(0x07);
    TFT_WriteData(0x03);
    TFT_WriteData(0x10);

    /* 默认全屏显示区域 */
    TFT_WriteCommand(0x2A);
    TFT_WriteData(0x00);
    TFT_WriteData(0x00);
    TFT_WriteData(0x00);
    TFT_WriteData((uint8_t)(TFT_WIDTH - 1));

    TFT_WriteCommand(0x2B);
    TFT_WriteData(0x00);
    TFT_WriteData(0x00);
    TFT_WriteData(0x00);
    TFT_WriteData((uint8_t)(TFT_HEIGHT - 1));

    TFT_WriteCommand(0xF0);
    TFT_WriteData(0x01);
    TFT_WriteCommand(0xF6);
    TFT_WriteData(0x00);

    TFT_WriteCommand(0x3A);            /* 像素格式：16bit */
    TFT_WriteData(0x05);

    TFT_WriteCommand(0x29);            /* 打开显示 */
}

/*====================== 图形绘制 ======================*/

void TFT_FillScreen(uint16_t color)
{
    TFT_FillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) return;

    TFT_SetWindow(x, y, x, y);
    TFT_WriteData16(color);
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (w == 0 || h == 0) return;
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) return;

    if ((uint32_t)x + w > TFT_WIDTH)  w = (uint16_t)(TFT_WIDTH  - x);
    if ((uint32_t)y + h > TFT_HEIGHT) h = (uint16_t)(TFT_HEIGHT - y);
    if (w == 0 || h == 0) return;

    TFT_SetWindow(x, y, (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));
    tft_write_color_n(color, (uint32_t)w * h);
}

void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (w == 0 || h == 0) return;

    TFT_DrawLine(x, y, (uint16_t)(x + w - 1), y, color);
    TFT_DrawLine(x, (uint16_t)(y + h - 1), (uint16_t)(x + w - 1), (uint16_t)(y + h - 1), color);
    TFT_DrawLine(x, y, x, (uint16_t)(y + h - 1), color);
    TFT_DrawLine((uint16_t)(x + w - 1), y, (uint16_t)(x + w - 1), (uint16_t)(y + h - 1), color);
}

void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int32_t dx = (int32_t)x1 - (int32_t)x0;
    int32_t dy = (int32_t)y1 - (int32_t)y0;
    int32_t sx, sy, err;

    /* 水平 / 垂直线使用批量填充，速度更快 */
    if (dy == 0)
    {
        if (x0 > x1) { uint16_t t = x0; x0 = x1; x1 = t; }
        TFT_FillRect(x0, y0, (uint16_t)(x1 - x0 + 1), 1, color);
        return;
    }
    if (dx == 0)
    {
        if (y0 > y1) { uint16_t t = y0; y0 = y1; y1 = t; }
        TFT_FillRect(x0, y0, 1, (uint16_t)(y1 - y0 + 1), color);
        return;
    }

    sx = (dx > 0) ? 1 : -1;
    sy = (dy > 0) ? 1 : -1;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    err = dx - dy;

    for (;;)
    {
        TFT_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;

        {
            int32_t e2 = err << 1;
            if (e2 > -dy) { err -= dy; x0 = (uint16_t)((int32_t)x0 + sx); }
            if (e2 <  dx) { err += dx; y0 = (uint16_t)((int32_t)y0 + sy); }
        }
    }
}

void TFT_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int32_t x = 0;
    int32_t y = (int32_t)r;
    int32_t d = 3 - 2 * (int32_t)r;

    while (x <= y)
    {
        TFT_DrawPixel((uint16_t)(x0 + x), (uint16_t)(y0 + y), color);
        TFT_DrawPixel((uint16_t)(x0 - x), (uint16_t)(y0 + y), color);
        TFT_DrawPixel((uint16_t)(x0 + x), (uint16_t)(y0 - y), color);
        TFT_DrawPixel((uint16_t)(x0 - x), (uint16_t)(y0 - y), color);
        TFT_DrawPixel((uint16_t)(x0 + y), (uint16_t)(y0 + x), color);
        TFT_DrawPixel((uint16_t)(x0 - y), (uint16_t)(y0 + x), color);
        TFT_DrawPixel((uint16_t)(x0 + y), (uint16_t)(y0 - x), color);
        TFT_DrawPixel((uint16_t)(x0 - y), (uint16_t)(y0 - x), color);

        if (d < 0)
        {
            d += 4 * x + 6;
        }
        else
        {
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void TFT_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int32_t x = 0;
    int32_t y = (int32_t)r;
    int32_t d = 3 - 2 * (int32_t)r;

    while (x <= y)
    {
        TFT_FillRect((uint16_t)(x0 - (uint16_t)y), (uint16_t)(y0 + (uint16_t)x), (uint16_t)(2 * y + 1), 1, color);
        TFT_FillRect((uint16_t)(x0 - (uint16_t)y), (uint16_t)(y0 - (uint16_t)x), (uint16_t)(2 * y + 1), 1, color);
        TFT_FillRect((uint16_t)(x0 - (uint16_t)x), (uint16_t)(y0 + (uint16_t)y), (uint16_t)(2 * x + 1), 1, color);
        TFT_FillRect((uint16_t)(x0 - (uint16_t)x), (uint16_t)(y0 - (uint16_t)y), (uint16_t)(2 * x + 1), 1, color);

        if (d < 0)
        {
            d += 4 * x + 6;
        }
        else
        {
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void TFT_DrawBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bc)
{
    TFT_DrawLine(x, y, (uint16_t)(x + w), y, TFT_GRAY0);
    TFT_DrawLine((uint16_t)(x + w - 1), (uint16_t)(y + 1), (uint16_t)(x + w - 1), (uint16_t)(y + 1 + h), TFT_GRAY2);
    TFT_DrawLine(x, (uint16_t)(y + h), (uint16_t)(x + w), (uint16_t)(y + h), TFT_GRAY2);
    TFT_DrawLine(x, y, x, (uint16_t)(y + h), TFT_GRAY0);
    TFT_DrawLine((uint16_t)(x + 1), (uint16_t)(y + 1), (uint16_t)(x + 1 + w - 2), (uint16_t)(y + 1 + h - 2), bc);
}

void TFT_Draw3DBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t mode)
{
    uint16_t lt;   /* 左上颜色 */
    uint16_t rb;   /* 右下颜色 */

    switch (mode)
    {
        case 1:  lt = TFT_GRAY2; rb = TFT_GRAY0; break;
        case 2:  lt = TFT_WHITE; rb = TFT_WHITE; break;
        default: lt = TFT_GRAY0; rb = TFT_GRAY2; break;
    }

    TFT_DrawLine(x, y, (uint16_t)(x + w), y, lt);
    TFT_DrawLine((uint16_t)(x + w - 1), (uint16_t)(y + 1), (uint16_t)(x + w - 1), (uint16_t)(y + 1 + h), rb);
    TFT_DrawLine(x, (uint16_t)(y + h), (uint16_t)(x + w), (uint16_t)(y + h), rb);
    TFT_DrawLine(x, y, x, (uint16_t)(y + h), lt);
}

void TFT_DrawButtonRaised(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    TFT_DrawLine(x1, y1, x2, y1, TFT_GRAY2);
    TFT_DrawLine((uint16_t)(x1 + 1), (uint16_t)(y1 + 1), x2, (uint16_t)(y1 + 1), TFT_GRAY1);
    TFT_DrawLine(x1, y1, x1, y2, TFT_GRAY2);
    TFT_DrawLine((uint16_t)(x1 + 1), (uint16_t)(y1 + 1), (uint16_t)(x1 + 1), y2, TFT_GRAY1);
    TFT_DrawLine(x1, y2, x2, y2, TFT_WHITE);
    TFT_DrawLine(x2, y1, x2, y2, TFT_WHITE);
}

void TFT_DrawButtonSunken(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    TFT_DrawLine(x1, y1, x2, y1, TFT_WHITE);
    TFT_DrawLine(x1, y1, x1, y2, TFT_WHITE);
    TFT_DrawLine((uint16_t)(x1 + 1), (uint16_t)(y2 - 1), x2, (uint16_t)(y2 - 1), TFT_GRAY1);
    TFT_DrawLine(x1, y2, x2, y2, TFT_GRAY2);
    TFT_DrawLine((uint16_t)(x2 - 1), (uint16_t)(y1 + 1), (uint16_t)(x2 - 1), y2, TFT_GRAY1);
    TFT_DrawLine(x2, y1, x2, y2, TFT_GRAY2);
}

void TFT_DrawButton(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t pressed)
{
    if (pressed) TFT_DrawButtonSunken(x1, y1, x2, y2);
    else         TFT_DrawButtonRaised(x1, y1, x2, y2);
}

uint16_t TFT_BGR2RGB(uint16_t c)
{
    uint16_t b = (uint16_t)((c >> 0)  & 0x1Fu);
    uint16_t g = (uint16_t)((c >> 5)  & 0x3Fu);
    uint16_t r = (uint16_t)((c >> 11) & 0x1Fu);

    return (uint16_t)((b << 11) | (g << 5) | r);
}

/*====================== 文字显示 ======================*/

/* 画一个 8x16 ASCII 字符 */
static void tft_draw_ascii16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t ch)
{
    uint8_t i, j;
    uint8_t idx = (ch > 32u) ? (uint8_t)(ch - 32u) : 0u;

    for (i = 0; i < 16; i++)
    {
        uint8_t bits = asc16[idx * 16u + i];
        for (j = 0; j < 8; j++)
        {
            if (bits & (0x80u >> j))        TFT_DrawPixel((uint16_t)(x + j), (uint16_t)(y + i), fc);
            else if (fc != bc)              TFT_DrawPixel((uint16_t)(x + j), (uint16_t)(y + i), bc);
        }
    }
}

/*==================== 字符解析（UTF-8 / GBK 自适应） ====================*/

/**
 * @brief 把字符串开头的 n 个字节按"大端"拼成一个整数
 * @note  多字节字符常量（如 '你'）的取值规则正好就是源字节按大端打包，
 *        所以用同样方式拼出字符串的键，两者可以直接比较。
 *        键的编码与源文件一致，因此 UTF-8 源和 GBK 源都能直接写中文。
 */
static uint32_t tft_str_key(const uint8_t *s, uint8_t n)
{
    uint32_t v = 0u;
    uint8_t  i;

    for (i = 0u; i < n; i++)
    {
        v = (v << 8) | (uint32_t)s[i];
    }
    return v;
}

/* 由字符常量的数值推算它占几个字节（1~4） */
static uint8_t tft_key_len(uint32_t key)
{
    if (key > 0xFFFFFFu) return 4u;
    if (key > 0xFFFFu)   return 3u;
    if (key > 0xFFu)     return 2u;
    return 1u;
}

/* 字库中没有的字符：按 UTF-8 长度跳过，非 UTF-8 则按 GBK 双字节 */
static uint8_t tft_skip_len(const uint8_t *s)
{
    uint8_t c = s[0];

    if (c < 0x80u) return 1u;
    if ((c & 0xE0u) == 0xC0u && (s[1] & 0xC0u) == 0x80u) return 2u;
    if ((c & 0xF0u) == 0xE0u && (s[1] & 0xC0u) == 0x80u &&
                                (s[2] & 0xC0u) == 0x80u) return 3u;
    if ((c & 0xF8u) == 0xF0u && (s[1] & 0xC0u) == 0x80u &&
                                (s[2] & 0xC0u) == 0x80u &&
                                (s[3] & 0xC0u) == 0x80u) return 4u;
    return 2u;      /* GBK 双字节 */
}

/* 查找 16 点阵字模；命中时通过 len 返回该字在字符串里占用的字节数 */
static const uint8_t *tft_find_hz16(const uint8_t *s, uint8_t *len)
{
    uint16_t k;

    for (k = 0; k < hz16_num; k++)
    {
        uint32_t key = hz16[k].Key;
        uint8_t  n;

        if (key < 0x80u) continue;              /* 跳过哨兵项 */
        n = tft_key_len(key);
        if (tft_str_key(s, n) == key)
        {
            *len = n;
            return (const uint8_t *)hz16[k].Msk;
        }
    }
    return NULL;
}

/* 查找 24 点阵字模；命中时通过 len 返回该字在字符串里占用的字节数 */
static const uint8_t *tft_find_hz24(const uint8_t *s, uint8_t *len)
{
    uint16_t k;

    for (k = 0; k < hz24_num; k++)
    {
        uint32_t key = hz24[k].Key;
        uint8_t  n;

        if (key < 0x80u) continue;              /* 跳过哨兵项 */
        n = tft_key_len(key);
        if (tft_str_key(s, n) == key)
        {
            *len = n;
            return (const uint8_t *)hz24[k].Msk;
        }
    }
    return NULL;
}

/* 用字模数据画一个 16x16 汉字 */
static void tft_blit_hz16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const uint8_t *msk)
{
    uint8_t i, j;
    uint8_t m;

    for (i = 0; i < 16; i++)
    {
        m = msk[i * 2];
        for (j = 0; j < 8; j++)
        {
            if (m & (0x80u >> j))   TFT_DrawPixel((uint16_t)(x + j), (uint16_t)(y + i), fc);
            else if (fc != bc)      TFT_DrawPixel((uint16_t)(x + j), (uint16_t)(y + i), bc);
        }

        m = msk[i * 2 + 1];
        for (j = 0; j < 8; j++)
        {
            if (m & (0x80u >> j))   TFT_DrawPixel((uint16_t)(x + j + 8), (uint16_t)(y + i), fc);
            else if (fc != bc)      TFT_DrawPixel((uint16_t)(x + j + 8), (uint16_t)(y + i), bc);
        }
    }
}

/* 用字模数据画一个 24x24 汉字 */
static void tft_blit_hz24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const uint8_t *msk)
{
    uint8_t i, j;
    uint8_t m;

    for (i = 0; i < 24; i++)
    {
        m = msk[i * 3];
        for (j = 0; j < 8; j++)
        {
            if (m & (0x80u >> j))   TFT_DrawPixel((uint16_t)(x + j), (uint16_t)(y + i), fc);
            else if (fc != bc)      TFT_DrawPixel((uint16_t)(x + j), (uint16_t)(y + i), bc);
        }

        m = msk[i * 3 + 1];
        for (j = 0; j < 8; j++)
        {
            if (m & (0x80u >> j))   TFT_DrawPixel((uint16_t)(x + j + 8), (uint16_t)(y + i), fc);
            else if (fc != bc)      TFT_DrawPixel((uint16_t)(x + j + 8), (uint16_t)(y + i), bc);
        }

        m = msk[i * 3 + 2];
        for (j = 0; j < 8; j++)
        {
            if (m & (0x80u >> j))   TFT_DrawPixel((uint16_t)(x + j + 16), (uint16_t)(y + i), fc);
            else if (fc != bc)      TFT_DrawPixel((uint16_t)(x + j + 16), (uint16_t)(y + i), bc);
        }
    }
}

void TFT_DrawString16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *str)
{
    uint16_t x0 = x;
    const uint8_t *s = (const uint8_t *)str;
    uint8_t len;
    const uint8_t *msk;

    if (s == NULL) return;

    while (*s)
    {
        if (*s == '\n') { x = x0; y = (uint16_t)(y + 16); s++; continue; }
        if (*s == '\r') { s++; continue; }

        if (*s < 0x80u)                         /* ASCII */
        {
            tft_draw_ascii16(x, y, fc, bc, *s);
            x = (uint16_t)(x + 8);
            s++;
        }
        else                                    /* 汉字 */
        {
            len = 0u;
            msk = tft_find_hz16(s, &len);       /* 按字符常量匹配，找不到返回 NULL */
            if (msk != NULL) tft_blit_hz16(x, y, fc, bc, msk);
            else             len = tft_skip_len(s);
            x = (uint16_t)(x + 16);
            s += len;
        }
    }
}

void TFT_DrawString24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *str)
{
    uint16_t x0 = x;
    const uint8_t *s = (const uint8_t *)str;
    uint8_t len;
    const uint8_t *msk;

    if (s == NULL) return;

    while (*s)
    {
        if (*s == '\n') { x = x0; y = (uint16_t)(y + 24); s++; continue; }
        if (*s == '\r') { s++; continue; }

        if (*s < 0x80u)                         /* ASCII */
        {
            tft_draw_ascii16(x, y, fc, bc, *s);
            x = (uint16_t)(x + 8);
            s++;
        }
        else                                    /* 汉字 */
        {
            len = 0u;
            msk = tft_find_hz24(s, &len);       /* 优先用 24 点阵字模 */

            if (msk != NULL)
            {
                tft_blit_hz24(x, y, fc, bc, msk);
            }
            else
            {
                /* 24 点阵字库暂无此字：退化为 16 点阵并居中，保证仍能显示 */
                len = 0u;
                msk = tft_find_hz16(s, &len);
                if (msk != NULL) tft_blit_hz16((uint16_t)(x + 4), (uint16_t)(y + 4), fc, bc, msk);
                else             len = tft_skip_len(s);
            }
            x = (uint16_t)(x + 24);             /* 字距始终按 24 点阵前进 */
            s += len;
        }
    }
}

void TFT_DrawNumber32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t num)
{
    uint8_t i, j, k, c;

    if (num >= sz32_num) return;

    for (i = 0; i < 32; i++)
    {
        for (j = 0; j < 4; j++)
        {
            c = sz32[num * 128u + i * 4u + j];
            for (k = 0; k < 8; k++)
            {
                if (c & (0x80u >> k))   TFT_DrawPixel((uint16_t)(x + j * 8 + k), (uint16_t)(y + i), fc);
                else if (fc != bc)      TFT_DrawPixel((uint16_t)(x + j * 8 + k), (uint16_t)(y + i), bc);
            }
        }
    }
}

/*====================== 图片显示 ======================*/

void TFT_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *pixels)
{
    uint32_t n;

    if (pixels == NULL) return;
    if (w == 0 || h == 0) return;
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) return;

    if ((uint32_t)x + w > TFT_WIDTH)  w = (uint16_t)(TFT_WIDTH  - x);
    if ((uint32_t)y + h > TFT_HEIGHT) h = (uint16_t)(TFT_HEIGHT - y);

    TFT_SetWindow(x, y, (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));

    TFT_CS_LOW();
    TFT_DC_HIGH();
    n = (uint32_t)w * h;
    while (n--)
    {
#if TFT_IMAGE_LITTLE_ENDIAN
        /* 数据为低字节在前：先取低字节，再取高字节，按"高字节先发"输出 */
        uint8_t lo = *pixels++;
        uint8_t hi = *pixels++;
        tft_spi_write_byte(hi);
        tft_spi_write_byte(lo);
#else
        tft_spi_write_byte(*pixels++);   /* 高字节 */
        tft_spi_write_byte(*pixels++);   /* 低字节 */
#endif
    }
    TFT_CS_HIGH();
}

int TFT_DrawImageHdr(uint16_t x, uint16_t y, const uint8_t *image)
{
    uint16_t w, h;

    if (image == NULL) return -1;

    /* 图片头 byte1：0x10 表示 16 位色（RGB565） */
    if (image[1] != 0x10u) return -2;

    /* byte2~3 = 宽，byte4~5 = 高，均为小端 */
    w = (uint16_t)((uint16_t)image[2] | ((uint16_t)image[3] << 8));
    h = (uint16_t)((uint16_t)image[4] | ((uint16_t)image[5] << 8));

    if (w == 0u || h == 0u) return -3;

    TFT_DrawImage(x, y, w, h, image + 8);
    return 0;
}

/*====================== 帧率统计 ======================*/

/* 帧率统计状态（文件内私有） */
static struct
{
    uint32_t start_tick;     /* 本统计周期起始时刻 */
    uint32_t frames;         /* 本周期已计帧数 */
    uint16_t period_ms;      /* 统计周期（毫秒） */
    uint16_t fps10;          /* 当前帧率 ×10 */
    uint16_t shown_fps10;    /* 已显示的帧率（缓存，用于避免重复绘制） */
    uint16_t shown_x;        /* 已显示的位置 */
    uint16_t shown_y;
} s_fps = { 0u, 0u, 500u, 0u, 0xFFFFu, 0xFFFFu, 0xFFFFu };

void TFT_FpsInit(uint16_t period_ms)
{
    s_fps.period_ms = (period_ms == 0u) ? 500u : period_ms;
    TFT_FpsReset();
}

void TFT_FpsInvalidate(void)
{
    s_fps.shown_fps10 = 0xFFFFu;
    s_fps.shown_x     = 0xFFFFu;
    s_fps.shown_y     = 0xFFFFu;
}

void TFT_FpsReset(void)
{
    s_fps.start_tick = HAL_GetTick();
    s_fps.frames     = 0u;
    s_fps.fps10      = 0u;
    TFT_FpsInvalidate();
}

uint8_t TFT_FpsTick(void)
{
    uint32_t now     = HAL_GetTick();
    uint32_t elapsed = now - s_fps.start_tick;

    s_fps.frames++;

    if (elapsed < s_fps.period_ms)
    {
        return 0u;
    }

    if (elapsed > 0u)
    {
        uint32_t f10 = (s_fps.frames * 10000u) / elapsed;
        s_fps.fps10 = (f10 > 9999u) ? 9999u : (uint16_t)f10;
    }

    s_fps.frames     = 0u;
    s_fps.start_tick = now;
    return 1u;
}

uint16_t TFT_FpsGet(void)
{
    return s_fps.fps10;
}

void TFT_FpsShow(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc)
{
    char     buf[10];
    uint16_t v;

    /* 值与坐标都没变则不重绘，保证每帧调用也几乎无开销 */
    if (s_fps.shown_fps10 == s_fps.fps10 &&
        s_fps.shown_x == x && s_fps.shown_y == y)
    {
        return;
    }
    s_fps.shown_fps10 = s_fps.fps10;
    s_fps.shown_x     = x;
    s_fps.shown_y     = y;

    v = s_fps.fps10;

    /* 手工格式化，避免引入 printf/sprintf 的体积开销 */
    buf[0] = 'F';
    buf[1] = 'P';
    buf[2] = 'S';
    buf[3] = ':';
    buf[4] = (char)('0' + (v / 1000u));
    buf[5] = (char)('0' + ((v / 100u) % 10u));
    buf[6] = (char)('0' + ((v / 10u) % 10u));
    buf[7] = '.';
    buf[8] = (char)('0' + (v % 10u));
    buf[9] = '\0';

    TFT_DrawString16(x, y, fc, bc, buf);
}
