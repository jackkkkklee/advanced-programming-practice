/*
Author: Congwang Li
Class:  ECE6122
Last Date Modified: 9/20/2022
Description:
used to hold a single puzzle with a constant 9 x 9array of unsigned char elements
 and solve puzzle by backtracking
*/
#include <iostream>
#include "SudokuGrid.h"
#include "string"

using namespace std;


/*!
 *
 * @param mStrGridName set name
 */
void SudokuGrid::setMStrGridName(const string &mStrGridName) {
    m_strGridName = mStrGridName;
}

/*!
 *
 * @param gridElement set GridElement
 */
void SudokuGrid::setGridElement(unsigned char gridElement[9][9]) {
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            SudokuGrid::gridElement[i][j] = gridElement[i][j];
        }
    }
}

/*!
 *
 * @param os
 * @param gridIn
 * @return to read SudokuGrid information from stream
 */
fstream& operator>>(fstream& os, SudokuGrid & gridIn)
{
    string input;
    int i = 0;
    unsigned char gridElement[9][9];
    getline(os,input);
    gridIn.setMStrGridName(input);
    while(i < 9 && getline(os,input))
    {
        for (int j = 0; j < 9; ++j) {
            gridElement[i][j] = input.at(j);
        }
        i++;
    }
    gridIn.setGridElement(gridElement);

    return os;
}

/*!
 *
 * @param os
 * @param gridIn
 * @return to output SudokuGrid information in certain format
 */
fstream& operator<<(fstream& os, const SudokuGrid & gridIn)
{
    os << gridIn.m_strGridName+"\n";
    for (int i = 0; i < 9; ++i)
    {
        for (int j = 0; j < 9; ++j)
        {
            os << gridIn.gridElement[i][j];
        }
        os << endl;
    }
    return os;
}

/*!
 *
 * @param row the row of that number
 * @param num the certain value of certain position in puzzle
 * @return is the value in the row or not
 */
bool SudokuGrid:: isInTheRow(int row ,char num)
{
    for (int col = 0; col < 9; col++)
        if (gridElement[row][col] == num)
        {
            return true;
        }
    return false;
}

/*!
 *
 * @param col the col of that number
 * @param num the certain value of certain position in puzzle
 * @return is the value in the col or not
 */
bool SudokuGrid::isInTheCol(int col , char num)
{
    for (int row = 0; row < 9; row++)
        if (gridElement[row][col] == num)
        {
            return true;
        }
    return false;
}

/*!
 *
 * @param startRow the offset in row [0-2]
 * @param startCol the offset in col [0-2]
 * @param num the certain value of certain position in puzzle
 * @return if the value show in 3x3 box
 */
bool  SudokuGrid::isInTheBox(int startRow,int startCol,char num)
{
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            if (gridElement[row+startRow][col+startCol] == num)
            {
                return true;
            }
        }
    }
    return false;
}

/*!
 *
 * @param row the row of that number
 * @param col the col of that number
 * @param num the certain value of certain position in puzzle
 * @return  if not show in row ,col and box
 */
bool SudokuGrid::isValidate(int row, int col, char num)
{
    return !isInTheRow(row,num) && !isInTheCol(col,num) && !isInTheBox(row-row%3,col - col%3,num);
}

/*!
 * to assign the row and col to next '0'
 * @param row
 * @param col
 * @return if the '0' exist
 */
bool SudokuGrid::findEmptyPlace(int& row, int& col )
{
    for (row = 0; row < 9; row++)
    {
        for (col = 0; col < 9; col++)
        {
            if (gridElement[row][col] == '0')
            {
                return true;
            }
        }
    }
    return false;
}

/*!
 * core function to solve puzzle by using backtracking
 * which means try every possible step.
 * Go back and keep site by vector 'solutions'.
 * Different with the example given in the lab(https://www.tutorialspoint.com/sudoku-solver-in-cplusplus)
 * I implement it by iteration Explicitly.
 */
void SudokuGrid::solve()
{
    int row ;
    int col;
    solution lastSolution = solution{0,0,-1};
    string sizes = to_string(solutions.size());

    while(findEmptyPlace(row,col))
    {
        //check the validation of num in this position
        for (int i = 1; i < 10; ++i) {
            //recover to last solution +1
            if (lastSolution.number != -1)
            {
                i = lastSolution.number;
                lastSolution.number = -1;
                continue;
            }

            if (isValidate(row,col,i+'0'))
            {
                gridElement[row][col] = i+'0';
                solutions.push_back(solution(row,col,i));
                break;
            }
        }
        if(gridElement[row][col] == '0')
        {
            // not valid num available; need to recover and backtracking
            lastSolution = solutions[solutions.size()-1];
            solutions.pop_back();
            gridElement[lastSolution.row][lastSolution.col] = '0';
        }
    }
    solutions.clear();
    return ;
}

