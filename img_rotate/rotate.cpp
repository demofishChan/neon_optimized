
//该代码仅适用于32位（RGBA）图像的转置
//加速比率：10倍左右
static void _transpose(unsigned char* dest_s, unsigned char* source_s, int dstw, int srcw, int dw, int dh)
{
    int ista=0,jsta=0;
    const int bpp = 4;//RGBA
#ifdef HAS_NEON
/*矩阵转置示意图
    d1       d2
 x00 x01  x02 x03
    d3       d4
 x10 x11  x12 x13
    d5       d6
 x20 x21  x22 x23
    d7       d8
 x30 x31  x32 x33

       _||_
       \  /
        \/

    d1       d2
 x00 x10  x20 x30
    d3       d4
 x01 x11  x21 x31
    d5       d6
 x02 x12  x22 x32
    d7       d8
 x03 x13  x23 x33
 */

    const int unit = 4;//必须是4
    //GPCLOCK;
    int nw = dw/unit;
    int nh = dh/unit;
    int srcstride = srcw*bpp;
    int dststride = dstw*bpp;
    if (nw > 1 && nh > 1)
    {
        asm (
                     "mov r5, #4\t\n"
                     "mul r8, %[srcstride], r5\t\n"
                     "mul r9, %[dststride], r5\t\n"
                     "mul r10, r5, r5\t\n"//In fact, it's 4*r5
                     "movs r5, %[nh]\t\n"//i
                     "sub r5, r5, #1\t\n"
                     "1:\t\n"
                     "sub r4, %[nw], #1\t\n"//j
                     "2:\t\n"
                     "mla r6, r4, r8, %[source_s]\t\n"
                     "mla r6, r5, r10, r6\t\n"
                     "vld1.32 {d1, d2}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d3, d4}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d5, d6}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d7, d8}, [r6]\t\n"

                     /*Transpose internal*/
                     "vtrn.32 d1, d3\t\n"
                     "vtrn.32 d2, d4\t\n"
                     "vtrn.32 d5, d7\t\n"
                     "vtrn.32 d6, d8\t\n"
                     "vswp d2, d5\t\n"
                     "vswp d4, d7\t\n"

                     "mla r7, r5, r9, %[dest_s]\t\n"
                     "mla r7, r4, r10, r7\t\n"
                     "vst1.32 {d1, d2}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d3, d4}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d5, d6}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d7, d8}, [r7]\t\n"

                     "subs r4, r4, #1\t\n"
                     "bne 2b\t\n"//Loop1

                     "mla r6, r4, r8, %[source_s]\t\n"
                     "mla r6, r5, r10, r6\t\n"
                     "vld1.32 {d1, d2}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d3, d4}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d5, d6}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d7, d8}, [r6]\t\n"

                     /*Transpose internal*/
                     "vtrn.32 d1, d3\t\n"
                     "vtrn.32 d2, d4\t\n"
                     "vtrn.32 d5, d7\t\n"
                     "vtrn.32 d6, d8\t\n"
                     "vswp d2, d5\t\n"
                     "vswp d4, d7\t\n"

                     "mla r7, r5, r9, %[dest_s]\t\n"
                     "mla r7, r4, r10, r7\t\n"
                     "vst1.32 {d1, d2}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d3, d4}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d5, d6}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d7, d8}, [r7]\t\n"

                     "subs r5, r5, #1\t\n"
                     "bne 1b\t\n"//Loop2

                     /*Last line*/
                     "sub r4, %[nw], #1\t\n"//j
                     "4:\t\n"
                     "mla r6, r4, r8, %[source_s]\t\n"
                     "mla r6, r5, r10, r6\t\n"
                     "vld1.32 {d1, d2}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d3, d4}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d5, d6}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d7, d8}, [r6]\t\n"

                     /*Transpose internal*/
                     "vtrn.32 d1, d3\t\n"
                     "vtrn.32 d2, d4\t\n"
                     "vtrn.32 d5, d7\t\n"
                     "vtrn.32 d6, d8\t\n"
                     "vswp d2, d5\t\n"
                     "vswp d4, d7\t\n"

                     "mla r7, r5, r9, %[dest_s]\t\n"
                     "mla r7, r4, r10, r7\t\n"
                     "vst1.32 {d1, d2}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d3, d4}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d5, d6}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d7, d8}, [r7]\t\n"

                     "subs r4, r4, #1\t\n"
                     "bne 4b\t\n"//Loop1
                     "mla r6, r4, r8, %[source_s]\t\n"
                     "mla r6, r5, r10, r6\t\n"
                     "vld1.32 {d1, d2}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d3, d4}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d5, d6}, [r6]\t\n"
                     "add r6, r6, %[srcstride]\t\n"
                     "vld1.32 {d7, d8}, [r6]\t\n"

                     /*Transpose internal*/
                     "vtrn.32 d1, d3\t\n"
                     "vtrn.32 d2, d4\t\n"
                     "vtrn.32 d5, d7\t\n"
                     "vtrn.32 d6, d8\t\n"
                     "vswp d2, d5\t\n"
                     "vswp d4, d7\t\n"

                     "mla r7, r5, r9, %[dest_s]\t\n"
                     "mla r7, r4, r10, r7\t\n"
                     "vst1.32 {d1, d2}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d3, d4}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d5, d6}, [r7]\t\n"
                     "add r7, r7, %[dststride]\t\n"
                     "vst1.32 {d7, d8}, [r7]\t\n"

                     "5:\t\n"
                     : [srcstride] "+r" (srcstride), [dststride] "+r" (dststride), [source_s] "+r" (source_s), [dest_s] "+r" (dest_s), [nw] "+r" (nw), [nh] "+r" (nh)
                     :
                     : "r4", "r5", "r6", "r7", "r8", "r9","r10", "cc","memory", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8"
                     );
    }
    ista = nh*unit;
    jsta = nw*unit;
#endif
//边角处理，先处理图像下边缘，再处理图像右边缘
    for (int i=ista; i<dh; ++i)
    {
        for (int j=0; j<dw; ++j)
        {
            unsigned char* dest = dest_s + (i*dstw+j)*bpp;
            unsigned char* source = source_s + (j*srcw+i)*bpp;
            ::memcpy(dest, source, bpp*sizeof(unsigned char));
        }
    }
    for (int i=0; i<ista; ++i)
    {
        for (int j=jsta; j<dw; ++j)
        {
            unsigned char* dest = dest_s + (i*dstw+j)*bpp;
            unsigned char* source = source_s + (j*srcw+i)*bpp;
            ::memcpy(dest, source, bpp*sizeof(unsigned char));
        }
    }

}

