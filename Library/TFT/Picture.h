#ifndef __PICTURE_H
#define __PICTURE_H

/* 100x100 RGB565 位图，共 20008 字节 = 8 字节图片头 + 100*100*2 像素
 * 用法：TFT_DrawImageHdr(x, y, gImage_k1271cn);  */
extern const unsigned char gImage_k1271cn[20008];

#endif
