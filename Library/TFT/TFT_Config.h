/**
 ******************************************************************************
 * @file    TFT_Config.h
 * @brief   TFT 屏幕（ST7735S / 1.8" 128x160 / 软件 SPI）硬件与显示配置
 * @note    修改引脚或屏幕尺寸时只需修改本文件，无需改动驱动逻辑
 ******************************************************************************
 */
#ifndef TFT_CONFIG_H
#define TFT_CONFIG_H

/*============================ 显示分辨率 ============================*/
#define TFT_WIDTH               128U
#define TFT_HEIGHT              160U

/*====================== 显存窗口偏移（GRAM Offset） ==================
 * 大部分 1.8" ST7735 模块偏移为 0/0（本驱动初始化时写入 0x2A:0..127 / 0x2B:0..159）。
 * 若整屏图像出现固定的上下/左右偏移，可尝试改为 1/0、2/1 等。
 */
#define TFT_COL_OFFSET          0U
#define TFT_ROW_OFFSET          0U

/*============================ 图像数据字节序 ============================
 * 1 = 每个像素低字节在前（Image2Lcd 常见输出，本工程 gImage_k1271cn 即为此格式）
 * 0 = 每个像素高字节在前
 * 无论哪种格式，驱动都会按"高字节先发"写入屏幕。
 */
#define TFT_IMAGE_LITTLE_ENDIAN   1

/*============================== 引脚定义 ==============================
 * SCL = 时钟(SCK)，SDA = 数据(MOSI)，DC = 命令/数据选择(原 RS)，
 * CS = 片选，RST = 复位，BLK = 背光
 */
#define TFT_GPIO_PORT           GPIOB
#define TFT_PIN_SCL             GPIO_PIN_4
#define TFT_PIN_SDA             GPIO_PIN_5
#define TFT_PIN_RST             GPIO_PIN_6
#define TFT_PIN_DC              GPIO_PIN_7
#define TFT_PIN_CS              GPIO_PIN_8
#define TFT_PIN_BLK             GPIO_PIN_9

/*============================ 快速电平操作 ============================*/
#define TFT_SCL_HIGH()   (TFT_GPIO_PORT->BSRR = (uint32_t)TFT_PIN_SCL)
#define TFT_SCL_LOW()    (TFT_GPIO_PORT->BRR  = (uint32_t)TFT_PIN_SCL)
#define TFT_SDA_HIGH()   (TFT_GPIO_PORT->BSRR = (uint32_t)TFT_PIN_SDA)
#define TFT_SDA_LOW()    (TFT_GPIO_PORT->BRR  = (uint32_t)TFT_PIN_SDA)
#define TFT_RST_HIGH()   (TFT_GPIO_PORT->BSRR = (uint32_t)TFT_PIN_RST)
#define TFT_RST_LOW()    (TFT_GPIO_PORT->BRR  = (uint32_t)TFT_PIN_RST)
#define TFT_DC_HIGH()    (TFT_GPIO_PORT->BSRR = (uint32_t)TFT_PIN_DC)
#define TFT_DC_LOW()     (TFT_GPIO_PORT->BRR  = (uint32_t)TFT_PIN_DC)
#define TFT_CS_HIGH()    (TFT_GPIO_PORT->BSRR = (uint32_t)TFT_PIN_CS)
#define TFT_CS_LOW()     (TFT_GPIO_PORT->BRR  = (uint32_t)TFT_PIN_CS)
#define TFT_BLK_HIGH()   (TFT_GPIO_PORT->BSRR = (uint32_t)TFT_PIN_BLK)
#define TFT_BLK_LOW()    (TFT_GPIO_PORT->BRR  = (uint32_t)TFT_PIN_BLK)

#endif /* TFT_CONFIG_H */
