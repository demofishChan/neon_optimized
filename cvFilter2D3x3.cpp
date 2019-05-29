#include <stdio.h>
#include <string.h>
#include "NEONvsSSE.h"

#ifndef ABS
#define ABS(a) ((a) < 0 ? -(a) : (a))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

typedef long					MLong;
typedef float					MFloat;
typedef double					MDouble;
typedef unsigned char			MByte;
typedef unsigned short			MWord;
typedef unsigned int 			MDWord;
typedef void*					MHandle;
typedef char					MChar;
typedef long					MBool;
typedef void					MVoid;
typedef void*					MPVoid;
typedef char*					MPChar;
typedef short					MShort;
typedef const char*				MPCChar;
typedef	MLong					MRESULT;
typedef MDWord					MCOLORREF;
typedef	signed		char		MInt8;
typedef	unsigned	char		MUInt8;
typedef	signed		short		MInt16;
typedef	unsigned	short		MUInt16;
typedef signed		int			MInt32;
typedef unsigned	int			MUInt32;

MVoid SwapBufList(MFloat **ppBufList, MInt32 length)
{
	MFloat *pTmp;
	MInt32 i;

	pTmp = ppBufList[0];
	for (i = 0; i < length - 1; i++)
	{
		ppBufList[i] = ppBufList[i + 1];
	}
	ppBufList[length - 1] = pTmp;
}

MVoid cvFilter2D3x3_ROW(MFloat *pTmp, MFloat *pSrc, MInt32 w, MFloat *pweight)
{
	MFloat sum0, sum1;
	MInt32 j;
	MFloat t0, t1, t2, t3;


	{
		t0 = pSrc[0];
		t1 = pSrc[1];
		sum0 = t0 * (pweight[0]) + t0 * pweight[1] + t1 * (pweight[2]);
		pTmp[0] = sum0;

		pSrc += 1;
		pTmp += 1;
	}

	j = 1;

	for (; j < w - 1; j++)
	{
		sum0 = pSrc[0] * pweight[1];
		sum0 += (pSrc[-1] * pweight[0] + pSrc[1] * (pweight[2]));
		pTmp[0] = sum0;

		pSrc += 1;
		pTmp += 1;
	}

	{
		t0 = pSrc[0];
		t1 = pSrc[-1];
		sum0 = t0 * pweight[1] + t0 * (pweight[2]) + t1 * pweight[0];
		pTmp[0] = sum0;

		pSrc += 1;
		pTmp += 1;
	}
}

MVoid cvFilter2D3x3_COL(MFloat **pBuf, MFloat *pDst, MInt32 w, MFloat *pweight)
{
	int i, n;
	MFloat sum;
	MFloat * bufTmp0, *bufTmp1, *bufTmp2;
	bufTmp0 = pBuf[0], bufTmp1 = pBuf[1], bufTmp2 = pBuf[2];
	i = 0;

	for (; i < w; i++)
	{
		int val;
		sum = bufTmp2[0];
		sum += (bufTmp1[0] + bufTmp0[0]);
		pDst[0] = sum;

		bufTmp0 += 1, bufTmp1 += 1, bufTmp2 += 1;
		pDst += 1;
	}
}

MVoid cvFilter2D3x3_COL_y(MFloat **pBuf, MFloat *pDst, MInt32 w, MFloat *pweight)
{
	int i, n;
	MFloat sum;
	MFloat * bufTmp0, *bufTmp1, *bufTmp2;
	bufTmp0 = pBuf[0], bufTmp1 = pBuf[1], bufTmp2 = pBuf[2];
	i = 0;

	for (; i < w; i++)
	{
		sum = (bufTmp0[0] * pweight[0] + bufTmp2[0] * pweight[6]);
		pDst[0] = sum;

		bufTmp0 += 1, bufTmp1 += 1, bufTmp2 += 1;
		pDst += 1;
	}
}

MVoid cvFilter2D3x3(MFloat *pSrc, MInt32 lW, MInt32 lH, MInt32 lSrcLB, MFloat *pDst, MInt32 lDstLB, MFloat *pBuf,
	 MFloat *pweight)
{
	MInt32 i, j;
	MFloat *ppBufListRow[3];
	MInt32 lPreLine = 0,
		lNexLine = 0;
	MInt32 lTop = 0 - lPreLine;
	MInt32 lBot = lH + lNexLine;
	MInt32 line = 0 - 1;

	MFloat *tmpSrc = pSrc + lTop * lSrcLB;
	MFloat *tmpDst = pDst;

	for (i = 0; i < 3; i++)
	{
		ppBufListRow[i] = pBuf + i*lW;
	}

	for (; line < lTop; line++)
	{
		SwapBufList(ppBufListRow, 3);
		cvFilter2D3x3_ROW(ppBufListRow[2], tmpSrc, lW, pweight);
	}
	for (; line < 1; line++)
	{
		SwapBufList(ppBufListRow, 3);
		cvFilter2D3x3_ROW(ppBufListRow[2], tmpSrc, lW, pweight);
		tmpSrc += lSrcLB;
	}

	for (; line < lBot; line++)
	{
		SwapBufList(ppBufListRow, 3);
		cvFilter2D3x3_ROW(ppBufListRow[2], tmpSrc, lW, pweight);
		cvFilter2D3x3_COL_y(ppBufListRow, tmpDst, lW, pweight);
		tmpSrc += lSrcLB;
		tmpDst += lDstLB;
	}

	tmpSrc -= lSrcLB;
	for (; line < lH + 1; line++)
	{
		SwapBufList(ppBufListRow, 3);
		cvFilter2D3x3_ROW(ppBufListRow[2], tmpSrc, lW, pweight);
		cvFilter2D3x3_COL_y(ppBufListRow, tmpDst, lW, pweight);
		tmpDst += lDstLB;
	}
}

MVoid extendImg(MFloat *pSrc, MInt32 lSrcW, MInt32 lSrcH, MInt32 lSrcLB, MFloat *pDst,
	MInt32 lDstW, MInt32 lDstH, MInt32 lDstLB, MInt32 lRadius)
{
	MFloat *src = pSrc;
	MFloat *dst = pDst;
	for (int i = 0; i < lSrcH; i++)
	{
		for (int k = 0; k < lRadius; k++)
		{
			dst[k + lRadius*lDstLB] = src[0];
		}
		memcpy(dst + lRadius + lRadius*lDstLB, src, lSrcW * 4);
		for (int k = 0; k < lRadius; k++)
		{
			dst[k + lRadius + lSrcW + lRadius*lDstLB] = src[lSrcW - 1];
		}
		dst += lDstLB;
		src += lSrcLB;
	}
	for (int i = 0; i < lRadius; i++)
	{
		memcpy(pDst + i*lDstLB, pDst + lRadius*lDstLB, lDstLB * 4);
		memcpy(pDst + (lSrcH + lRadius + i)*lDstLB, pDst + (lSrcH + lRadius - 1)*lDstLB, lDstLB * 4);
	}
}

MVoid cvFilter2D_FullCompute(MFloat *pSrc, MInt32 lW, MInt32 lH, MInt32 lSrcLB, MFloat *pDst, MInt32 lDstW, MInt32 lDstH, MInt32 lDstLB, MFloat *pBuf,
	MFloat *pweight, MInt32 lRadius)
{
	MInt32 i, j, k, l, m, n;
	MInt32 kerWeight = 2 * lRadius + 1;

	MFloat *tmpSrc = pSrc + lRadius * lSrcLB;
	MFloat *tmpDst = pDst;
	MFloat *tmpweight = pweight;

	MFloat *ptmpBuf[9];

	for (i = 0; i < lDstW; i++)
	{
		for (k = 0; k < kerWeight; k++)
		{
			ptmpBuf[k] = tmpSrc - (lRadius - k) * lSrcLB + i;
		}
		for (j = 0; j < lDstH; j++)
		{
			MFloat sum = 0;
			for (l = 0; l < kerWeight; l++)
			{
				for (m = 0; m < kerWeight; m++)
				{
					MFloat val = ptmpBuf[l][m] * tmpweight[l*kerWeight + m];
					sum += val;
				}
			}
			tmpDst[0] = sum;
			tmpDst += lDstLB;
			for (n = 0; n < (kerWeight - 1); n++)
			{
				ptmpBuf[n] = ptmpBuf[n + 1];
			}
			ptmpBuf[kerWeight - 1] += lSrcLB;
		}
		tmpDst = ++pDst;
	}
}


int main()
{
	MFloat pSrc[81] = { 0, 1.0, 2, 3, 4, 5.0, 6, 7, 11,
		8, 9, 1.0, 1.1, 12, 1.30, 14, 15, 21,
		16, 170, 18, 1.9, 20.0, 2.1, 22, 2.3, 25,
		24, 2.5, 26.0, 27, 2.8, 29, 30, 31, 33,
		32, 3.3, 34, 35, 36, 370, 3.8, 3.9, 40,
		40, 41.0, 42, 43, 4.40, 45, 46, 47, 42,
		4.80, 4.9, 5.0, 51, 52, 53, 54, 55, 58,
		56, 5.70, 58, 59, 6.00, 61, 62, 63, 70,
		12, 32, 52, 62, 2, 39, 9.8, 8.5, 65
	};
	MInt32 lW = 9;
	MInt32 lH = 9;
	MInt32 lSrcLB = 9;

	MFloat pSrc1[289] = { 0 };
	MInt32 lW1 = 17;
	MInt32 lH1 = 17;
	MInt32 lSrcLB1 = 17;

	MFloat pDst0[81] = { 0 };
	MFloat pDst1[81] = { 0 };
	MInt32 lDstLB = 9;
	MFloat pweight[9] = {1,1,1,0,0,0,-1,-1,-1};

	MFloat pBuf[3 * 8 * sizeof(MFloat)] = { 0 };
	MInt32 lRadius = 1;

	cvFilter2D3x3(pSrc, lW, lH, lSrcLB, pDst0, lDstLB, pBuf, pweight);
	for (int i = 0; i < 81; i++)
	{
		if (i % 9 == 0)
			printf("\n");
		printf("%f ", pDst0[i]);
	}

	extendImg(pSrc, lW, lH, lSrcLB, pSrc1, lW1, lH1, lSrcLB1, lRadius);
	cvFilter2D_FullCompute(pSrc1, lW1, lH1, lSrcLB1, pDst1, lW, lH, lDstLB, pBuf, pweight, lRadius);

	printf("\n");
	for (int i = 0; i < 81; i++)
	{
		if (i % 9 == 0)
			printf("\n");
		printf("%f ", pDst1[i]);
	}

	printf("diff:\n");
	for (int i = 0; i < 81; i++)
	{
		if (i % 9 == 0)
			printf("\n");
		printf("%f ", pDst0[i]- pDst1[i]);
	}


	return 0;
}