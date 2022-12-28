/*
Author: Congwang Li
Class:  ECE6122
Last Date Modified: 8/11/2022
Description:
    calculate temperature in thin pale by cuda
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <chrono>
/*!
 *
 * @param g a matrix contains new value
 * @param h a matrix contains temperature
 * @param interior size of interior
 */
__global__ void heatCalculation(double * g, double * h, const int n)
{
    //get row based address to
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    const int j = blockIdx.y * blockDim.y + threadIdx.y;
    //make sure extra threads do not do any work
    if (i > 0 && i < n -1  && j > 0 && j < n -1 )
    {
        h[i + j * n] = 0.25 * (g[(i + 1) + j * n] + g[(i - 1) + j * n] + g[i + (j + 1) * n] + g[i + (j - 1) * n]);
    }
}
/*!
 * init temperature
 * @param matrix a matrix to be init by base temperature
 * @param n number of points
 */
void initHeatMatrix(double * matrix, const int n)
{
    //set all interior points to 20
    for (int j = 0; j < n; j++)
    {
        for (int i = 0; i < n; i++)
        {
            matrix[i + n * j] = 20.0;
        }

    }

    //set some top part to 100
    for (int i = 0; i < n; i++)
    {
        if (i > (int)(0.3 * n) && i < (int)(0.7 * n))
        {
            matrix[i] = 100.0;
        }
    }

}
int main(int argc, char* args[]) {
    int n = 0;
    int I = 0;
    int opt;
    //parse args
    while ((opt = getopt(argc, args, "n:I:")) != -1)
    {
        if (opt == 'n')
        {
            n = atoi(optarg) + 2;
        }
        if (opt == 'I')
        {
            I = atoi(optarg);
        }
    }
    if (n <= 0 || I <= 0)
    {
        std::cout << "invalid input!" << std::endl;
        return 0;
    }

    //tow matrix to contain temperature
    double* heatMatrixG;
    double* heatMatrixH;

    //allocate unified memory
    cudaMallocManaged(&heatMatrixG, n * n * sizeof(double));
    cudaMallocManaged(&heatMatrixH, n * n * sizeof(double));

    // set dimBLock and grid size
    dim3 dimBlock(32, 32);
    dim3 dimGrid(ceil((n+31) / (32)) , ceil((n+31) / (32) ));

    //init heat matrix
    initHeatMatrix(heatMatrixH,n);
    initHeatMatrix(heatMatrixG,n);//todo maybe copy could be faster

    //calculate time

    auto startTime = std::chrono::system_clock::now();

    // run I times
    for (int i = 0; i < I; i+=2)
    {
        //change the param order of g and h  to avoid copy
        heatCalculation<<<dimGrid, dimBlock>>>(heatMatrixG, heatMatrixH, n);
        heatCalculation<<<dimGrid, dimBlock>>>(heatMatrixH, heatMatrixG, n);
    }
    //the result matrix determined by I since it is calculated twice in one for cycle
    double* res = heatMatrixG;
    if (I % 2 != 0)
    {
        res = heatMatrixH;
    }

    // Wait for GPU to finish before accessing on host
    cudaDeviceSynchronize();

    auto endTime = std::chrono::system_clock::now();
    printf("Thin plate calculation took %.3f milliseconds",(float )std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()/1000);

    //output to file
    std::ofstream resultFile;
    resultFile.open("finalTemperatures.csv");

    resultFile.precision(4);

    for (int j = 0; j < n; j++)
    {
        for (int i = 0; i < n; i++)
        {
            resultFile << res[i + j * n] << "," << std::fixed;
        }
        resultFile << std::endl;
    }
    resultFile.close();

    //free
    cudaFree(heatMatrixG);
    cudaFree(heatMatrixH);
    return 0;
}
