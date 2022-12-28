/*
Author: Congwang Li
Class:  ECE6122
Last Date Modified: 26/11/2022
Description:
    calculate temperature in thin pale by mpi
*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <mpi.h>

/*!
 *
 * @param g a matrix contains new value
 * @param h a matrix contains temperature
 * @param interior size of interior
 */
void heatCalculation(double * g, double * h, const int c , const int r, int rank, int numTask)
{

    //make sure extra threads do not do any work

    for (int j = 0; j < r; j++)
    {
        for (int i = 0; i < c; i++)
        {
            if (i > 0 && i < c -1  && j > 0 && j < r -1 )
            {
                //filter no.head+1  or tail-1 line when rank is 0 or last one
                if((rank == 0 && j == 1 )|| (j == r-2 && rank ==  numTask-1))
                {
                    continue;
                }
                h[i + j * c] = 0.25 * (g[(i + 1) + j * c] + g[(i - 1) + j * c] + g[i + (j + 1) * c] + g[i + (j - 1) * c]);
            }
        }

    }

}

/*!
 *
 * @param matrix
 * @param r
 * @param c
 */
void initHeatMatrix(double * matrix, const int r ,const int c , int rank)
{
    //set all interior points to 20
    for (int j = 0; j < r; j++)//3,2 = 3*c +2  i=3 j=2
    {
        for (int i = 0; i < c; i++)
        {
            matrix[i + c * j] = 20.0;
        }

    }

    //set some top part to 100 only if rank == 0
    if (rank == 0)
    {
        for (int i = 0; i < c; i++)
        {
            if (i > (int)(0.3 * c) && i < (int)(0.7 * c))
            {
                matrix[i+c] = 100.0;
            }
        }
    }
}

/*!
 * add dummy line if it is not exact division
 * @param rank
 * @param numTask
 * @param n grid size
 * @return total row number for this rank
 */
int getLocalRowNum(int rank, int numTask, int n)
{
    int row;

    if (n%numTask !=0)
    {
        row = n / numTask +1;
    }
    else
    {
        row = n / numTask;
    }

    return row;
}

/*!
 * add dummy data(head and tail) to each matrix
 * @param n
 * @param sendBuffer
 * @param recBuffer
 * @param heatMatrixH
 * @param preRank
 * @param nextRank
 * @param tag
 * @param endRow
 * @param status
 */
void prepareData(int n, double* sendBuffer, double * recBuffer,double* heatMatrixH, int preRank, int nextRank, int tag,int endRow ,MPI_Status status) {
    for (int k = 0; k < n; k++) {
        sendBuffer[k] = heatMatrixH[k + n];
    }
    MPI_Sendrecv(sendBuffer, n, MPI_DOUBLE, preRank, tag,
                 recBuffer, n, MPI_DOUBLE, nextRank, tag,
                 MPI_COMM_WORLD, &status);
    for (int k = 0; k < n; k++) {
        heatMatrixH[k + n * (endRow - 1)] = recBuffer[k];
    }

    //receive and as headline from pre rank , send tail to next rank
    for (int k = 0; k < n; k++) {
        sendBuffer[k] = heatMatrixH[k + n * (endRow - 2)];
    }
    MPI_Sendrecv(sendBuffer, n, MPI_DOUBLE, nextRank, tag,
                 recBuffer, n, MPI_DOUBLE, preRank, tag,
                 MPI_COMM_WORLD, &status);
    for (int k = 0; k < n; k++) {
        heatMatrixH[k] = recBuffer[k];
    }
}


void outPutToFile(int n,int r ,double* res,int* line,int base)
{

    //output to file
    std::ofstream resultFile;
    resultFile.open("finalTemperatures.csv",std::ofstream::out|std::ofstream::ate|std::ofstream::app);
    resultFile.precision(4);
    for (int j = base+ 1; j < base+ r-1; j++)
    {
        (*line)++;
        if(*line > n)
        {
            return;
        }
        for (int i = 0; i < n; i++)
        {

            resultFile << res[i + j * n] << "," << std::fixed;
        }
        resultFile << std::endl;

    }
    resultFile.close();
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

    int master = 0;
    int rank;
    int numTask;
    int tag = 0;

    MPI_Status status;
    MPI_Init(&argc, &args);
    MPI_Comm_size(MPI_COMM_WORLD, &numTask);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //add some dummy data
    int endRow = getLocalRowNum(rank, numTask, n)+2;


    //the top and tail rank send to each other ,just keep it simple but not use that data.
    int preRank = rank == 0 ? numTask -1 : rank -1;
    int nextRank = rank == numTask-1 ? 0: rank + 1;


    //buffer to send data and get data
    double* sendBuffer = (double*)malloc(sizeof(double) * n);
    double* recBuffer = (double*)malloc(sizeof(double) * n);

    //matrix to contain temperature
    double* heatMatrixG;
    double* heatMatrixH;

    //allocate memory
    heatMatrixG = (double*)malloc(sizeof(double) * endRow * n);
    heatMatrixH = (double*)malloc(sizeof(double) * endRow * n);

    //init heat matrix
    initHeatMatrix(heatMatrixH,endRow,n,rank);
    initHeatMatrix(heatMatrixG,endRow,n,rank);

    //calculate time
    auto startTime = std::chrono::system_clock::now();

    // run I times
    for (int i = 0; i < I; i+=2)
    {

        prepareData(n,  sendBuffer, recBuffer,heatMatrixG, preRank,  nextRank,  tag, endRow ,status);

        //change the param order of g and h  to avoid copy
        heatCalculation(heatMatrixG, heatMatrixH,n,endRow,rank,numTask);
        //copy(heatMatrixH, heatMatrixG,n,endRow);

        prepareData(n,  sendBuffer, recBuffer,heatMatrixH, preRank,  nextRank,  tag, endRow ,status);

        heatCalculation(heatMatrixH, heatMatrixG,n,endRow,rank,numTask);
    }
    //the result matrix determined by I since it is calculated twice in one for cycle
    double* res = heatMatrixG;
    if (I % 2 != 0)
    {
        res = heatMatrixH;
    }

    auto endTime = std::chrono::system_clock::now();

    if(rank == 0)
    {
        printf("Thin plate calculation took %.3f milliseconds",(float )std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()/1000);
    }

    double* gatherMatrix = (double*)malloc(sizeof(double) * endRow * n *numTask);

    //gather matrix and output
    MPI_Gather(res ,endRow * n,MPI_DOUBLE,gatherMatrix, endRow * n ,MPI_DOUBLE,master,MPI_COMM_WORLD);

    //output
    if(rank == 0 )
    {
        int line = 0;
        for (int k = 0 ; k < numTask; k++)
        {
            outPutToFile(n,endRow,gatherMatrix,&line,k*endRow);
        }

    }

    //free
    free(heatMatrixG);
    free(heatMatrixH);
    free(sendBuffer);
    free(recBuffer);
    free(gatherMatrix);
    MPI_Finalize();

    return 0;
}
