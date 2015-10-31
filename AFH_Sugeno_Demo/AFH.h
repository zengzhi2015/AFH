#ifndef AFH_H_
#define AFH_H_

#include "Functions.h"

class AFH {
private:
  int SoH;
  double SoB;
  double Sum_MAX;
  double MDRate;
  int H;
  int W;
private:
  int cal_N[256];
  double cal_n[256];
  float Sugeno[256][256];
public:
  Mat Img;
  Mat Img_pre;
  Mat Flag;
  Mat Smooth;
  Mat Merge;
  Mat Merge_pre;
private:
  Mat Hist;
  Mat Sum;
  Mat LScore;
  Mat GLBLSM;
  Mat N;
  Mat n;
  Mat score;
  Mat Threshold;
private:
  void HistOpr_Nnscore();
  void HistOpr_Thresh();
  void HistOpr_LScore();
  void HistOpr_GLBLSM();
  void HistOpr_Learn();
public:
  AFH();
  void CreateAFH(Mat frame);
  void Initialization(Mat Gray);
  void Segmentation(Mat Gray);
  void Learning(Mat Gray);
  ~AFH();
};

#endif
