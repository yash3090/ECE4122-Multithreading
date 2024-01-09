/*
Author: Yash Saraiya
Class: ECE4122 A
Last Date Modified: 09/27/2023
Description: Header file for ECE_PointCharge.
*/

#include <iostream>

class ECE_PointCharge {
    protected:
        double x, y, z, q;
    public:
        ECE_PointCharge();
        ECE_PointCharge(double, double, double, double);
        void setLocation(double, double,double);
        void setCharge(double);
        double getX() {return this->x;};
        double getY() {return this->y;};
};
