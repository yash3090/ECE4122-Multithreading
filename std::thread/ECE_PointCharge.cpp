/*
Author: Yash Saraiya
Class: ECE4122 A
Last Date Modified: 09/27/2023
Description: Implementation of the ECE_PointCharge class methods.
*/

#include <iostream>
#include "ECE_PointCharge.h"

ECE_PointCharge::ECE_PointCharge() {
    this->x = 0;
    this->y = 0;
    this->z = 0;
    this->q = 0;
}

ECE_PointCharge::ECE_PointCharge(double x, double y , double z, double q) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->q = q;
}
void ECE_PointCharge::setLocation(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
}
void ECE_PointCharge::setCharge(double q) {
    this->q = q;
}
