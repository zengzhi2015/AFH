#include "AFH.h"

AFH::AFH() {
  ;
}

void AFH::CreateAFH(Mat frame) {
  H = frame.rows;
  W = frame.cols;
  SoH = 20;
  SoB = 256/(double)SoH;
  Sum_MAX = 32;
  MDRate = 0.05;
  for(int i = 0; i<256; i++) {
    cal_N[i] = (int)floor((double)i/SoB);
    cal_n[i] = ((double)i - SoB * (double)cal_N[i])/SoB;
  }

  for(int i = 0; i < 256; i++) {
    for(int j = 0; j < 256; j++) {
      double x, y;
      x = 1-(double)i/255.0;
      y = 1-(double)j/255.0;
      if((x<=0.5)&(y<=0.5)) {
        Sugeno[i][j] = 0.02*(1-2*x)*2*y;
      }
      if((x>=0.5)&(y<=0.5)) {
        Sugeno[i][j] = 0;
      }
      if((x<=0.5)&(y>=0.5)) {
        Sugeno[i][j] = 0.02*(1-2*x)*(2-2*y) + 0.5*(1-2*x)*(2*y-1) + 0.1*2*x*(2*y-1);
      }
      if((x>=0.5)&(y>=0.5)) {
        Sugeno[i][j] = 0.1*(2-2*x)*(2*y-1) + 0.1*(2*x-1)*(2*y-1);
      }
    }
  }

  Img.create(H,W,CV_8UC1);
  Img_pre.create(H,W,CV_8UC1);
  Flag.create(H,W,CV_8UC1);
  Smooth.create(H,W,CV_8UC1);
  Merge.create(H,W,CV_8UC1);
  Merge_pre.create(H,W,CV_8UC1);

  const int sz[] = {H,W,SoH+1};
  Hist.create(3,sz,CV_64FC1);
  Sum.create(H,W,CV_64FC1);
  LScore.create(H,W,CV_64FC1);
  GLBLSM.create(H,W,CV_64FC1);
  N.create(H,W,CV_32SC1);
  n.create(H,W,CV_64FC1);
  score.create(H,W,CV_64FC1);
  Threshold.create(H,W,CV_64FC1);
}

void AFH::HistOpr_Nnscore() {
  uchar *p_image = Img.data;
  int *p_N = (int*)N.data;
  double *p_n = (double*)n.data;
  double *p_Hist = (double*)Hist.data;
  double *p_score = (double*)score.data;
  for(int i = 0; i<H; i++) {
    for(int j = 0; j<W; j++) {
      *p_N = cal_N[*p_image];
      *p_n = cal_n[*p_image];
      *p_score = *(p_Hist + *p_N) * (1 - *p_n) + *(p_Hist + *p_N + 1) * (*p_n);
      p_image += 1;
      p_N += 1;
      p_n += 1;
      p_Hist += SoH + 1;
      p_score += 1;
    }
  }
}

void AFH::HistOpr_Thresh() {
  double *p_Hist = (double*)Hist.data;
  double *p_Sum = (double*)Sum.data;
  double *p_Thresh = (double*)Threshold.data;
  double *p_score = (double*)score.data;
  uchar *p_Flag = Flag.data;

  double temp_array[SoH+1];
  double temp_sum;
  double temp_max;

  for(int i = 0; i<H; i++) {
    for(int j = 0; j<W; j++) {

      // Original version
      /*
      for(int k = 0; k<=SoH; k++) { // 拷贝出直方图
        temp_array[k] = *(p_Hist + k);
      }
      sort(temp_array,temp_array + SoH + 1); // 直方图排序
      temp_max = (*p_Sum) * MDRate;
      temp_sum = 0;
      *p_Thresh = 0;
      for(int k = 0; k<=SoH; k++) { // 求取阈值
        temp_sum += temp_array[k];
        if(temp_max < temp_sum) {
          if(k>0) {
            *p_Thresh = temp_array[k] - (temp_sum-temp_max)*(temp_array[k]-temp_array[k-1])/temp_array[k];
          }
          else {
            *p_Thresh = temp_array[k] - (temp_sum-temp_max);
          }
          break;
        }
      }
      */

      // The original version can be approximated and greatly accelerated as follows
      double max_hist = 0;
      for(int k = 0; k <= SoH; k++) {
        if(*(p_Hist + k)>=max_hist) {
          max_hist = *(p_Hist + k);
        }
      }
      *p_Thresh = max_hist*0.054;

      *p_Flag = (uchar)Thresh(255*(1.5*(*p_Thresh)-*p_score)/(*p_Thresh), 0, 255);

      p_Hist += SoH + 1;
      p_Sum += 1;
      p_Thresh += 1;
      p_score += 1;
      p_Flag += 1;
    }
  }
}

void AFH::HistOpr_LScore() {
  uchar *p_Flag = Flag.data;
  uchar *p_Merge = Merge.data;
  double *p_Lscore = (double*)LScore.data;

  for(int i = 0; i<H; i++) {
    for(int j = 0; j<W; j++) {
      *p_Lscore = Sugeno[*p_Flag][*p_Merge];
      p_Flag += 1;
      p_Merge += 1;
      p_Lscore += 1;
    }
  }
}

void AFH::HistOpr_GLBLSM() {
  Mat CM, I_M, Ip_M;
  int mc;
  Scalar s1, s2;
  double diff_ave;
  CM = (Merge == 0) & (Merge_pre == 0);
  erode(CM,CM,Mat(),Point(-1,-1),3);
  I_M = Img & CM;
  Ip_M = Img_pre & CM;
  s1 = sum(I_M);
  s2 = sum(Ip_M);
  mc = countNonZero(CM);
  diff_ave = abs(s1[0]-s2[0])/(double)mc;
  CM.convertTo(GLBLSM, CV_64FC1, diff_ave/255);
  LScore = LScore + GLBLSM;
}

void AFH::HistOpr_Learn() {
  int *p_N = (int*)N.data;
  double *p_n = (double*)n.data;
  double *p_Hist = (double*)Hist.data;
  double *p_LScore = (double*)LScore.data;
  double *p_Sum = (double*)Sum.data;

  for(int i = 0; i<H; i++) {
    for(int j = 0; j<W; j++) {
      *(p_Hist + *p_N) += (1 - *p_n) * (*p_LScore);
      *(p_Hist + *p_N + 1) +=  (*p_n) * (*p_LScore);
      *p_Sum += *p_LScore;

      if(*p_Sum > Sum_MAX) {
        for(int s = 0; s <= SoH; s++) {
          *(p_Hist + s) *= 0.75;
        }
        *p_Sum *= 0.75;
      }

      p_N += 1;
      p_n += 1;
      p_Hist += SoH + 1;
      p_LScore += 1;
      p_Sum += 1;
    }
  }
}

// ///////////////////////////////////////////////////////////////

void AFH::Initialization(Mat Gray) {
  Img.copyTo(Img_pre);
  Gray.copyTo(Img);
  HistOpr_Nnscore();
  LScore.setTo(1);
  HistOpr_Learn();
}

void AFH::Segmentation(Mat Gray) {
  Img.copyTo(Img_pre);
  Gray.copyTo(Img);
  HistOpr_Nnscore();
  HistOpr_Thresh();
  medianBlur(Flag, Smooth, 5);
  SeedFill(Smooth, Smooth.cols*Smooth.rows/2000, true);
}

void AFH::Learning(Mat Gray) {
  Merge.copyTo(Merge_pre);
  Gray.copyTo(Merge);
  HistOpr_LScore();
  HistOpr_GLBLSM();
  HistOpr_Learn();
}

AFH::~AFH() {
  ;
}
