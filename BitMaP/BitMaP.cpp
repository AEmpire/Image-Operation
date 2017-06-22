// BitMaP.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <windows.h>
using namespace std;

enum StretchMode
{
	nearest,  //最临近插值算法
	bilinear  //双线性内插值算法
};

void Stretch(const string& srcFile, const string& desFile, int desW, int desH, StretchMode mode)
{
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;

	FILE *pFile;
	if ((pFile = fopen(srcFile.c_str(), "rb")) == NULL)
	{
		printf("open bmp file error.");
		exit(-1);
	}
	//读取文件和Bitmap头信息
	fseek(pFile, 0, SEEK_SET);
	fread(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, pFile);
	fread(&bmiHeader, sizeof(BITMAPINFOHEADER), 1, pFile);
	//先不支持小于16位的位图
	int bitCount = bmiHeader.biBitCount;
	if (bitCount < 16)
	{
		exit(-1);
	}
	int srcW = bmiHeader.biWidth;
	int srcH = bmiHeader.biHeight;

	int lineSize = bitCount * srcW / 8;
	//偏移量，windows系统要求每个扫描行按四字节对齐
	int alignBytes = ((bmiHeader.biWidth * bitCount + 31) & ~31) / 8L
		- bmiHeader.biWidth * bitCount / 8L;
	//原图像缓存
	int srcBufSize = lineSize * srcH;
	BYTE* srcBuf = new BYTE[srcBufSize];
	int i, j;
	//读取文件中数据
	for (i = 0; i < srcH; i++)
	{
		fread(&srcBuf[lineSize * i], lineSize, 1, pFile);
		fseek(pFile, alignBytes, SEEK_CUR);
	}

	//目标图像缓存
	int desBufSize = ((desW * bitCount + 31) / 32) * 4 * desH;
	int desLineSize = ((desW * bitCount + 31) / 32) * 4;
	BYTE *desBuf = new BYTE[desBufSize];
	double rateH = (double)srcH / desH;
	double rateW = (double)srcW / desW;
	//最临近插值算法
	if (mode == nearest)
	{
		for (i = 0; i < desH; i++)
		{
			//选取最邻近的点
			int tSrcH = (int)(rateH * i + 0.5);
			for (j = 0; j < desW; j++)
			{
				int tSrcW = (int)(rateW * j + 0.5);
				memcpy(&desBuf[i * desLineSize] + j * bmiHeader.biBitCount / 8, &srcBuf[tSrcH * lineSize] + tSrcW * bmiHeader.biBitCount / 8, bmiHeader.biBitCount / 8);
			}
		}
	}
	//双线型内插值算法
	else
	{
		for (i = 0; i < desH; i++)
		{
			int tH = (int)(rateH * i);
			int tH1 = min(tH + 1, srcH - 1);
			float u = (float)(rateH * i - tH);
			for (j = 0; j < desW; j++)
			{
				int tW = (int)(rateW * j);
				int tW1 = min(tW + 1, srcW - 1);
				float v = (float)(rateW * j - tW);

				//f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1) 
				for (int k = 0; k < 3; k++)
				{
					desBuf[i * desLineSize + j * bitCount / 8 + k] =
						(1 - u)*(1 - v) * srcBuf[tH * lineSize + tW * bitCount / 8 + k] +
						(1 - u)*v*srcBuf[tH1 * lineSize + tW * bitCount / 8 + k] +
						u * (1 - v) * srcBuf[tH * lineSize + tW1 * bitCount / 8 + k] +
						u * v * srcBuf[tH1 * lineSize + tW1 * bitCount / 8 + k];
				}
			}
		}
	}

	//创建目标文件
	HFILE hfile = _lcreat(desFile.c_str(), 0);
	//文件头信息
	BITMAPFILEHEADER nbmfHeader;
	nbmfHeader.bfType = 0x4D42;
	nbmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
		+ desW * desH * bitCount / 8;
	nbmfHeader.bfReserved1 = 0;
	nbmfHeader.bfReserved2 = 0;
	nbmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	//Bitmap头信息
	BITMAPINFOHEADER   bmi;
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = desW;
	bmi.biHeight = desH;
	bmi.biPlanes = 1;
	bmi.biBitCount = bitCount;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;
	bmi.biXPelsPerMeter = 0;
	bmi.biYPelsPerMeter = 0;
	bmi.biClrUsed = 0;
	bmi.biClrImportant = 0;

	//写入文件头信息
	_lwrite(hfile, (LPCSTR)&nbmfHeader, sizeof(BITMAPFILEHEADER));
	//写入Bitmap头信息
	_lwrite(hfile, (LPCSTR)&bmi, sizeof(BITMAPINFOHEADER));
	//写入图像数据
	_lwrite(hfile, (LPCSTR)desBuf, desBufSize);
	_lclose(hfile);
}

int main(int argc, char* argv[])
{
	if ((fopen("‪‪FLAG_B24.BMP", "rb")) == NULL)
	{
		printf("open bmp file error.");
		return -1;
	}
	string srcFile("E:\FLAG_B24.BMP");
	string desFileN("nearest.bmp");
	string desFileB("bilinear.bmp");
	Stretch(srcFile, desFileN, 800, 600, nearest);
	Stretch(srcFile, desFileB, 800, 600, bilinear);
	//int alignBytes = ~31;
	//printf("alignbytes : %d",alignBytes);

	system("pause");
	return 0;
}