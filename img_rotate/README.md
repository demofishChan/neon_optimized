### 分而治之
图像转置的优化思路是：   
1、将图像分割成一系列小矩阵。   
分成的小矩阵当然是越大越好，但在矩阵变大时，汇编代码的复杂度变高，且寄存器如果使用完了也非常难处理，这里选的是4X4的矩阵。   
2、每个小矩阵的宏观位置转置。   
3、实现每个小矩阵的内部转置。   
必须把矩阵图和寄存器向量的关系图画清，然后推演一番。   
Neon指令vtrn是解决转置问题的核心。   


### 核心指令示意图：
![Image text](https://github.com/demofishChan/neon_optimized/blob/master/img_rotate/1.jpg)

4、边角处理 
写惯了基于一行的Neon优化，到这步很容易犯错，一定要记得这里是二维的Neon优化，边角料是两条边。 


### 示意图：
![Image text](https://github.com/demofishChan/neon_optimized/blob/master/img_rotate/2.jpg)

