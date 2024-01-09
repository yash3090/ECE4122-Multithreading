/*
Author: Yash Saraiya
Class: ECE4122 A
Last Date Modified: 09/27/2023
Description: Implementation of the ECE_ElectricField class methods.
*/

#include <iostream>
#include <cmath>
#include "ECE_ElectricField.h"

ECE_ElectricField::ECE_ElectricField() {
    this->Ex = 0;
    this->Ey = 0;
    this->Ez = 0;
}

void ECE_ElectricField::getElectricField(double &Ex, double &Ey, double &Ez) {
    Ex = this->Ex;
    Ey = this->Ey;
    Ez = this->Ez;
}

void ECE_ElectricField::computeFieldAt(double x, double y, double z) {
    double xDiff = x-this->x;
    double yDiff = y-this->y;
    double zDiff = z-this->z;
    double rDist = sqrt(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff);
    this->Ex = (900*this->q*xDiff)/(rDist*rDist*rDist);
    this->Ey = (900*this->q*yDiff)/(rDist*rDist*rDist);
    this->Ez = (900*this->q*zDiff)/(rDist*rDist*rDist);
}
