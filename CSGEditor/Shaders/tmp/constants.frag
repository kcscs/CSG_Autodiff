#version 460
#define DERIVATIVE_ORDER 1
#define DERIVATIVES_ENABLED
const int tetra[3] = {
    0,1,4
};
const int tri[3] = {
    0,1,3
};
const int choose[2][2] = {
    {1,0},
    {1,1},
};
#define SIZE 4
