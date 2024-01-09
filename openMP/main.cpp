/*
Author: Yash Saraiya
Class: ECE4122 A
Last Date Modified: 10/7/2023
Description: Main file runs the main program. The program takes in user input and validifies it. 
Creates electirc field array and then calls for loop which is parallelized using openMP to do calculations on the net electric field caused by different parts of the electricField array.
The program then outputs the resultant electric field at the user input point.
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
double Ex; // size is equal to number threadj
double Ey;
double Ez;
int numThreads;
double x, y, z, charge;
int rows, cols;

/*
Main program handles user input set-up and multithreading to determine the output.
*/
int main() {
    double xSeperation, ySeperation;
    std::string stringTemp;
    int n = std::thread::hardware_concurrency();
    double lowestX;
    double highestY;
    bool runProgram = true;

    while(runProgram) {
        Ex = 0;
        Ey = 0;
        Ez = 0;

        std::cout << "\nPlease enter the number of concurrent threads to use: ";
        std::getline(std::cin, stringTemp);
        std::istringstream iss(stringTemp);
        if (iss >> numThreads && iss.eof()) {
            if (numThreads <= 0 || numThreads > n){
                //invalid case
                std::cout << "Invalid number of threads" << std::endl;
                continue;
            }
        } else {
            std::cout << "Invalid number of threads" << std::endl;
            continue;
        }

        
        std::cout << "Please enter the number of rows and columns in the N x M array: ";
        std::getline(std::cin, stringTemp);
        iss.clear();
        iss.str(stringTemp);
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
                if (abs(temp-x) < 0.0001 && abs(y-highestY)<0.0001) { // had to do this as c++ double comparism error prone sometimes
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
        // start clock
        auto start = std::chrono::high_resolution_clock::now();
        
        int j;
        double tempEx, tempEy, tempEz;
        // have different form scheduling
        #pragma omp parallel for num_threads(numThreads) private(j, tempEx, tempEy, tempEz) reduction(+:Ex,Ey,Ez)
        for (int i = 0; i < rows; ++i) { // change row/col to desired
            for (int j = 0; j < cols; ++j) {
                vecElectric[i][j].computeFieldAt(x, y, z);
                vecElectric[i][j].getElectricField(tempEx, tempEy, tempEz);
                Ex += tempEx;
                Ey += tempEy;
                Ez += tempEz;
            }
        }
        // end clock
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end-start;
        // Save the original formatting settings
        std::ios_base::fmtflags originalFlags = std::cout.flags();
        std::streamsize originalPrecision = std::cout.precision();
        std::cout << "The electric field at (" << x << ", " << y << ", " << z << ") in V/m is" << std::endl;
        std::cout << "Ex = " << std::scientific << std::setprecision(4) << Ex << std::endl;
        std::cout << "Ey = "  << std::scientific << std::setprecision(4) << Ey << std::endl;
        std::cout << "Ez = " << std::scientific << std::setprecision(4) << Ez << std::endl;
        double totalEnergy = sqrt(Ex*Ex + Ey*Ey + Ez*Ez);
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
    std::cout << "Bye!" << std::endl;
    return 0;
}