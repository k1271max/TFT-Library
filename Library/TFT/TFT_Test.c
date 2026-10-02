/**
 ******************************************************************************
 * @file    TFT_Test.c
 * @brief   TFT 屏幕综合测试程序：一个函数 TFT_TestAll() 覆盖全部功能
 * @note    共 10 页（0~9），每页展示一类功能：
 *           0. 底层驱动（TFT_Init / TFT_Reset / 寄存器读写 / 背光 / 反显）
 *           1. 开机动画（竖向渐变 + 标题滑入）
 *           2. 颜色（色块 / 灰阶渐变 / RGB565 宏 / BGR 转换 / 底层直接写显存）
 *           3. 图形（星芒直线 / 圆 / 实心圆 / 矩形边框 / 实心矩形）
 *           4. 立体框与按钮（DrawBox / Draw3DBox 三种模式 / 凸起与凹下按钮）
 *           5. 文字（16 点阵 ASCII 与中文 / 24 点阵 / 透明背景）
 *           6. 数字（32 点阵数码管全部字形）
 *           7. 图片（RGB565 位图显示）
 *           8. 屏幕刷新率（整屏重画测速 / 实时显示 / 条形图）
 *           9. 动画与结束（进度条 / 弹跳小球 / 背光闪烁）
 *
 *         如需调整演示节奏，修改下面的延时宏即可。
 ******************************************************************************
 */

#include "TFT_Test.h"
#include "Picture.h"

/* 每页停留时间（毫秒） */
#define TFT_TEST_PAGE_DELAY   1000u

/* 说明：汉字可以直接写在字符串里，无需转义。
 * 驱动会自动识别源文件是 UTF-8 还是 GBK 编码（见 TFT.c 的 tft_decode_char） */

/**
 * @brief  在指定行绘制区间 [a0,a1] 中"不被 [b0,b1] 覆盖"的部分
 * @note   做移动动画时，用它只更新对象新增/消失的像素，
 *         避免把整个对象擦掉再重画导致的闪烁。
 */
static void tft_test_row_diff(int32_t a0, int32_t a1,
                              int32_t b0, int32_t b1,
                              uint16_t y, uint16_t color)
{
    if (a0 < 0) a0 = 0;
    if (a1 > (int32_t)TFT_WIDTH - 1) a1 = (int32_t)TFT_WIDTH - 1;
    if (a1 < a0 || y >= TFT_HEIGHT) return;

    if (b1 >= b0)
    {
        if (b0 > a0)
        {
            TFT_FillRect((uint16_t)a0, y, (uint16_t)(b0 - a0), 1, color);
        }
        if (a1 > b1)
        {
            TFT_FillRect((uint16_t)(b1 + 1), y, (uint16_t)(a1 - b1), 1, color);
        }
    }
    else
    {
        TFT_FillRect((uint16_t)a0, y, (uint16_t)(a1 - a0 + 1), 1, color);
    }
}

void TFT_TestAll(void)
{
    uint8_t  i;
    uint16_t x, y;

    /*==========================================================================
     * 0. 底层驱动测试
     *========================================================================*/
    TFT_Init();                             /* GPIO 初始化 + 复位 + 寄存器配置 */
    TFT_SetBacklight(1);

    /* TFT_Reset()：显式硬件复位（复位后必须重新初始化屏幕） */
    TFT_Reset();
    TFT_Init();

    /* TFT_WriteCommand / TFT_WriteData / TFT_WriteRegister */
    TFT_WriteRegister(0x3A, 0x05);          /* 重新确认 16bit 像素格式 */
    TFT_WriteCommand(0x3A);
    TFT_WriteData(0x05);

    /* TFT_WriteCommand：打开反显再关闭，直观验证命令通道 */
    TFT_WriteCommand(0x21);                 /* INVON  */
    HAL_Delay(250);
    TFT_WriteCommand(0x20);                 /* INVOFF */

    /*==========================================================================
     * 1. 开机动画：竖向青蓝渐变 + 标题条从中间展开
     *    （测试 TFT_FillScreen / TFT_FillRect / TFT_RGB565）
     *========================================================================*/
    for (y = 0; y < TFT_HEIGHT; y++)
    {
        uint8_t g = (uint8_t)((uint32_t)y * 255u / (TFT_HEIGHT - 1u));
        uint8_t b = (uint8_t)(255u - g);
        TFT_FillRect(0, y, TFT_WIDTH, 1, TFT_RGB565(0, g, b));
    }

    for (i = 0; i <= TFT_WIDTH / 2u; i += 4u)
    {
        TFT_FillRect((uint16_t)(64u - i), 58, (uint16_t)(2u * i), 22, TFT_BLACK);
        HAL_Delay(8);
    }
    TFT_DrawRect(0, 58, TFT_WIDTH, 22, TFT_WHITE);
    TFT_DrawString16(12, 62, TFT_WHITE, TFT_BLACK, "TFT  DEMO");
    TFT_DrawString16(28, 88, TFT_YELLOW, TFT_BLACK, "128x160");
    HAL_Delay(TFT_TEST_PAGE_DELAY);

    /*==========================================================================
     * 2. 颜色测试
     *========================================================================*/
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 2, TFT_WHITE, TFT_BLACK, "[2] COLOR");

    /* 8 个标准色块 */
    {
        const uint16_t bar[8] = {
            TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE,
            TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_GRAY1
        };
        for (i = 0; i < 8u; i++)
        {
            TFT_FillRect((uint16_t)(i * 16u), 20, 15, 26, bar[i]);
        }
    }

    /* 灰阶渐变（验证 TFT_RGB565 宏的逐级过渡） */
    for (x = 0; x < TFT_WIDTH; x++)
    {
        uint8_t v = (uint8_t)((uint32_t)x * 255u / (TFT_WIDTH - 1u));
        TFT_FillRect(x, 50, 1, 18, TFT_RGB565(v, v, v));
    }
    TFT_DrawString16(2, 70, TFT_GRAY1, TFT_BLACK, "RGB565 GRAY");

    /* TFT_BGR2RGB：同一颜色的正常与反序对比 */
    TFT_FillRect(6, 88, 40, 20, TFT_RED);
    TFT_FillRect(52, 88, 40, 20, TFT_BGR2RGB(TFT_RED));
    TFT_DrawString16(98, 90, TFT_WHITE, TFT_BLACK, "RGB");
    TFT_DrawString16(98, 106, TFT_WHITE, TFT_BLACK, "BGR");

    /* TFT_SetWindow + TFT_WriteData16：绕过高层接口直接写 4 个像素 */
    TFT_SetWindow(6, 116, 9, 116);
    TFT_WriteData16(TFT_RED);
    TFT_WriteData16(TFT_GREEN);
    TFT_WriteData16(TFT_BLUE);
    TFT_WriteData16(TFT_WHITE);

    /* TFT_SetCursor + TFT_WriteData16 */
    TFT_SetCursor(6, 124);
    TFT_WriteData16(TFT_MAGENTA);

    /* TFT_DrawPixel：在一个矩形内画棋盘格 */
    for (y = 0; y < 24; y++)
    {
        for (x = 0; x < 48; x++)
        {
            if (((x + y) & 1u) == 0u)
            {
                TFT_DrawPixel((uint16_t)(72 + x), (uint16_t)(120 + y), TFT_BLUE);
            }
        }
    }
    TFT_DrawString16(6, 144, TFT_WHITE, TFT_BLACK, "PIXEL OK");
    HAL_Delay(TFT_TEST_PAGE_DELAY);

    /*==========================================================================
     * 3. 图形测试：直线 / 圆 / 实心圆 / 矩形
     *========================================================================*/
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 2, TFT_WHITE, TFT_BLACK, "[3] SHAPES");

    /* 星芒：从中心向 16 个方向画直线（TFT_DrawLine） */
    {
        static const int16_t dir_x[16] = {
            1000, 924, 707, 383, 0, -383, -707, -924,
            -1000, -924, -707, -383, 0, 383, 707, 924
        };
        static const int16_t dir_y[16] = {
            0, 383, 707, 924, 1000, 924, 707, 383,
            0, -383, -707, -924, -1000, -924, -707, -383
        };
        for (i = 0; i < 16u; i++)
        {
            int16_t px = (int16_t)(64 + dir_x[i] * 50 / 1000);
            int16_t py = (int16_t)(76 + dir_y[i] * 52 / 1000);
            TFT_DrawLine(64, 76, (uint16_t)px, (uint16_t)py, TFT_BLUE);
        }
    }

    TFT_DrawCircle(64, 76, 40, TFT_YELLOW);     /* 圆环 */
    TFT_DrawCircle(64, 76, 24, TFT_GREEN);      /* 圆环 */
    TFT_FillCircle(64, 76, 10, TFT_RED);        /* 实心圆 */

    /* 矩形边框 / 实心矩形 / 斜线 */
    TFT_DrawRect(4, 138, 36, 18, TFT_CYAN);
    TFT_FillRect(46, 138, 36, 18, TFT_MAGENTA);
    TFT_DrawLine(90, 156, 124, 138, TFT_YELLOW);
    HAL_Delay(TFT_TEST_PAGE_DELAY);

    /*==========================================================================
     * 4. 立体框与按钮
     *========================================================================*/
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 2, TFT_WHITE, TFT_BLACK, "[4] BOX/BUTTON");

    TFT_DrawBox(4, 20, 56, 26, TFT_GRAY2);              /* TFT_DrawBox */
    TFT_DrawString16(20, 25, TFT_WHITE, TFT_GRAY2, "BOX");

    TFT_Draw3DBox(68, 20, 56, 26, 0);                   /* TFT_Draw3DBox */
    TFT_DrawString16(80, 25, TFT_WHITE, TFT_GRAY2, "3D-0");

    TFT_Draw3DBox(4, 52, 56, 26, 1);
    TFT_DrawString16(16, 57, TFT_WHITE, TFT_GRAY2, "3D-1");

    TFT_Draw3DBox(68, 52, 56, 26, 2);
    TFT_DrawString16(80, 57, TFT_WHITE, TFT_GRAY2, "3D-2");

    /* 凸起 / 凹下按钮 */
    TFT_DrawButtonRaised(4, 88, 60, 114);
    TFT_DrawString16(8, 93, TFT_WHITE, TFT_BLACK, "RAISED");

    TFT_DrawButtonSunken(68, 88, 124, 114);
    TFT_DrawString16(72, 93, TFT_WHITE, TFT_BLACK, "SUNKEN");

    /* TFT_DrawButton：按状态自动选择凸起 / 凹下 */
    TFT_DrawButton(4, 122, 60, 148, 0);
    TFT_DrawString16(20, 127, TFT_WHITE, TFT_BLACK, "OFF");

    TFT_DrawButton(68, 122, 124, 148, 1);
    TFT_DrawString16(88, 127, TFT_WHITE, TFT_BLACK, "ON");
    HAL_Delay(TFT_TEST_PAGE_DELAY);

    /*==========================================================================
     * 5. 文字测试
     *========================================================================*/
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 2, TFT_WHITE, TFT_BLACK, "[5] TEXT");

    /* TFT_DrawString16：ASCII */
    TFT_DrawString16(2, 20, TFT_WHITE, TFT_BLACK, "ASCII 8x16:");
    TFT_DrawString16(2, 38, TFT_YELLOW, TFT_BLACK, "0123456789+-*/");
    TFT_DrawString16(2, 56, TFT_CYAN, TFT_BLACK, "Hello, STM32!");

    /* TFT_DrawString16：中文直接写在字符串里即可，无需任何转义 */
    TFT_DrawString16(2, 74, TFT_GREEN, TFT_BLACK, "你好世界");

    /* 透明文字：fc == bc 时不写背景，文字叠加在色块上 */
    TFT_FillRect(0, 92, TFT_WIDTH, 18, TFT_MAGENTA);
    TFT_DrawString16(4, 93, TFT_WHITE, TFT_WHITE, "transparent");

    /* TFT_DrawString24：24 点阵汉字（hz24 字库），支持 '\n' 换行。
     * 字库中找不到的字会自动退化为 16 点阵居中显示 */
    TFT_DrawString24(4, 116, TFT_BLUE, TFT_BLACK, "你好世界");
    HAL_Delay(TFT_TEST_PAGE_DELAY);

    /*==========================================================================
     * 6. 数字测试：32 点阵数码管全部字形（0~9 与 . : % ° -）
     *========================================================================*/
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 2, TFT_WHITE, TFT_BLACK, "[6] NUMBER32");

    for (i = 0; i < 15u; i++)
    {
        uint16_t gx = (uint16_t)((i % 4u) * 32u);
        uint16_t gy = (uint16_t)(22u + (i / 4u) * 32u);
        TFT_DrawNumber32(gx, gy, (i < 10u) ? TFT_YELLOW : TFT_CYAN, TFT_BLACK, i);
    }
    HAL_Delay(TFT_TEST_PAGE_DELAY);

    /*==========================================================================
     * 7. 图片测试：RGB565 位图（跳过 8 字节图片头）
     *========================================================================*/
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 2, TFT_WHITE, TFT_BLACK, "[7] IMAGE");
    TFT_DrawRect(12, 20, 104, 104, TFT_BLUE);
    TFT_DrawImageHdr(14, 22, gImage_k1271cn);   /* 带图片头，直接传数组名 */
    TFT_DrawString16(0, 132, TFT_YELLOW, TFT_BLACK, "gImage_k1271cn");
    HAL_Delay(TFT_TEST_PAGE_DELAY);

    /*==========================================================================
     * 8. 屏幕刷新率（FPS）测试
     *    测的是"屏幕实际刷新率"：每循环一次就把整屏重画一遍，
     *    统计每秒能完成多少次整屏刷新，全程不加任何人为延时。
     *    TFT_FpsInit / TFT_FpsReset / TFT_FpsTick / TFT_FpsGet /
     *    TFT_FpsShow / TFT_FpsInvalidate
     *========================================================================*/
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 2, TFT_WHITE, TFT_BLACK, "[8] SCREEN FPS");

    TFT_FpsInit(500);                   /* 每 500ms 更新一次统计值 */
    TFT_FpsReset();

    {
        uint32_t t0    = HAL_GetTick();
        uint16_t frame = 0;

        while ((uint32_t)(HAL_GetTick() - t0) < 10000u)
        {
            /* 一帧 = 一次完整的整屏刷新 */
            TFT_FillScreen(TFT_BLACK);

            /* 底部细色带，颜色缓慢循环，用来目视确认屏幕确实在刷新 */
            TFT_FillRect(0, 156, TFT_WIDTH, 4,
                         TFT_RGB565(0, (uint8_t)((frame & 7u) * 32u),
                                       (uint8_t)(255u - (frame & 7u) * 32u)));

            TFT_FpsTick();
            TFT_FpsInvalidate();            /* 整屏刚被重画过，强制重绘读数 */
            TFT_FpsShow(28, 20, TFT_GREEN, TFT_BLACK);

            frame++;
        }
    }

    /* TFT_FpsGet()：用返回值画条形图（满格 = 20.0 FPS） */
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 4, TFT_GRAY1, TFT_BLACK, "REFRESH RATE");
    TFT_DrawString16(4, 24, TFT_GRAY1, TFT_BLACK, "avg over 10s");

    {
        uint16_t f = TFT_FpsGet();
        uint16_t w = (uint16_t)((f >= 200u) ? 108u : ((uint32_t)f * 108u / 200u));

        TFT_DrawRect(8, 42, 112, 14, TFT_WHITE);
        TFT_FillRect(10, 44, w, 10, TFT_YELLOW);
    }

    TFT_FpsInvalidate();
    TFT_FpsShow(20, 70, TFT_MAGENTA, TFT_BLACK);
    TFT_DrawString16(4, 92, TFT_GRAY1, TFT_BLACK, "bar: 0~20 fps");
    HAL_Delay(TFT_TEST_PAGE_DELAY);

    /*==========================================================================
     * 9. 动画与结束
     *========================================================================*/
    TFT_FillScreen(TFT_BLACK);
    TFT_DrawString16(4, 2, TFT_WHITE, TFT_BLACK, "[9] ANIMATION");

    /* 进度条（TFT_DrawRect + TFT_FillRect） */
    TFT_DrawRect(8, 22, 112, 14, TFT_WHITE);
    for (x = 0; x <= 108; x += 4)
    {
        TFT_FillRect((uint16_t)(10 + x), 24, 4, 10, TFT_GREEN);
        HAL_Delay(15);
    }

    /* 弹跳小球：逐行只更新"旧球消失 / 新球新增"的月牙区域，
     * 小球不会整个被擦掉，因此不会闪烁 */
    {
        const int32_t R = 8;
        uint8_t hw[9];                  /* hw[k] = 距圆心 k 行处的最大水平半宽 */
        int32_t k;
        int16_t bx = 12, by = 76, vx = 4, vy = 3;

        for (k = 0; k <= R; k++)
        {
            int32_t t = R;
            while ((t * t + k * k) > (R * R)) { t--; }
            hw[k] = (uint8_t)t;
        }

        /* 先完整画一次首帧。注意必须与后面的增量算法使用同一套跨度（hw 表）：
         * TFT_FillCircle 用的是 Bresenham 跨度，比 hw 略大几个像素，
         * 若用它画首帧，之后按 hw 擦除时初始位置就会残留几个像素。 */
        for (k = -R; k <= R; k++)
        {
            int32_t ry = (int32_t)by + k;
            int32_t w  = hw[(k < 0) ? -k : k];

            if (ry < 0 || ry >= (int32_t)TFT_HEIGHT) continue;
            TFT_FillRect((uint16_t)((int32_t)bx - w), (uint16_t)ry,
                         (uint16_t)(2 * w + 1), 1, TFT_YELLOW);
        }

        for (x = 0; x < 120u; x++)
        {
            int16_t nx = (int16_t)(bx + vx);
            int16_t ny = (int16_t)(by + vy);
            int32_t rlo, rhi;

            if (nx <= 10 || nx >= 118) { vx = (int16_t)(-vx); nx = (int16_t)(bx + vx); }
            if (ny <= 60 || ny >= 148) { vy = (int16_t)(-vy); ny = (int16_t)(by + vy); }

            /* 覆盖新旧两个圆并集的所有行（范围由实际位置决定，与步长无关） */
            rlo = (int32_t)((by < ny) ? by : ny) - R;
            rhi = (int32_t)((by > ny) ? by : ny) + R;

            for (k = rlo; k <= rhi; k++)
            {
                int32_t ry = k;         /* 注意：此时 k 已经是绝对行号 */
                int32_t d, w;
                int32_t o0 = 0, o1 = -1, n0 = 0, n1 = -1;

                if (ry < 0 || ry >= (int32_t)TFT_HEIGHT) continue;

                d = ry - (int32_t)by;
                if (d >= -R && d <= R) { w = hw[(d < 0) ? -d : d]; o0 = bx - w; o1 = bx + w; }

                d = ry - (int32_t)ny;
                if (d >= -R && d <= R) { w = hw[(d < 0) ? -d : d]; n0 = nx - w; n1 = nx + w; }

                tft_test_row_diff(o0, o1, n0, n1, (uint16_t)ry, TFT_BLACK);   /* 擦掉旧球多余的部分 */
                tft_test_row_diff(n0, n1, o0, o1, (uint16_t)ry, TFT_YELLOW);  /* 补上新球新增的部分 */
            }

            bx = nx;
            by = ny;
            HAL_Delay(12);
        }
    }

    /* 结束提示 + 背光闪烁 */
    TFT_FillScreen(TFT_BLACK);
    TFT_FillRect(0, 64, TFT_WIDTH, 28, TFT_BLUE);
    TFT_DrawString16(8, 70, TFT_WHITE, TFT_BLUE, "ALL TESTS PASS");

    for (i = 0; i < 3u; i++)
    {
        TFT_SetBacklight(0);
        HAL_Delay(150);
        TFT_SetBacklight(1);
        HAL_Delay(150);
    }
}
