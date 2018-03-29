/////////////////////////////////////////////////////////////////////
//                        影像处理基础平台                         //
//                                                                 //
// 功  能：提供影像处理的基本功能。                                //
//                                                                 //
// 版  权：版权所有 2007~2010 梁跃                                 //
//=================================================================//


#ifndef	_IMAGE_H_
#define _IMAGE_H_


#include <stdio.h>
#include <stdlib.h>


#ifdef WIN32
#define EXPORT_C __declspec(dllexport)
#else
#define EXPORT_C
#endif


#pragma warning(disable : 4103)
#pragma warning(disable : 4251)
#pragma warning(disable : 4273)
#pragma warning(disable : 4996)


/////////////////////////////////////////////////////////////////////
//                             宏定义                              //
//=================================================================//
// 版本信息
#define VERSION                     0x01000000 // 1.0.0.0

// 数据类型
#ifdef WIN32
typedef unsigned char               BYTE;     // 1BYTE
typedef unsigned short              WORD;     // 2BYTE
typedef unsigned long               DWORD;    // 4BYTE
typedef unsigned long               COLORREF; // 4BYTE
typedef long                        LONG;     // 4BYTE
#else
typedef unsigned char               BYTE;     // 1BYTE
typedef unsigned short              WORD;     // 2BYTE
typedef unsigned int                DWORD;    // 4BYTE
typedef unsigned int                COLORREF; // 4BYTE
typedef int                         LONG;     // 4BYTE
#endif

#ifndef NO_ERROR
#define NO_ERROR                    0
#endif

#ifndef BOOL
#define BOOL                        int
#endif

#ifndef TRUE
#define TRUE                        1
#endif

#ifndef FALSE
#define FALSE                       0
#endif

#ifndef max
#define max(a,b)                    (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)                    (((a) < (b)) ? (a) : (b))
#endif


// 影像绝对尺寸
#define IMAGE_WIDTH(pimage)         ((int)((pimage)->bmpInfo->bmiHeader.biWidth))
#define IMAGE_HEIGHT(pimage)        ((int)((pimage)->bmpInfo->bmiHeader.biHeight))

// 处理区域尺寸
#define IMAGE_AREA_WIDTH(pimage)    ((int)((pimage)->ex - (pimage)->sx + 1))
#define IMAGE_AREA_HEIGHT(pimage)   ((int)((pimage)->ey - (pimage)->sy + 1))

// 影像色深
#define IMAGE_BITCOUNT(pimage)      ((int)((pimage)->bmpInfo->bmiHeader.biBitCount))

// 组合 RGB
#define RGB(r,g,b)                  ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define RGBA(r,g,b,a)               ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16))|(((DWORD)(BYTE)(a))<<24))
#define RGB565_RGB(r,g,b)           ((COLORREF)((((r >> 3) & 0x1F) << 11) | (((g >> 2) & 0x3F) << 5) | (((b >> 3) & 0x1F))))

// 分解 RGB
#define RGB565_GET_R(color)         (BYTE)((((color) >> 11) & 0x1F) << 3)
#define RGB565_GET_G(color)         (BYTE)((((color) >> 5)  & 0x3F) << 2)
#define RGB565_GET_B(color)         (BYTE)((((color) >> 0)  & 0x1F) << 3)

#define GET_RED(color)              (BYTE)((color)  &  0xff)
#define GET_GRN(color)              (BYTE)(((color) >> 0x08) & 0xff)
#define GET_BLU(color)              (BYTE)(((color) >> 0x10) & 0xff)
#define GET_ALP(color)              (BYTE)(((color) >> 0x18) & 0xff)
//===================================================================


/////////////////////////////////////////////////////////////////////
//                            错误返回值                           //
//=================================================================//
#ifndef _ERROR_H_

// 公有错误信息 (-50 ~ 0)
#define ERR_MEM                     -1 // 分配存储失败
#define ERR_IMAGE                   -2 // 影像无效
#define ERR_RANGE                   -3 // 处理区域超出影像尺寸
#define ERR_BITCOUNT                -4 // 影像色深不符合要求
#define ERR_PARAMETER               -5 // 参数错误
#define ERR_LIST                    -6 // 链标错误
#define ERR_MATRIX                  -7 // 矩阵错误

// I/O 错误信息 (-100 ~ -51)
#define ERR_IO_SEEK                 -51 // 影像文件定位错误
#define ERR_IO_FORMAT               -52 // 影像文件格式错误
#define ERR_IO_OPENFILE             -53 // 打开影像文件失败
#define ERR_IO_BITCOUNT             -54 // 影像文件中影像色深不符合要求
#define ERR_IO_COMPRESS             -55 // 影像使用压缩格式
#define ERR_IO_READDATA             -56 // 读取影像文件数据失败
#define ERR_IO_WRITEDATA            -57 // 写入影像文件数据失败

#endif
//===================================================================


#ifdef __cplusplus
extern "C" {
#endif


/////////////////////////////////////////////////////////////////////
//                          标准影像头定义                         //
//=================================================================//
#ifndef _WINDOWS_
typedef struct tagPOINT {
	LONG x;
	LONG y;
} POINT;

typedef struct tagRECT {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT;

typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFOHEADER {
	DWORD   biSize;
	LONG    biWidth;
	LONG    biHeight;
	WORD    biPlanes;
	WORD    biBitCount;
	DWORD   biCompression;
	DWORD   biSizeImage;
	LONG    biXPelsPerMeter;
	LONG    biYPelsPerMeter;
	DWORD   biClrUsed;
	DWORD   biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

#pragma pack(pop)

typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} BITMAPINFO;
#endif

typedef struct {
	int             sx, ex;    // 横向处理范围
	int             sy, ey;    // 纵向处理范围
	int             buffWidth; // 行缓冲区宽度
	unsigned char  *data;      // 影像数据缓冲 Image Data Buffer
	unsigned char **matrix;    // 影像数据矩阵 Image Data Matrix
	BITMAPINFO     *bmpInfo;   // 影像文件信息
} IMAGE;
//===================================================================


/////////////////////////////////////////////////////////////////////
//                         彩色系统结构定义                        //
//=================================================================//
// GRAY
typedef unsigned char IMGGRAY;

// RGB
typedef struct {
	unsigned char B;  // 蓝
	unsigned char G;  // 绿
	unsigned char R;  // 红
} IMGRGB;

// RGBA
typedef struct {
	unsigned char B;  // 蓝
	unsigned char G;  // 绿
	unsigned char R;  // 红
	unsigned char A;  // 透明
} IMGRGBA;

// YIQ彩色系统通常被北美的电视系统所采用（属于NTSC系统）
typedef struct {
	float         Y;  // 亮度
	float         I;  // 色度
	float         Q;  // 饱和度
} YIQ;

// YUV彩色系统被欧洲的电视系统采用（属于PAL系统）
typedef struct {
	float         Y;  // 亮度
	float         U;  // 色度
	float         V;  // 饱和度
} YUV;

// YCbCr是从YUV色彩系统中衍生出来的，JPEG采用的色彩系统正是该系统
typedef struct {
	float         Y;  // 亮度
	float         Cb; // 色度
	float         Cr; // 饱和度
} YCbCr;
//===================================================================


/////////////////////////////////////////////////////////////////////
//                             核心函数                            //
//=================================================================//
// 库信息函数
EXPORT_C void IMAGE_GetLibraryInfo(char * const pszinfo);
EXPORT_C unsigned long IMAGE_GetVersion(void);


// 矩阵存储
EXPORT_C void** IMAGE_AllocMatrix(int const nrows, int const ncols, int const nsize);
EXPORT_C void IMAGE_FreeMatrix(void ** const pmatrix);
EXPORT_C int IMAGE_CopyMatrix(void ** const pmtxsrc, void ** const pmtxdst, int const nrows, int const ncols, int const nsize);


// 影像存储
EXPORT_C void IMAGE_ZeroImage(IMAGE * const pimage);
EXPORT_C void IMAGE_FreeImage(IMAGE * const pimage);
EXPORT_C int IMAGE_AllocImage(IMAGE * const pimage, int const nwidth, int const nheight, int const nbitcount);

EXPORT_C int IMAGE_CopyImage(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_CopyImageArea(IMAGE * const pimgsrc, IMAGE * const pimgdst);


// 设置/恢复工作区函数
EXPORT_C void IMAGE_SetImageArea(IMAGE * const pimage, int const sx, int const sy, int const ex, int const ey);
EXPORT_C void IMAGE_ResetImageArea(IMAGE * const pimage);


// 像素操作函数
EXPORT_C unsigned long IMAGE_GetPixelColor(IMAGE * const pimage, int const x, int const y);
EXPORT_C void IMAGE_SetPixelColor(IMAGE * const pimage, int const x, int const y, unsigned long const color);


// 色深转换函数
EXPORT_C int IMAGE_Convert1BitTo8Bit(IMAGE * const pimg1bitsrc, IMAGE * const pimg8bitdst);
EXPORT_C int IMAGE_Convert8BitTo24Bit(IMAGE * const pimg8bitsrc, IMAGE * const pimg24bitdst);


// 灰度转换函数
#define GRAY_STD 0 // 按照标准灰度方式
#define GRAY_RED 1 // 按照红色通道方式
#define GRAY_GRN 2 // 按照绿色通道方式
#define GRAY_BLU 3 // 按照蓝色通道方式
EXPORT_C int IMAGE_ColorToGray(IMAGE * const pimg24bitsrc, IMAGE * const pimg8bitdst, int const nflag);


// 几何运算函数
EXPORT_C int IMAGE_FlipImage(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_MirrorImage(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_Resize(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_ResizeEx(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_Rotate(IMAGE * const pimgsrc, IMAGE * const pimgdst, float const fradian);
EXPORT_C int IMAGE_RotateEx(IMAGE * const pimgsrc, IMAGE * const pimgdst, float const fradian);


// 彩色系统变换
EXPORT_C int IMAGE_RGB2YIQ(IMAGE * const prgbsrc, YIQ ** const pyiqdst);
EXPORT_C int IMAGE_YIQ2RGB(YIQ ** const pyiqsrc, IMAGE * const prgbdst);

EXPORT_C int IMAGE_RGB2YUV(IMAGE * const prgbsrc, YUV ** const pyuvdst);
EXPORT_C int IMAGE_YUV2RGB(YUV ** const pyuvsrc, IMAGE * const prgbdst);

EXPORT_C int IMAGE_RGB2YCbCr(IMAGE * const prgbsrc, YCbCr ** const pycbcrdst);
EXPORT_C int IMAGE_YCbCr2RGB(YCbCr ** const pycbcrsrc, IMAGE * const prgbdst);


// I/O函数
EXPORT_C int IMAGE_LoadImage(char * const pszfilename, IMAGE * const pimage);

EXPORT_C int IMAGE_LoadBmpHeader(char * const pszfilename, BITMAPINFOHEADER * const pbmpinfoheader);
EXPORT_C int IMAGE_LoadBmp(char * const pszfilename, IMAGE * const pimage);
EXPORT_C int IMAGE_SaveBmp(char * const pszfilename, IMAGE * const pimage);

EXPORT_C int IMAGE_LoadJpgHeader(char * const pszfilename, BITMAPINFOHEADER * const pbmpinfoheader);
EXPORT_C int IMAGE_LoadJpg(char * const pszfilename, IMAGE * const pimage);
EXPORT_C int IMAGE_SaveJpg(char * const pszfilename, IMAGE * const pimage, int const nquality);

EXPORT_C int IMAGE_LoadPngHeader(char * const pszfilename, BITMAPINFOHEADER * const pbmpinfoheader);
EXPORT_C int IMAGE_LoadPng(char * const pszfilename, IMAGE * const pimage);
EXPORT_C int IMAGE_SavePng(char * const pszfilename, IMAGE * const pimage);


EXPORT_C int IMAGE_LoadImageEx(FILE * const pfile, IMAGE * const pimage);

EXPORT_C int IMAGE_LoadBmpHeaderEx(FILE * const pfile, BITMAPINFOHEADER * const pbmpinfoheader);
EXPORT_C int IMAGE_LoadBmpEx(FILE * const pfile, IMAGE * const pimage);
EXPORT_C int IMAGE_SaveBmpEx(FILE * const pfile, IMAGE * const pimage);

// 注意：IMAGE_LoadJpgEx会加载比实际需要更多的字节，这样会导致FILE定位超前，在使用时可将
//       Jpeg文件保存在文件最后或者在文件中详细记录纪录各个部分相对于文件开始的偏移量
EXPORT_C int IMAGE_LoadJpgHeaderEx(FILE * const pfile, BITMAPINFOHEADER * const pbmpinfoheader);
EXPORT_C int IMAGE_LoadJpgEx(FILE * const pfile, IMAGE * const pimage);
EXPORT_C int IMAGE_SaveJpgEx(FILE * const pfile, IMAGE * const pimage, int const nquality);

EXPORT_C int IMAGE_LoadPngHeaderEx(FILE * const pfile, BITMAPINFOHEADER * const pbmpinfoheader);
EXPORT_C int IMAGE_LoadPngEx(FILE * const pfile, IMAGE * const pimage);
EXPORT_C int IMAGE_SavePngEx(FILE * const pfile, IMAGE * const pimage);
//===================================================================


/////////////////////////////////////////////////////////////////////
//                             功能函数                            //
//=================================================================//
EXPORT_C int IMAGE_ChangeBright(IMAGE * const pimgsrc, IMAGE * const pimgdst, int const namount);
EXPORT_C int IMAGE_ChangeBrightEx(IMAGE * const pimgsrc, IMAGE * const pimgdst, float const famount);

EXPORT_C int IMAGE_Multiply(IMAGE * const pimgsrc, IMAGE * const pimgdst, int const nmultiply);
EXPORT_C int IMAGE_Divide(IMAGE * const pimgsrc, IMAGE * const pimgdst, int const ndivide);

EXPORT_C int IMAGE_ExpandContrast(IMAGE * const pimgsrc, IMAGE * const pimgdst, int const nlower, int const nupper);
EXPORT_C int IMAGE_GammaBrighten(IMAGE * const pimgsrc, IMAGE * const pimgdst, float const fgamma);

EXPORT_C int IMAGE_CalcHisto(IMAGE * const pimage, unsigned long * const redtab, unsigned long * const grntab, unsigned long * const blutab);
EXPORT_C int IMAGE_Histoequalize(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_Negative(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_BlackWhite(IMAGE * const pimgsrc, IMAGE * const pimgdst, int const nthresh);



EXPORT_C int IMAGE_Blur(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_GaussBlur(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_GaussBlurEx(IMAGE * const pimgsrc, IMAGE * const pimgdst, float const sigma);

EXPORT_C int IMAGE_Sharpen(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_SharpenGentle(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_SharpenLaplace(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_Outline(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_MedianFilter(IMAGE * const pimgsrc, IMAGE * const pimgdst, int const nsize);
EXPORT_C int IMAGE_RemoveNoise(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_MatrixConvert(IMAGE * const pimgsrc, IMAGE * const pimgdst, char * const kernel);
EXPORT_C int IMAGE_MatrixConvertEx(IMAGE * const pimgsrc, IMAGE * const pimgdst, char * const kernel, int const nsize, int const ndivide);



EXPORT_C int IMAGE_Dilate(IMAGE * const pimgsrc, IMAGE * const pimgdst, int const namount);
EXPORT_C int IMAGE_Erode(IMAGE * const pimgsrc, IMAGE * const pimgdst, int const namount);

EXPORT_C int IMAGE_DilationH(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_DilationV(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_DilationBlock(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_ErosionH(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_ErosionV(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_ErosionBlock(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_OpenH(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_OpenV(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_OpenBlock(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_CloseH(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_CloseV(IMAGE * const pimgsrc, IMAGE * const pimgdst);
EXPORT_C int IMAGE_CloseBlock(IMAGE * const pimgsrc, IMAGE * const pimgdst);

EXPORT_C int IMAGE_Thinning(IMAGE * const pimgsrc, IMAGE * const pimgdst);



// 复数
typedef struct {
	double re; // 实部
	double im; // 虚部
} COMPLEX;

EXPORT_C int IMAGE_DFT(IMAGE * const pimgsrc, COMPLEX ** const ppdata, int const powerwidth, int const powerheight);
EXPORT_C int IMAGE_IDFT(COMPLEX ** const ppdata, IMAGE * const pimgdst, int const powerwidth, int const powerheight);

EXPORT_C int IMAGE_DCT(IMAGE * const pimgsrc, float ** const ppdata, int const powerwidth, int const powerheight);
EXPORT_C int IMAGE_IDCT(float ** const ppdata, IMAGE * const pimgdst, int const powerwidth, int const powerheight);

EXPORT_C int IMAGE_DWT(IMAGE * const pimgsrc, float ** const ppdata, int const levs, BOOL const bpacket, char const cparam, int const nparam);
EXPORT_C int IMAGE_IDWT(float ** const ppdata, IMAGE * const pimgdst, int const levs, BOOL const bpacket, char const cparam, int const nparam);
//===================================================================



#ifdef __cplusplus
}
#endif

#endif
