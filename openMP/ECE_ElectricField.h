/*
Author: Yash Saraiya
Class: ECE4122 A
Last Date Modified: 10/7/2023
Description: Header file for ECE_ElectricField which is a child of the ECE_PointCharge class.
*/

#include <iostream>
#include "ECE_PointCharge.h"

class ECE_ElectricField: public ECE_PointCharge {
    protected:
        double Ex, Ey, Ez;
    public:
        ECE_ElectricField();
        void computeFieldAt(double x, double y, double z); // computes electric field at x,y,z 
        void getElectricField(double &Ex, double &Ey, double &Ez); // updates these variables and return electric field by that point charge
};