/**
 ******************************************************************************
 * @file    TFT.h
 * @brief   TFT 屏幕一体化驱动（底层驱动 + 图形 + 文字 + 图片）
 * @note    所有对外接口统一使用 TFT_ 前缀
 ******************************************************************************
 */
#ifndef TFT_H
#define TFT_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "TFT_Config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ 基础类型 ============================*/
#ifndef __TYPEDEF_U8_U16__
#define __TYPEDEF_U8_U16__
typedef uint8_t  u8;
typedef uint16_t u16;
#endif

/*============================ 颜色定义 ============================*/
/* 由 RGB888 得到 RGB565 */
#define TFT_RGB565(r, g, b)  ((uint16_t)((((r) & 0xF8u) << 8) | (((g) & 0xFCu) << 3) | ((b) >> 3)))

#define TFT_RED         0xF800u
#define TFT_GREEN       0x07E0u
#define TFT_BLUE        0x001Fu
#define TFT_WHITE       0xFFFFu
#define TFT_BLACK       0x0000u
#define TFT_YELLOW      0xFFE0u
#define TFT_CYAN        0x07FFu
#define TFT_MAGENTA     0xF81Fu
#define TFT_GRAY0       0xEF7Du
#define TFT_GRAY1       0x8410u
#define TFT_GRAY2       0x4208u

/*====================== 底层驱动（SPI / 寄存器） ======================*/

/**
 * @brief  发送命令
 */
void TFT_WriteCommand(uint8_t cmd);

/**
 * @brief  发送 8 位数据
 */
void TFT_WriteData(uint8_t data);

/**
 * @brief  发送 16 位数据（高字节在前）
 */
void TFT_WriteData16(uint16_t data);

/**
 * @brief  写入一个寄存器（命令 + 单字节数据）
 */
void TFT_WriteRegister(uint8_t cmd, uint8_t data);

/**
 * @brief  硬件复位
 */
void TFT_Reset(void);

/**
 * @brief  设置显示窗口（写点、填充时自动换行）
 * @param  x0,y0 左上角坐标
 * @param  x1,y1 右下角坐标
 */
void TFT_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief  设置显示起始点（等价于 TFT_SetWindow(x,y,x,y)）
 */
void TFT_SetCursor(uint16_t x, uint16_t y);

/**
 * @brief  初始化 GPIO 与屏幕，使用前必须调用一次
 */
void TFT_Init(void);

/**
 * @brief  控制背光
 * @param  on 非 0 点亮，0 熄灭
 */
void TFT_SetBacklight(uint8_t on);

/*========================== 图形绘制 ==========================*/

/**
 * @brief  全屏填充
 */
void TFT_FillScreen(uint16_t color);

/**
 * @brief  画一个像素点（自动做边界裁剪）
 */
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief  填充矩形（自动做边界裁剪，底层一次开窗批量写入，速度快）
 */
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief  画矩形边框
 */
void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief  画任意直线（Bresenham 算法，水平/垂直线自动优化）
 */
void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/**
 * @brief  画圆（Bresenham 算法）
 */
void TFT_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

/**
 * @brief  画实心圆
 */
void TFT_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

/**
 * @brief  画凸起（立体）方框
 */
void TFT_DrawBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bc);

/**
 * @brief  画立体方框
 * @param  mode 0:凸起 1:凹下 2:白色高亮
 */
void TFT_Draw3DBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t mode);

/**
 * @brief  画凸起按钮（未按下状态）
 */
void TFT_DrawButtonRaised(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * @brief  画凹下按钮（按下状态）
 */
void TFT_DrawButtonSunken(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * @brief  按状态画按钮
 * @param  pressed 非 0 表示按下
 */
void TFT_DrawButton(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t pressed);

/**
 * @brief  RGB 与 BGR 颜色互换
 */
uint16_t TFT_BGR2RGB(uint16_t c);

/*========================== 文字显示 ==========================*/

/**
 * @brief  显示 16 点阵字符串（支持 ASCII 与 16x16 汉字，'\n' 换行）
 * @param  x,y 左上角坐标
 * @param  fc  前景色
 * @param  bc  背景色（fc == bc 时透明）
 */
void TFT_DrawString16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *str);

/**
 * @brief  显示 24 点阵字符串（ASCII 使用 16 点阵，汉字使用 24x24 点阵）
 */
void TFT_DrawString24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *str);

/**
 * @brief  显示 32 点阵数码管数字
 * @param  num 字符索引：0~9 为数字，10:'.' 11:':' 12:'%' 13:'°' 14:'-'
 */
void TFT_DrawNumber32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t num);

/*========================== 图片显示 ==========================*/

/**
 * @brief  显示 RGB565 图像（按行优先扫描）
 * @param  x,y    左上角坐标
 * @param  w,h    图像宽高（像素）
 * @param  pixels 像素数据首地址（不含图片头 8 字节）
 * @note   像素字节序由 TFT_IMAGE_LITTLE_ENDIAN 配置，默认低字节在前
 *         （Image2Lcd 输出格式），驱动会按高字节先发写入屏幕。
 */
void TFT_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *pixels);

/**
 * @brief  显示"带 8 字节图片头"的整张 RGB565 图片（Image2Lcd 取模格式）
 * @param  x,y   左上角坐标
 * @param  image 图片数组首地址，直接写数组名即可，不需要 +8
 * @note   自动从图片头读出宽高：byte1 必须为 0x10(16 位色)，
 *         byte2~3 为宽、byte4~5 为高（小端），像素数据在 +8 处。
 * @return 0 成功；-1 指针为空；-2 图片头不是 16 位色；-3 宽或高为 0
 */
int TFT_DrawImageHdr(uint16_t x, uint16_t y, const uint8_t *image);

/*========================== 帧率统计 ==========================*/

/**
 * @brief  初始化帧率统计
 * @param  period_ms 统计周期（毫秒），传 0 时使用默认值 500ms
 * @note   使用前调用一次即可，之后每帧调用 TFT_FpsTick()
 */
void TFT_FpsInit(uint16_t period_ms);

/**
 * @brief  复位统计（清零帧计数与当前帧率）
 */
void TFT_FpsReset(void);

/**
 * @brief  每显示完一帧调用一次，到统计周期后自动刷新帧率
 * @retval 1 = 本周期结束且帧率已更新；0 = 尚未到统计周期
 */
uint8_t TFT_FpsTick(void);

/**
 * @brief  获取当前帧率 ×10
 * @retval 例如返回 123 表示 12.3 FPS
 */
uint16_t TFT_FpsGet(void);

/**
 * @brief  在屏幕指定位置显示 "FPS:xxx.x"
 * @param  x,y 左上角坐标
 * @param  fc  前景色
 * @param  bc  背景色
 * @note   固定宽度输出，刷新时不会留下残影；
 *         内部会缓存上次的值与坐标，值未变化且坐标相同时不重绘，
 *         因此可以放心地每帧调用，额外开销极小。
 */
void TFT_FpsShow(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc);

/**
 * @brief  使 TFT_FpsShow 的缓存失效，强制下一次调用重绘
 * @note   屏幕内容被整体重画（例如调用了 TFT_FillScreen）之后必须调用一次，
 *         否则帧率数值没有变化时 TFT_FpsShow 会因缓存命中而跳过重绘。
 */
void TFT_FpsInvalidate(void);

#ifdef __cplusplus
}
#endif

#endif /* TFT_H */
