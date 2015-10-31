#ifndef BACKSUB_H_
#define BACKSUB_H_

#include "AFH.h"

struct pair_args {
  AFH *Model;
  Mat *Gray;
};

int BackgroundSubtraction(String learningFolder, String segmentationFolder, String resultsFolder, bool autoDownscale);
void *thread_initialize(void* args);
void *thread_segmentation(void* args);
void *thread_learning(void* args);

#endif
