/*
Author: Yash Saraiya
Class: ECE4122 A
Last Date Modified: 09/27/2023
Description: Main file runs the main program. The program takes in user input and validifies it. 
Creates electirc field array and then calls multiple threads to do calculations on the net electricfield caused by different parts of the electricField array.
The program then waits for synchronization of all the threads and outputs the resultant electric field at the user input point.
*/
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>
#include <condition_variable>
#include <atomic>
#include <iomanip>
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include "ECE_ElectricField.h"

std::vector<std::vector<ECE_ElectricField>> vecElectric;
std::vector<double> vecEx; // size is equal to number threadj
std::vector<double> vecEy;
std::vector<double> vecEz;
enum enStatus {eWaiting, eRun, eFinished, eExit}; // enums for threads status, so dont have to tear down create new threads, can reuse
std::atomic<int> threadsStatus[50];
int numThreads;
int lastRows[50];

double x, y, z, charge;
int rows, cols;

/*
@param int id. The particular threads id.
Calculates the resultant electric field at the user input point caused by the charges in the rows the particular thread is in charge of. 
*/
void CalculateElectricField(int id) {
    double tempEx(0.0), tempEy(0.0), tempEz(0.0);
    double sumEx(0.0), sumEy(0.0), sumEz(0.0);
    while (true) {
        // Wait until signaled to start
        while (threadsStatus[id] == eWaiting) {
            std::this_thread::yield();
        }
        // Do calculation for the row/col we want
        for (int i = lastRows[id]; i < lastRows[id+1]; ++i) { // change row/col to desired
            for (int j = 0; j < cols; ++j) { //change col to desired
                vecElectric[i][j].computeFieldAt(x, y, z);
                vecElectric[i][j].getElectricField(tempEx, tempEy, tempEz);
                sumEx += tempEx;
                sumEy += tempEy;
                sumEz += tempEz;
            }
        }
        vecEx[id] = sumEx;
        vecEy[id] = sumEy;
        vecEz[id] = sumEz;
        //change thread status to finished
        threadsStatus[id] = eFinished;
        
        // Wait until signaled what to do next
        while (threadsStatus[id] == eFinished) {
            std::this_thread::yield();
        }
        if (threadsStatus[id] == eExit) {
            break;
        }
    }   
}

/*
Main program handles user input set-up and multithreading to determine the output.
*/
int main() {
    double xSeperation, ySeperation;
    std::string stringTemp;
    int n = std::thread::hardware_concurrency();
    double lowestX;
    double highestY;
    bool AllFinished;
    std::vector<std::thread> threads;
    bool runProgram = true;

    while(runProgram) {
        std::cout << "\nYour computer supports "<< n <<" concurrent threads.\n";
        std::cout << "Please enter the number of rows and columns in the N x M array: ";
        std::getline(std::cin, stringTemp);
        std::istringstream iss(stringTemp);
        if (iss >> rows >> cols && iss.eof()) {
            if (rows <= 0 || cols <= 0){
                //invalid case
                std::cout << "Invalid M and N" << std::endl;
                continue;
            }
        } else {
            std::cout << "Invalid M and N" << std::endl;
            continue;
        }
        std::cout << "Please enter the x and y separation distances in meters: ";
        std::getline(std::cin, stringTemp);
        iss.clear();
        iss.str(stringTemp);
        if (iss >> xSeperation >> ySeperation && iss.eof()) {
            if (xSeperation <= 0 || ySeperation <= 0){
                //invalid case
                std::cout << "Invalid x and seperation" << std::endl;
                continue;
            }
        } else {
            std::cout << "Invalid x and seperation" << std::endl;
            continue;
        }
        std::cout << "Please enter the common charge on the points in micro C: "; 
        iss.clear();
        std::getline(std::cin, stringTemp);
        iss.str(stringTemp);
        if (iss >> charge && iss.eof()) {
        } else {
            std::cout << "Invalid charge" << std::endl;
            continue;
        }
        // check if valid input charge
        std::cout << "Please enter the location in space to determine the electric field (x y z) in meters: ";
        iss.clear();
        std::getline(std::cin, stringTemp);
        iss.str(stringTemp);
        if (iss >> x >> y >> z && iss.eof()) {
        } else {
            std::cout << "Invalid location entered" << std::endl;
            continue;
        }
        
        // Create a 2D vector with the given dimensions
        vecElectric.assign(rows, std::vector<ECE_ElectricField>(cols));

        // determining starting x and y coordinates to fill in location for ElectricField vector
        if (rows & 0b1) { // #rows is odd
            highestY = (rows/2) * ySeperation;
        } else {
            highestY = (((rows/2)-1)*ySeperation + ySeperation/2.0);
        }
        if (cols & 0b1) { // #cols is odd
            lowestX = -(cols/2) * xSeperation;
        } else {
            lowestX = -(((cols/2)-1)*xSeperation + xSeperation/2.0);
        }

        bool conflict = false;
        for (int i = 0; i < rows; i++) {
            double temp = lowestX;
            for (int j = 0; j < cols; j++) {
                if (abs(temp-x) < 0.00001 && abs(y-highestY)<0.00001) { // had to do this as c++ double comparism error prone sometimes
                    conflict = true;
                }
                vecElectric[i][j].setCharge(charge);
                vecElectric[i][j].setLocation(temp, highestY, 0);
                temp += xSeperation;
            }
            highestY -= ySeperation;
        }
        if(conflict && z==0) {
            std::cout << "Invalid location entered" << std::endl;
            continue;
        }

        AllFinished = false;
        for (int i = 0; i < n+2; ++i) {
            threadsStatus[i] = eWaiting;;
        }
        // determining the number of threads that should be used to do the calculation.
        numThreads = 10;
        if (cols < 10) {
            numThreads = std::min(rows/2, n-1);
        } else {
            numThreads = std::min(rows, n-1);
        }
        if (numThreads == 0) {
            numThreads = 1;
        }
        vecEx.assign(numThreads, 0);
        vecEy.assign(numThreads, 0);
        vecEz.assign(numThreads, 0);
        
        // allocating the number of rows each thread calculates the resultant electric field for
        int rowsPerThread = rows / numThreads;
        int remainder = rows % numThreads;

        for (int i = 0; i < numThreads; ++i) {
            if (i < remainder) {
                lastRows[i+1] = (i+1) * (rowsPerThread + 1);
            } else {
                lastRows[i+1] = ((i+1) * rowsPerThread) + remainder;
            }
        }
        lastRows[0] = 0;

        // start clock
        auto start = std::chrono::high_resolution_clock::now();
        // assign every thread its id
        for (int i = 0; i < numThreads; ++i) {
            threads.push_back(std::thread(CalculateElectricField, i));
        }
        // change status of all threads to running
        for (int j = 0; j < numThreads; ++j) {
                threadsStatus[j] = eRun;
        }
        // whie loop to check if all the threads calculations have finished their calculations
        do {
            AllFinished = true;
            for (int j = 0; j < numThreads; ++j) {
                if (threadsStatus[j] != eFinished) {
                    AllFinished = false;
                    break;
                }
            }
            std::this_thread::yield();
        } while (!AllFinished);

        double resultEx = 0.0;
        double resultEy = 0.0;
        double resultEz = 0.0;
        for (auto ex: vecEx) {
            resultEx += ex;
        }
        for (auto ey: vecEy) {
            resultEy += ey;
        }
        for (auto ez: vecEz) {
            resultEz += ez;
        }
        
        // end clock
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end-start;
        // Save the original formatting settings
        std::ios_base::fmtflags originalFlags = std::cout.flags();
        std::streamsize originalPrecision = std::cout.precision();
        std::cout << "The electric field at (" << x << ", " << y << ", " << z << ") in V/m is" << std::endl;
        std::cout << "Ex = " << std::scientific << std::setprecision(4) << resultEx << std::endl;
        std::cout << "Ey = "  << std::scientific << std::setprecision(4) << resultEy << std::endl;
        std::cout << "Ez = " << std::scientific << std::setprecision(4) << resultEz << std::endl;
        double totalEnergy = sqrt(resultEx*resultEx + resultEy*resultEy + resultEz*resultEz);
        std::cout << "|E| = " << std::scientific << std::setprecision(4) << totalEnergy << std::endl;
        // Restore the original formatting settings
        std::cout.flags(originalFlags);
        std::cout.precision(originalPrecision);
        std::cout << "The calculation took " << duration.count()*1000000 << " microsec!" <<std::endl;
        std::cout << "Do you want to enter a new location (Y/N)? ";
        iss.clear();
        std::getline(std::cin, stringTemp);
        iss.str(stringTemp);
        std::string cont;
        // check is user input it Y if not then terminate program.
        if (iss >> cont && iss.eof()) {
            if (cont != "Y") {
                runProgram = false;
                break;
            }
        } else {
            runProgram = false;
            break;
        }
    }
    //after program terminates exit threads
    for (int j = 0; j < numThreads; ++j) {
        threadsStatus[j] = eExit;
    }
    //join all the threads
    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "Bye!" << std::endl;
    return 0;
}