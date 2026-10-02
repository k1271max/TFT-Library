/**
 ******************************************************************************
 * @file    TFT_Test.h
 * @brief   TFT 屏幕综合测试程序（一个函数跑完全部功能）
 ******************************************************************************
 */
#ifndef TFT_TEST_H
#define TFT_TEST_H

#include "TFT.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  运行 TFT 综合测试程序
 * @note   内部会自动调用 TFT_Init()，调用者只需在 main 中调用一次。
 *         测试内容：初始化 / 寄存器读写 / 背光 / 反显 / 全屏填充 /
 *         渐变 / 色块 / 灰阶 / 像素 / 直线 / 矩形 / 圆 / 实心圆 /
 *         立体框 / 3D 框 / 按钮 / 16 与 24 点阵文字 / 中文 /
 *         透明文字 / 32 点阵数字 / 图片显示 / 屏幕刷新率统计 / 动画。
 */
void TFT_TestAll(void);

#ifdef __cplusplus
}
#endif

#endif /* TFT_TEST_H */
