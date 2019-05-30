#include <stdio.h>
#include <string.h>
#include <iostream>
#include "NEONvsSSE.h"

#define TEST_WIDTH 64
#define TEST_HEIGHT 64
#define SRC_FILE    "../data/bytes_42305.jpeg"
#define INPUT_FILE  "../data/bytes_42305.in"
#define OUTPUT_REF_FILE "../data/bytes_42305_ref.out"
#define OUTPUT_NEON_FILE "../data/bytes_42305_neon.out"

//for 1 channel
void integral1d_horizontal_1c_float_ref(float *sum, int rows, int cols) 
{
    //restrict to 1 channel in this function
    const int chs = 1;
    for (int y = 1; y < rows + 1; y++) 
    {
      float *ptr = sum + cols * chs * y;
      for (int x = 0; x < cols * chs; x++) 
      {
        ptr[x + chs] += ptr[x];
      }
    }
}


void integral1d_horizontal_1c_float_neon(float * ptrSrc, int rows, int cols, int chs)
{
    bool flag = true;
    if(1 == chs)
    {
        float32x4_t zero_32x4 = vdupq_n_f32(0.0f);//0   0   0   0
        float32_t tmp = 0.0f;
        for(int y=1; y<rows+1; y++)
        {
            float32_t *ptr = ptrSrc + y * cols;
            float32x4_t s0_32x4 = vdupq_n_f32(ptr[0]);//s0 s0  s0  s0
            int x = 0;
            for (x = 1; x<cols*chs - 4; x += 4)
            {
                float32_t * ptr_src = &ptr[x];
                float32x4_t src_32x4_0, src_32x4_1, src_32x4_2, src_32x4_3;
                tmp = vgetq_lane_f32(s0_32x4, 3);
                s0_32x4    = vdupq_n_f32(tmp);

                src_32x4_0 = vld1q_f32(ptr_src); //f1  f2  f3  f4
                src_32x4_1 = vextq_f32(zero_32x4, src_32x4_0, 3); //0  f1  f2  f3
                src_32x4_2 = vextq_f32(zero_32x4, src_32x4_0, 2); //0  0   f1  f2 
                src_32x4_3 = vextq_f32(zero_32x4, src_32x4_0, 1); //0  0   0   f1 

                src_32x4_0 = vaddq_f32(src_32x4_0, src_32x4_1);
                src_32x4_1 = vaddq_f32(src_32x4_2, src_32x4_3);
                src_32x4_2 = vaddq_f32(src_32x4_0, src_32x4_1);
                s0_32x4	   = vaddq_f32(src_32x4_2, s0_32x4);

                vst1q_f32(ptr_src, s0_32x4);
            }

            tmp = vgetq_lane_f32(s0_32x4, 3);
            ptr[x] += tmp;

            for(; x<cols*chs+1; x++)
            {
                ptr[x+chs] += ptr[x];
            }
        } 
        flag = false;
    }

    if(flag)
    {

        for(int y=1; y<rows+1; y++)
        {
            float *ptr = ptrSrc + y * cols;
            int x = 0;
            for(; x<cols*chs; x++)
            {
                ptr[x+chs] += ptr[x];
            }
        }
    }
}


int main()
{
    //init buffer
    int nSize = TEST_WIDTH * TEST_HEIGHT;
    float* pSrc = new float[nSize];
    float* pSum = new float[(TEST_HEIGHT + 1)/*border one rows*/ * TEST_WIDTH];
    memset(pSrc, 0, nSize * sizeof(float));
    memset(pSum, 0, (TEST_HEIGHT + 1)/*border one rows*/ * TEST_WIDTH * sizeof(float));

    //read file
    {
        FILE* fp = fopen(SRC_FILE, "rb");
        if(!fp)
        {
            std::cout << "read file error!!!" << std::endl;
            return  -1;
        }
        fread(pSrc, nSize, sizeof(float), fp);
        fclose(fp);
    }
    //write input
    {
        FILE* fp = fopen(INPUT_FILE, "wb");
        if(!fp)
        {
            std::cout << "write file error!!!" << std::endl;
            return  -1;
        }
        fwrite(pSrc, TEST_WIDTH*TEST_HEIGHT, sizeof(float), fp);
        fflush(fp);
        fclose(fp);
    }
    
    //border one rows
    for(int i = 0; i < TEST_HEIGHT; i++)
    {
        float* pSrc_t = pSrc + i * TEST_WIDTH;
        float* pDst_t = pSum + i * (TEST_WIDTH + 1);
        memcpy((pDst_t + 1), pSrc_t, TEST_WIDTH*sizeof(float));
    }
    
    //process by c code 
    integral1d_horizontal_1c_float_ref(pSum, TEST_HEIGHT, TEST_WIDTH);
    //write c_base result
    {
        FILE* fp = fopen(OUTPUT_REF_FILE, "wb");
        if(!fp)
        {
            std::cout << "write file error!!!" << std::endl;
            return  -1;
        }
        fwrite(pSum, (TEST_WIDTH + 1)*TEST_HEIGHT, sizeof(float), fp);
        fflush(fp);
        fclose(fp);
    }


    //process by neon code
    integral1d_horizontal_1c_float_neon(pSum, TEST_HEIGHT, TEST_WIDTH, 1);
    //write neon_base result
    {
        FILE* fp = fopen(OUTPUT_NEON_FILE, "wb");
        if(!fp)
        {
            std::cout << "write file error!!!" << std::endl;
            return  -1;
        }
        fwrite(pSum, (TEST_WIDTH + 1)*TEST_HEIGHT, sizeof(float), fp);
        fflush(fp);
        fclose(fp);
    }

    //compare resultsk

    //delete buffer
    delete[] pSrc;
    delete[] pSum;
}


