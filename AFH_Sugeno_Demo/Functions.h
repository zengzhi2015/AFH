#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "CommonLibs.h"

#define Abs(a) (((a) >= 0) ? (a) : -(a))
#define Diff(a, b) (((a) > (b)) ? (a - b) : (b - a))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Norm(a, b, c) sqrt((a)*(a) + (b)*(b) + (c)*(c))
#define Norm2(a, b, c) ((a)*(a) + (b)*(b) + (c)*(c))
#define Square(a) ((a) * (a))
#define Sign(a) (((a) >= 0) ? 1 : -1)
#define Thresh(x, low, up) Min(Max(x, low), up)

void SeedFill(Mat &Gray, int filterSize, bool removePos);
void BGR2LuvGxGy(Mat &RGB, Mat &L, Mat &u, Mat &v, Mat &Gx, Mat &Gy);
void PostProc(Mat &Mask);

#endif
