#include "Functions.h"

void BGR2LuvGxGy(Mat &BGR, Mat &L, Mat &u, Mat &v, Mat &Gx, Mat &Gy) {
  L.create(BGR.rows,BGR.cols, CV_8UC1);
  u.create(BGR.rows,BGR.cols, CV_8UC1);
  v.create(BGR.rows,BGR.cols, CV_8UC1);
  Gx.create(BGR.rows,BGR.cols, CV_8UC1);
  Gy.create(BGR.rows,BGR.cols, CV_8UC1);
  Mat Luv;
  cvtColor(BGR, Luv, CV_BGR2Luv);
  const int max = Luv.rows * Luv.cols;
  uchar *p_Luv;
  uchar *p_L;
  uchar *p_u;
  uchar *p_v;
  p_Luv = Luv.data;
  p_L = L.data;
  p_u = u.data;
  p_v = v.data;
  for(int i = 0; i < max; i++) {
    *p_L = *p_Luv;
    *p_u = (uchar)Thresh((double)(*(p_Luv + 1) - 70) * 2, 0, 255);
    *p_v = (uchar)Thresh(((double)(*(p_Luv + 2)) - 90) * 3, 0, 255);
    p_Luv += 3;
    p_L += 1;
    p_u += 1;
    p_v += 1;
  }
  Mat L_temp;
  L.convertTo(L_temp, CV_16S);
  Mat Gx_temp;
  Mat Gy_temp;
  Sobel(L_temp, Gx_temp, CV_16S, 1, 0, CV_SCHARR);
  Sobel(L_temp, Gy_temp, CV_16S, 0, 1, CV_SCHARR);
  short *p_Gx_temp, *p_Gy_temp;
  uchar *p_Gx, *p_Gy;
  p_Gx_temp = (short*)Gx_temp.data;
  p_Gy_temp = (short*)Gy_temp.data;
  p_Gx = Gx.data;
  p_Gy = Gy.data;
  for(int i = 0; i < max; i++) {
    *p_Gx = Thresh((*p_Gx_temp + (255 << 3)) >> 4, 0, 255);
    *p_Gy = Thresh((*p_Gy_temp + (255 << 3)) >> 4, 0, 255);
    p_Gx_temp += 1;
    p_Gy_temp += 1;
    p_Gx += 1;
    p_Gy += 1;
  }
}

void SeedFill(Mat &Gray, int filterSize, bool removePos) {
  const int H = Gray.rows;
  const int W = Gray.cols;
  const int max = H*W;
  Mat lableImg(H,W,CV_32SC1,Scalar(0));
  Mat tempImg(H,W,CV_8UC1,Scalar(0));
  int fill_color;
  if(removePos) {
    tempImg = Gray > 0;
    tempImg.convertTo(lableImg, CV_32SC1, 0.003921);
    fill_color = 0;
  }
  else {
    tempImg = Gray == 0;
    tempImg.convertTo(lableImg, CV_32SC1, 0.003921);
    fill_color = 255;
  }
  int label = 2;
  int *p_lableImg;
  int Areas[10000];
  int sum;
  for (int i = 0; i < H; i++) {
    for (int j = 0; j < W; j++) {
      p_lableImg = (int*)lableImg.data + i*W + j;
      if (*p_lableImg == 1) {
        stack<pair<int,int>> neighborPixels;
        neighborPixels.push(pair<int,int>(i,j));
        sum = 0;
        while(!neighborPixels.empty()) {
          pair<int,int> curPixel = neighborPixels.top();
          int curY = curPixel.first;
          int curX = curPixel.second;
          *((int*)lableImg.data + curY*W + curX) = label;
          sum += 1;
          neighborPixels.pop() ;
          if(curX==0) { // left bound
            if(curY==0) { // left and up bound
              if(*((int*)lableImg.data + curY*W + curX+1) == 1) {// right pixel
                neighborPixels.push(pair<int,int>(curY, curX+1)) ;
              }
              if(*((int*)lableImg.data + (curY+1)*W + curX) == 1) {// down pixel
                neighborPixels.push(std::pair<int,int>(curY+1, curX)) ;
              }
            }
            if(curY==H-1) { // left and down bound
              if(*((int*)lableImg.data + curY*W + curX+1) == 1) {// right pixel
                neighborPixels.push(pair<int,int>(curY, curX+1)) ;
              }
              if(*((int*)lableImg.data + (curY-1)*W + curX) == 1) {// up pixel
                neighborPixels.push(pair<int,int>(curY-1, curX)) ;
              }
            }
            if((curY<H-1)&(curY>0)) { // left bound only
              if(*((int*)lableImg.data + curY*W + curX+1) == 1) {// right pixel
                neighborPixels.push(pair<int,int>(curY, curX+1)) ;
              }
              if(*((int*)lableImg.data + (curY-1)*W + curX) == 1) {// up pixel
                neighborPixels.push(pair<int,int>(curY-1, curX)) ;
              }
              if(*((int*)lableImg.data + (curY+1)*W + curX) == 1) {// down pixel
                neighborPixels.push(std::pair<int,int>(curY+1, curX)) ;
              }
            }
          }
          if(curX==W-1) { // right bound
            if(curY==0) { // right and up bound
              if(*((int*)lableImg.data + curY*W + curX-1) == 1) {// left pixel
                neighborPixels.push(pair<int,int>(curY, curX-1)) ;
              }
              if(*((int*)lableImg.data + (curY+1)*W + curX) == 1) {// down pixel
                neighborPixels.push(std::pair<int,int>(curY+1, curX)) ;
              }
            }
            if(curY==H-1) { // right and down bound
              if(*((int*)lableImg.data + curY*W + curX-1) == 1) {// left pixel
                neighborPixels.push(pair<int,int>(curY, curX-1)) ;
              }
              if(*((int*)lableImg.data + (curY-1)*W + curX) == 1) {// up pixel
                neighborPixels.push(pair<int,int>(curY-1, curX)) ;
              }
            }
            if((curY<H-1)&(curY>0)) { // right bound only
              if(*((int*)lableImg.data + curY*W + curX-1) == 1) {// left pixel
                neighborPixels.push(pair<int,int>(curY, curX-1)) ;
              }
              if(*((int*)lableImg.data + (curY-1)*W + curX) == 1) {// up pixel
                neighborPixels.push(pair<int,int>(curY-1, curX)) ;
              }
              if(*((int*)lableImg.data + (curY+1)*W + curX) == 1) {// down pixel
                neighborPixels.push(std::pair<int,int>(curY+1, curX)) ;
              }
            }
          }
          if((curX<W-1)&(curX>0)) {
            if(curY==0) { // up bound
              if(*((int*)lableImg.data + curY*W + curX-1) == 1) {// left pixel
                neighborPixels.push(pair<int,int>(curY, curX-1)) ;
              }
              if(*((int*)lableImg.data + curY*W + curX+1) == 1) {// right pixel
                neighborPixels.push(pair<int,int>(curY, curX+1)) ;
              }
              if(*((int*)lableImg.data + (curY+1)*W + curX) == 1) {// down pixel
                neighborPixels.push(std::pair<int,int>(curY+1, curX)) ;
              }
            }
            if(curY==H-1) { // down bound
              if(*((int*)lableImg.data + curY*W + curX-1) == 1) {// left pixel
                neighborPixels.push(pair<int,int>(curY, curX-1)) ;
              }
              if(*((int*)lableImg.data + curY*W + curX+1) == 1) {// right pixel
                neighborPixels.push(pair<int,int>(curY, curX+1)) ;
              }
              if(*((int*)lableImg.data + (curY-1)*W + curX) == 1) {// up pixel
                neighborPixels.push(pair<int,int>(curY-1, curX)) ;
              }
            }
            if((curY<H-1)&(curY>0)) { // no bound
              if(*((int*)lableImg.data + curY*W + curX-1) == 1) {// left pixel
                neighborPixels.push(pair<int,int>(curY, curX-1)) ;
              }
              if(*((int*)lableImg.data + curY*W + curX+1) == 1) {// right pixel
                neighborPixels.push(pair<int,int>(curY, curX+1)) ;
              }
              if(*((int*)lableImg.data + (curY-1)*W + curX) == 1) {// up pixel
                neighborPixels.push(pair<int,int>(curY-1, curX)) ;
              }
              if(*((int*)lableImg.data + (curY+1)*W + curX) == 1) {// down pixel
                neighborPixels.push(std::pair<int,int>(curY+1, curX)) ;
              }
            }
          }
        }
        Areas[label] = sum;
        label += 1;
      }
    }
  }
  uchar *p_Gray;
  p_lableImg = (int*)lableImg.data;
  p_Gray = Gray.data;
  for(int i = 0; i < max; i++) {
    if(*p_lableImg > 0) {
      if(Areas[*p_lableImg] < filterSize) {
        *p_Gray = fill_color;
      }
    }
    p_lableImg += 1;
    p_Gray += 1;
  }
}
// /////////////////////////////////////////////////////////////////////////////////////////////

void PostProc(Mat &Mask) {
  int H = Mask.rows;
  int W = Mask.cols;
  int size = W*H/320;
  medianBlur(Mask, Mask, 5);
  SeedFill(Mask, size, true);
  SeedFill(Mask, size, false);
  medianBlur(Mask, Mask, 3);
}
