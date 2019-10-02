/**
 * @brief implementation of remap
 * 
 */
#include "quick_cv.hpp"
#include "common/qcv_log.hpp"
#include <algorithm>
#include <string.h>

#ifdef USE_X86
#include "common/NEON_2_SSE.h"
#elif defined USE_ARM
#include <arm_neon.h>
#endif

#ifdef MULTIPTHREAD
#include "MulAlgParam.hpp"
#endif

inline void sort(uint8x16_t &a, uint8x16_t &b)
{
    uint8x16_t min = vminq_u8(a, b);
    uint8x16_t max = vmaxq_u8(a, b);
    a = min;
    b = max;
}

inline void sort_low(uint8_t &x,  uint8_t &y)
{
    uint8_t min_num = min(x, y);
    uint8_t max_num = max(x, y);
    x = min_num;
    y = max_num;
}

void MedianBlur3X3_U8(unsigned char *src, unsigned char *dst, int width, int height, int src_stride, int dst_stride, int channel)
{

	int i, j, k, cn = channel;

    if(width == 1 || height == 1) {
        int len =  width + height - 1;
        int sdelta = height == 1 ? cn : src_stride;
        int sdelta0 = height == 1 ? 0 : src_stride - cn;
        int ddelta = height == 1 ? cn : dst_stride;

        for( i = 0; i < len; i++, src += sdelta0, dst += ddelta) {
            for( j = 0; j < cn; j++, src++ )
            {
                uint8_t p0 = src[i > 0 ? -sdelta : 0];
                uint8_t p1 = src[0];
                uint8_t p2 = src[i < len - 1 ? sdelta : 0];

                sort_low(p0, p1); sort_low(p1, p2); sort_low(p0, p1);
                dst[j] = p1;
            }
        }
        return;
    }


    width *= cn;
    for( i = 0; i < height; i++, dst += dst_stride )
    {
        uint8_t* row0 = src + max(i - 1, 0)*src_stride;
        uint8_t* row1 = src + i*src_stride;
        uint8_t* row2 = src + min(i + 1, height-1)*src_stride;
        int limit = cn;

        for(j = 0;; )
        {
            for( ; j < limit; j++ )
            {
                int j0 = j >= cn ? j - cn : j;
                int j2 = j < width - cn ? j + cn : j;
                uint8_t p0 = row0[j0]; uint8_t p1= row0[j]; uint8_t p2= row0[j2];
                uint8_t p3 = row1[j0]; uint8_t p4= row1[j]; uint8_t p5= row1[j2];
                uint8_t p6 = row2[j0]; uint8_t p7= row2[j]; uint8_t p8= row2[j2];

                sort_low(p1, p2); sort_low(p4, p5); sort_low(p7, p8); sort_low(p0, p1);
                sort_low(p3, p4); sort_low(p6, p7); sort_low(p1, p2); sort_low(p4, p5);
                sort_low(p7, p8); sort_low(p0, p3); sort_low(p5, p8); sort_low(p4, p7);
                sort_low(p3, p6); sort_low(p1, p4); sort_low(p2, p5); sort_low(p4, p7);
                sort_low(p4, p2); sort_low(p6, p4); sort_low(p4, p2);
                dst[j] = p4;
            }

            if( limit == width )
                break;
            
        
            for( ; j <= width - 16 - cn; j += 16 )
            {
                uint8x16_t p0 = vld1q_u8(row0+j-cn);
                uint8x16_t p1 = vld1q_u8(row0+j);
                uint8x16_t p2 = vld1q_u8(row0+j+cn);
                uint8x16_t p3 = vld1q_u8(row1+j-cn);
                uint8x16_t p4 = vld1q_u8(row1+j);
                uint8x16_t p5 = vld1q_u8(row1+j+cn);
                uint8x16_t p6 = vld1q_u8(row2+j-cn);
                uint8x16_t p7 = vld1q_u8(row2+j);
                uint8x16_t p8 = vld1q_u8(row2+j+cn);

                sort(p1, p2); sort(p4, p5); sort(p7, p8); sort(p0, p1);
                sort(p3, p4); sort(p6, p7); sort(p1, p2); sort(p4, p5);
                sort(p7, p8); sort(p0, p3); sort(p5, p8); sort(p4, p7);
                sort(p3, p6); sort(p1, p4); sort(p2, p5); sort(p4, p7);
                sort(p4, p2); sort(p6, p4); sort(p4, p2);
                vst1q_u8(dst+j, p4);
            }     
            limit = width;
        }
    }
}


void MedianBlur5x5_U8(unsigned char *src, unsigned char *dst, int width, int height, int src_stride, int dst_stride, int channel)
{
    int i, j, k, cn = channel;

    if( width == 1 || height == 1 )
    {
        int len = width + height - 1;
        int sdelta = height == 1 ? cn : src_stride;
        int sdelta0 = height == 1 ? 0 : src_stride - cn;
        int ddelta = height == 1 ? cn : dst_stride;

        for( i = 0; i < len; i++, src += sdelta0, dst += ddelta )
            for( j = 0; j < cn; j++, src++ )
            {
                int i1 = i > 0 ? -sdelta : 0;
                int i0 = i > 1 ? -sdelta*2 : i1;
                int i3 = i < len-1 ? sdelta : 0;
                int i4 = i < len-2 ? sdelta*2 : i3;
                uint8_t p0 = src[i0], p1 = src[i1], p2 = src[0], p3 = src[i3], p4 = src[i4];

                sort_low(p0, p1); sort_low(p3, p4); sort_low(p2, p3); sort_low(p3, p4); sort_low(p0, p2);
                sort_low(p2, p4); sort_low(p1, p3); sort_low(p1, p2);
                dst[j] = p2;
            }
        return;
    }

   width *= cn;
   for( i = 0; i < height; i++, dst += dst_stride )
   {
       uint8_t* row[5];
       row[0] = src + max(i - 2, 0)*src_stride;
       row[1] = src + max(i - 1, 0)*src_stride;
       row[2] = src + i*src_stride;
       row[3] = src + min(i + 1, height-1)*src_stride;
       row[4] = src + min(i + 2, height-1)*src_stride;
       int limit = cn*2;

       for(j = 0;; )
       {
           for( ; j < limit; j++ )
           {
               uint8_t p[25];
               int j1 = j >= cn ? j - cn : j;
               int j0 = j >= cn*2 ? j - cn*2 : j1;
               int j3 = j < width - cn ? j + cn : j;
               int j4 = j < width - cn*2 ? j + cn*2 : j3;
               for( k = 0; k < 5; k++ )
               {
                   uint8_t* rowk = row[k];
                   p[k*5] = rowk[j0]; p[k*5+1] = rowk[j1];
                   p[k*5+2] = rowk[j]; p[k*5+3] = rowk[j3];
                   p[k*5+4] = rowk[j4];
               }

               sort_low(p[1], p[2]); sort_low(p[0], p[1]); sort_low(p[1], p[2]); sort_low(p[4], p[5]); sort_low(p[3], p[4]);
               sort_low(p[4], p[5]); sort_low(p[0], p[3]); sort_low(p[2], p[5]); sort_low(p[2], p[3]); sort_low(p[1], p[4]);
               sort_low(p[1], p[2]); sort_low(p[3], p[4]); sort_low(p[7], p[8]); sort_low(p[6], p[7]); sort_low(p[7], p[8]);
               sort_low(p[10], p[11]); sort_low(p[9], p[10]); sort_low(p[10], p[11]); sort_low(p[6], p[9]); sort_low(p[8], p[11]);
               sort_low(p[8], p[9]); sort_low(p[7], p[10]); sort_low(p[7], p[8]); sort_low(p[9], p[10]); sort_low(p[0], p[6]);
               sort_low(p[4], p[10]); sort_low(p[4], p[6]); sort_low(p[2], p[8]); sort_low(p[2], p[4]); sort_low(p[6], p[8]);
               sort_low(p[1], p[7]); sort_low(p[5], p[11]); sort_low(p[5], p[7]); sort_low(p[3], p[9]); sort_low(p[3], p[5]);
               sort_low(p[7], p[9]); sort_low(p[1], p[2]); sort_low(p[3], p[4]); sort_low(p[5], p[6]); sort_low(p[7], p[8]);
               sort_low(p[9], p[10]); sort_low(p[13], p[14]); sort_low(p[12], p[13]); sort_low(p[13], p[14]); sort_low(p[16], p[17]);
               sort_low(p[15], p[16]); sort_low(p[16], p[17]); sort_low(p[12], p[15]); sort_low(p[14], p[17]); sort_low(p[14], p[15]);
               sort_low(p[13], p[16]); sort_low(p[13], p[14]); sort_low(p[15], p[16]); sort_low(p[19], p[20]); sort_low(p[18], p[19]);
               sort_low(p[19], p[20]); sort_low(p[21], p[22]); sort_low(p[23], p[24]); sort_low(p[21], p[23]); sort_low(p[22], p[24]);
               sort_low(p[22], p[23]); sort_low(p[18], p[21]); sort_low(p[20], p[23]); sort_low(p[20], p[21]); sort_low(p[19], p[22]);
               sort_low(p[22], p[24]); sort_low(p[19], p[20]); sort_low(p[21], p[22]); sort_low(p[23], p[24]); sort_low(p[12], p[18]);
               sort_low(p[16], p[22]); sort_low(p[16], p[18]); sort_low(p[14], p[20]); sort_low(p[20], p[24]); sort_low(p[14], p[16]);
               sort_low(p[18], p[20]); sort_low(p[22], p[24]); sort_low(p[13], p[19]); sort_low(p[17], p[23]); sort_low(p[17], p[19]);
               sort_low(p[15], p[21]); sort_low(p[15], p[17]); sort_low(p[19], p[21]); sort_low(p[13], p[14]); sort_low(p[15], p[16]);
               sort_low(p[17], p[18]); sort_low(p[19], p[20]); sort_low(p[21], p[22]); sort_low(p[23], p[24]); sort_low(p[0], p[12]);
               sort_low(p[8], p[20]); sort_low(p[8], p[12]); sort_low(p[4], p[16]); sort_low(p[16], p[24]); sort_low(p[12], p[16]);
               sort_low(p[2], p[14]); sort_low(p[10], p[22]); sort_low(p[10], p[14]); sort_low(p[6], p[18]); sort_low(p[6], p[10]);
               sort_low(p[10], p[12]); sort_low(p[1], p[13]); sort_low(p[9], p[21]); sort_low(p[9], p[13]); sort_low(p[5], p[17]);
               sort_low(p[13], p[17]); sort_low(p[3], p[15]); sort_low(p[11], p[23]); sort_low(p[11], p[15]); sort_low(p[7], p[19]);
               sort_low(p[7], p[11]); sort_low(p[11], p[13]); sort_low(p[11], p[12]);
               dst[j] = p[12];
           }

           if( limit == width )
               break;


          for( ; j <= width - 16 - cn*2; j += 16 )
            {
                uint8x16_t p[25];
                for( k = 0; k < 5; k++ )
                {
                    uint8_t* rowk = row[k];
                    p[k*5] = vld1q_u8(rowk+j-cn*2); p[k*5+1] = vld1q_u8(rowk+j-cn);
                    p[k*5+2] = vld1q_u8(rowk+j); p[k*5+3] = vld1q_u8(rowk+j+cn);
                    p[k*5+4] = vld1q_u8(rowk+j+cn*2);
                }

                sort(p[1], p[2]); sort(p[0], p[1]); sort(p[1], p[2]); sort(p[4], p[5]); sort(p[3], p[4]);
                sort(p[4], p[5]); sort(p[0], p[3]); sort(p[2], p[5]); sort(p[2], p[3]); sort(p[1], p[4]);
                sort(p[1], p[2]); sort(p[3], p[4]); sort(p[7], p[8]); sort(p[6], p[7]); sort(p[7], p[8]);
                sort(p[10], p[11]); sort(p[9], p[10]); sort(p[10], p[11]); sort(p[6], p[9]); sort(p[8], p[11]);
                sort(p[8], p[9]); sort(p[7], p[10]); sort(p[7], p[8]); sort(p[9], p[10]); sort(p[0], p[6]);
                sort(p[4], p[10]); sort(p[4], p[6]); sort(p[2], p[8]); sort(p[2], p[4]); sort(p[6], p[8]);
                sort(p[1], p[7]); sort(p[5], p[11]); sort(p[5], p[7]); sort(p[3], p[9]); sort(p[3], p[5]);
                sort(p[7], p[9]); sort(p[1], p[2]); sort(p[3], p[4]); sort(p[5], p[6]); sort(p[7], p[8]);
                sort(p[9], p[10]); sort(p[13], p[14]); sort(p[12], p[13]); sort(p[13], p[14]); sort(p[16], p[17]);
                sort(p[15], p[16]); sort(p[16], p[17]); sort(p[12], p[15]); sort(p[14], p[17]); sort(p[14], p[15]);
                sort(p[13], p[16]); sort(p[13], p[14]); sort(p[15], p[16]); sort(p[19], p[20]); sort(p[18], p[19]);
                sort(p[19], p[20]); sort(p[21], p[22]); sort(p[23], p[24]); sort(p[21], p[23]); sort(p[22], p[24]);
                sort(p[22], p[23]); sort(p[18], p[21]); sort(p[20], p[23]); sort(p[20], p[21]); sort(p[19], p[22]);
                sort(p[22], p[24]); sort(p[19], p[20]); sort(p[21], p[22]); sort(p[23], p[24]); sort(p[12], p[18]);
                sort(p[16], p[22]); sort(p[16], p[18]); sort(p[14], p[20]); sort(p[20], p[24]); sort(p[14], p[16]);
                sort(p[18], p[20]); sort(p[22], p[24]); sort(p[13], p[19]); sort(p[17], p[23]); sort(p[17], p[19]);
                sort(p[15], p[21]); sort(p[15], p[17]); sort(p[19], p[21]); sort(p[13], p[14]); sort(p[15], p[16]);
                sort(p[17], p[18]); sort(p[19], p[20]); sort(p[21], p[22]); sort(p[23], p[24]); sort(p[0], p[12]);
                sort(p[8], p[20]); sort(p[8], p[12]); sort(p[4], p[16]); sort(p[16], p[24]); sort(p[12], p[16]);
                sort(p[2], p[14]); sort(p[10], p[22]); sort(p[10], p[14]); sort(p[6], p[18]); sort(p[6], p[10]);
                sort(p[10], p[12]); sort(p[1], p[13]); sort(p[9], p[21]); sort(p[9], p[13]); sort(p[5], p[17]);
                sort(p[13], p[17]); sort(p[3], p[15]); sort(p[11], p[23]); sort(p[11], p[15]); sort(p[7], p[19]);
                sort(p[7], p[11]); sort(p[11], p[13]); sort(p[11], p[12]);
                
                vst1q_u8(dst+j, p[12]);
            }

          limit = width;

        }
    }
}


