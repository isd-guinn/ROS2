#include <iostream>

#define JOY_MAX 32767
#define V_MAX 18.0f

void joy2theworld(float x, float y, float& left_v, float& right_v, const int x_o = 0, const int y_o = 0)
{
    y = -y;
    // remap values to biased values
    x = (x>x_o) ? (x-x_o)/(JOY_MAX-x_o)*JOY_MAX : (x_o-x)/(JOY_MAX+x_o)*(-JOY_MAX);
    y = (y>y_o) ? (y-y_o)/(JOY_MAX-y_o)*JOY_MAX : (y_o-y)/(JOY_MAX+y_o)*(-JOY_MAX);

    // std::cout << "Biased: " << x << " " << y << std::endl;  

    // normalize value to voltage
    left_v = (y+x)/JOY_MAX*V_MAX;
    right_v = (y-x)/JOY_MAX*V_MAX;

    // std::cout << "Voltage: " << left_v << " " << right_v << std::endl;

    // constraint the value
    left_v = (left_v > V_MAX) ? V_MAX : (left_v < -V_MAX) ? -V_MAX : left_v;
    right_v = (right_v > V_MAX) ? V_MAX : (right_v < -V_MAX) ? -V_MAX : right_v;
}