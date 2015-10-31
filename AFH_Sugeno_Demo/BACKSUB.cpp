#include "BACKSUB.h"

void *thread_initialize(void* args) {
  struct pair_args *my_data;
  my_data = (struct pair_args *) args;
  AFH *My_Model = my_data->Model;
  Mat *My_Gray = my_data->Gray;
  My_Model->Initialization(*My_Gray);
  bool status = true;
  pthread_exit((void*) status);
  return NULL;
}

void *thread_segmentation(void* args) {
  struct pair_args *my_data;
  my_data = (struct pair_args *) args;
  AFH *My_Model = my_data->Model;
  Mat *My_Gray = my_data->Gray;
  My_Model->Segmentation(*My_Gray);
  bool status = true;
  pthread_exit((void*) status);
  return NULL;
}

void *thread_learning(void* args) {
  struct pair_args *my_data;
  my_data = (struct pair_args *) args;
  AFH *My_Model = my_data->Model;
  Mat *My_Gray = my_data->Gray;
  My_Model->Learning(*My_Gray);
  bool status = true;
  pthread_exit((void*) status);
  return NULL;
}

int BackgroundSubtraction(String learningFolder, String segmentationFolder, String resultsFolder, bool autoDownscale) {

  // set structures ///////////////////////////////////
  bool initialized = false;
  _finddata_t file_learn, file_segm;
  long handle_learn, handle_segm;
  String path_img, path_result;
  Mat BGR, L, u, v, Gx, Gy, Mask, bin_results;
  AFH Model_L, Model_u, Model_v, Model_Gx, Model_Gy;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pair_args args_L, args_u, args_v, args_Gx, args_Gy;
  pthread_t tid_L, tid_u, tid_v, tid_Gx, tid_Gy;
  void *status_L, *status_u, *status_v, *status_Gx, *status_Gy;
  // ///////////////////////////////////

  // Initialization ////////////////////
  if((handle_learn = _findfirst((learningFolder + "*.*").c_str(), &file_learn))==-1) {
    cout<<"No such learning folder!"<<endl;
    return -1;
  }
  else {
    while(_findnext(handle_learn, &file_learn)==0) {
      if(file_learn.attrib != 16) {
        cout << "learning: " << file_segm.name << "\n";
        path_img = learningFolder + file_learn.name;
        BGR = imread(path_img,IMREAD_COLOR);
        if(BGR.empty()) {
            cout << "No more images in learning folder!\n";
            break;
        }
        else {
          if(Max(BGR.rows,BGR.cols) > 600) {
            pyrDown(BGR,BGR);
          }
          imshow("BGR", BGR);
        }
        if(!initialized) {
          bin_results.create(BGR.rows,BGR.cols,CV_8UC1);
          Model_L.CreateAFH(BGR);
          Model_u.CreateAFH(BGR);
          Model_v.CreateAFH(BGR);
          Model_Gx.CreateAFH(BGR);
          Model_Gy.CreateAFH(BGR);
          initialized = true;
        }
        BGR2LuvGxGy(BGR, L, u, v, Gx, Gy);
        args_L.Model = &Model_L;
        args_L.Gray = &L;
        args_u.Model = &Model_u;
        args_u.Gray = &u;
        args_v.Model = &Model_v;
        args_v.Gray = &v;
        args_Gx.Model = &Model_Gx;
        args_Gx.Gray = &Gx;
        args_Gy.Model = &Model_Gy;
        args_Gy.Gray = &Gy;
        pthread_create(&tid_L, &attr, thread_initialize, (void *) &args_L);
        pthread_create(&tid_u, &attr, thread_initialize, (void *) &args_u);
        pthread_create(&tid_v, &attr, thread_initialize, (void *) &args_v);
        pthread_create(&tid_Gx, &attr, thread_initialize, (void *) &args_Gx);
        pthread_create(&tid_Gy, &attr, thread_initialize, (void *) &args_Gy);
        pthread_join(tid_L, &status_L);
        pthread_join(tid_u, &status_u);
        pthread_join(tid_v, &status_v);
        pthread_join(tid_Gx, &status_Gx);
        pthread_join(tid_Gy, &status_Gy);
        waitKey(1);
      }
    }
  }

  if(!initialized) {
    cout << "There is no image for learning!\n";
    return -1;
  }

  // /////////////////////////////////////////////////////////////

  // Segmentation ////////////////////////////////////////////////
  if((handle_segm = _findfirst((segmentationFolder + "*.*").c_str(), &file_segm))==-1) {
    cout<<"No such segmentation folder!"<<endl;
    return -1;
  }
  else {
    while(_findnext(handle_segm, &file_segm)==0) {
      if(file_segm.attrib != 16) {
        cout << "Segmentation: " << file_segm.name << "\n";
        path_img = segmentationFolder + file_segm.name;
        BGR = imread(path_img,IMREAD_COLOR);
        if(BGR.empty()) {
            cout << "No more images in segmentation folder!\n";
            break;
        }
        else {
          if(Max(BGR.rows,BGR.cols) > 600) {
            pyrDown(BGR,BGR);
          }
          imshow("BGR", BGR);
        }
        BGR2LuvGxGy(BGR, L, u, v, Gx, Gy);
        args_L.Model = &Model_L;
        args_L.Gray = &L;
        args_u.Model = &Model_u;
        args_u.Gray = &u;
        args_v.Model = &Model_v;
        args_v.Gray = &v;
        args_Gx.Model = &Model_Gx;
        args_Gx.Gray = &Gx;
        args_Gy.Model = &Model_Gy;
        args_Gy.Gray = &Gy;
        pthread_create(&tid_L, &attr, thread_segmentation, (void *) &args_L);
        pthread_create(&tid_u, &attr, thread_segmentation, (void *) &args_u);
        pthread_create(&tid_v, &attr, thread_segmentation, (void *) &args_v);
        pthread_create(&tid_Gx, &attr, thread_segmentation, (void *) &args_Gx);
        pthread_create(&tid_Gy, &attr, thread_segmentation, (void *) &args_Gy);
        pthread_join(tid_L, &status_L);
        pthread_join(tid_u, &status_u);
        pthread_join(tid_v, &status_v);
        pthread_join(tid_Gx, &status_Gx);
        pthread_join(tid_Gy, &status_Gy);
        Mask = Model_L.Smooth | Model_u.Smooth |  Model_v.Smooth | Model_Gx.Smooth | Model_Gy.Smooth;
        PostProc(Mask);
        bin_results = Mask > 127;
        PostProc(bin_results);
        imshow("bin_results", bin_results);
        args_L.Model = &Model_L;
        args_L.Gray = &Mask;
        args_u.Model = &Model_u;
        args_u.Gray = &Mask;
        args_v.Model = &Model_v;
        args_v.Gray = &Mask;
        args_Gx.Model = &Model_Gx;
        args_Gx.Gray = &Mask;
        args_Gy.Model = &Model_Gy;
        args_Gy.Gray = &Mask;
        pthread_create(&tid_L, &attr, thread_learning, (void *) &args_L);
        pthread_create(&tid_u, &attr, thread_learning, (void *) &args_u);
        pthread_create(&tid_v, &attr, thread_learning, (void *) &args_v);
        pthread_create(&tid_Gx, &attr, thread_learning, (void *) &args_Gx);
        pthread_create(&tid_Gy, &attr, thread_learning, (void *) &args_Gy);
        pthread_join(tid_L, &status_L);
        pthread_join(tid_u, &status_u);
        pthread_join(tid_v, &status_v);
        pthread_join(tid_Gx, &status_Gx);
        pthread_join(tid_Gy, &status_Gy);
        path_result = resultsFolder + "result of " + file_segm.name;
        imwrite(path_result, bin_results);
        waitKey(1);
      }
    }
  }

  _findclose(handle_learn);
  _findclose(handle_segm);
  pthread_attr_destroy(&attr);
  return 0;
}
