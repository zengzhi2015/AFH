#include "BACKSUB.h"

int main()
{
  String learningFolder("D:\\AFH_demo\\learnFolder\\");
  String segmentationFolder("D:\\AFH_demo\\segmFolder\\");
  String resultsFolder("D:\\AFH_demo\\resultFolder\\");

  BackgroundSubtraction(learningFolder, segmentationFolder, resultsFolder, true);

  return 0;
}


