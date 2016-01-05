#include "math_function.h"

#define BIG_NUMBER 1.79e308

float mf::findMax(const float &num1, const float &num2, const float &num3)
{
    float max;
    if(num1>num2)
        max = num1;
    else
        max = num2;

    if(max<num3)
        max = num3;

    return max;
}



void mf::vectorFindMax(double &maxVal, int &index, const double *val)
{
    maxVal = 0;
    for(int i = 0;i<sizeof(val)/sizeof(double);i++)
    {
        if(maxVal < val[i])
        {
            index = i;
            maxVal = val[i];
        }
    }
}


void mf::vectorFindMin(double &minVal, int &index, const std::vector<double> &val)
{
    minVal = BIG_NUMBER;
    for(int i = 0;i<val.size();i++)
    {
        if(minVal > val[i])
        {
            index = i;
            minVal = val[i];
        }
    }
}
