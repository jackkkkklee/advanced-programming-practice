/*
Author: Congwang Li
Class:  ECE6122
Last Date Modified: 9/15/2022
Description:
define the  member ,method of SudokuGrid class
*/
#include "string"
#include <fstream>
#include <vector>
using namespace std;
/*!
*  a step of solve puzzle
*/
struct solution
{
    int row;
    int col;
    int number;
    solution(int row,int col, int number)
    {
        this->row = row;
        this->col = col;
        this->number = number;
    }
};

class SudokuGrid
{

    unsigned char gridElement[9][9];
    vector<solution> solutions;
public:
    string m_strGridName;
    void setMStrGridName(const string &mStrGridName);
    void setGridElement(unsigned char gridElement[9][9]);
    //reads a single SudokuGrid object from a fstream file
    friend fstream& operator>>(fstream& os, SudokuGrid & gridIn);
    //writes the SudokuGrid object to a file in the same format that is used in reading in the object
    friend fstream& operator<<(fstream& os, const SudokuGrid & gridIn);
    void solve();
private:
    bool isInTheRow(int row ,char num);
    bool isInTheCol(int col , char num);
    bool isInTheBox(int startRow,int startCol,char num);
    bool isValidate(int startRow, int startCol, char num);
    bool findEmptyPlace(int& row, int& col );
};

