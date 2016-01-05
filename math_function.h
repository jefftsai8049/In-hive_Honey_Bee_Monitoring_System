#ifndef MATH_FUNCTION_H
#define MATH_FUNCTION_H

#include <stdlib.h>
#include <vector>
#include <math.h>
#include <opencv.hpp>

namespace mf {

float findMax(const float& num1,const float& num2,const float& num3);

void vectorFindMax(double& maxVal,int& index,const double* val);

void vectorFindMin(double& minVal, int& index, const std::vector<double>& val);



}
#endif // MATH_FUNCTION_H
