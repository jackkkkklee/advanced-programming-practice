/*
Author: Congwang Li
Class:  ECE6122
Last Date Modified: 9/20/2022
Description:
control the input and output of puzzle and use multiple thread to solve puzzle
*/
#include <mutex>
#include <fstream>
#include <iostream>
#include <functional>
#include "SudokuGrid.h"
#include <vector>
#include <thread>

using namespace std;
mutex outFileMutex;
mutex inFileMutex;
/*
 * In the main() function, open the output file Lab2Prob2.txt using the
command line argument. Use the same format for the output file as in the
input file
 */
fstream outFile;
/*
 * In the main() function open the file using the command line argument
 */
fstream inFile;

void solveSudokuPuzzles();

/*!
 *
 * @param argc
 * @param args the file path of input file
 * @return
 */
int main(int argc, char* args[]) {
    if( argc == 2 ) {
        string fileDir = args[1];
        try
        {
            inFile.open (fileDir);
            bool isOpen = inFile.is_open();
            if (!isOpen)
            {
                throw exception();
            }
        }
        catch(exception &e)
        {
            printf("fail to open file %s for reading",fileDir.c_str());
            inFile.close();
            return 0;
        }
        try
        {
            outFile.open("Lab2Prob2.txt",ios::out);
            bool isOpen = outFile.is_open();
            if (!isOpen)
            {
                throw exception();
            }
        }
        catch(exception &e)
        {
            printf("fail to open file Lab2Prob2.txt for writing");
            inFile.close();
            outFile.close();
            return 0;
        }

        //get max thread dynamically
        unsigned int numOfThreads = thread::hardware_concurrency() -1;
        thread array[numOfThreads];
        for (int i = 0; i < numOfThreads; ++i) {
            array[i] = thread (solveSudokuPuzzles);
        }
        //join thread
        for (int i = 0; i < numOfThreads; ++i) {
            array[i].join();
        }
        //close stream
        inFile.close();
        outFile.close();
    }
    else
    {
        std::cout << "Invalid Input!" << std::endl;
    }

    return 0;
}

/*!
 *  solveSudokuPuzzles in multiple threads by lock
 */
void solveSudokuPuzzles() {
    SudokuGrid obj =  SudokuGrid();
    SudokuGrid* sudokuGrid = &obj;
    do
    {
        inFileMutex.lock();
        // do reading from file and double check stream
        if(inFile.good())
        {
            inFile >> *sudokuGrid ;
            inFileMutex.unlock();
        }
        else
        {
            inFileMutex.unlock();
            break;
        }
        sudokuGrid->solve();
        outFileMutex.lock();
        outFile << *sudokuGrid;
        outFileMutex.unlock();
    }while(inFile.good());
}