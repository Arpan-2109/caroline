// Copyright (c) 2014 The Caroline authors. All rights reserved.
// Use of this source file is governed by a MIT license that can be found in the
// LICENSE file.
/// @author: Kukleva Anna <Kuklevaanna@gmail.com>

#include "core/demo/stereo_calib_demo.h"

#include <string>

#include "core/cameras.h"
#include "core/preferences_service.h"

namespace {

const int x = 7;
const int y = 5;
float sq_size = 30;

}

#if CV_VERSION_MAJOR > 2 || (CV_VERSION_MAJOR == 2 && CV_VERSION_MINOR >= 4)
#define cvMoveWindow cv::moveWindow
#endif

namespace demo {

// static
const char StereoCalibDemo::kDemoName[] = "stereo_calib";

StereoCalibDemo::StereoCalibDemo(base::CommandLine* command_line)
  : Caroline(command_line),
    cap_number1_(2),
    cap_number2_(1) {
}

bool StereoCalibDemo::Init() {
  cap1_ = cv::VideoCapture(cap_number1_);
  cap2_ = cv::VideoCapture(cap_number2_);

  return cap1_.isOpened() && cap2_.isOpened();
}

int StereoCalibDemo::Run() {
  core::Cameras cam;
  core::CameraCalibration calib;

  cv::Matx33d intr_mat;

  // Number of pair images
  int k = 3;
  for (int i = 0; i < k; i++) {
    cap1_ >> frame1_;
    cap2_ >> frame2_;

    cv::cvtColor(frame1_, frame1_, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frame2_, frame2_, cv::COLOR_BGR2GRAY);

    calib.addImagePair(frame1_, frame2_, x, y);

    cv::imwrite("../im/left" + std::to_string(i) + ".png", frame1_);
    cv::imwrite("../im/right" + std::to_string(i) + ".png", frame2_);

    cv::resize(frame1_, frame1_, cv::Size(),
               0.3, 0.3);
    cv::resize(frame2_, frame2_, cv::Size(),
               0.3, 0.3);

    cv::imshow("left", frame1_);
    cv::imshow("right", frame2_);

    cvMoveWindow("left", 0, 0);
    cvMoveWindow("right", 500, 0);

    cv::waitKey(-1);
  }

  cv::destroyWindow("left");
  cv::destroyWindow("right");

  if (!calib.return_size()) {
    std::cout << "No chessboard on images.\n";
    return core::RETURN_OK;
  }

  calib.HarvestChessboardIdealPointList(x, y, sq_size);

  cam = calib.calibrate(x, y, sq_size);
  
  cam.SaveToConfig(config());

  Json::Value json_K1 = MatxToJSON(cv::Mat(cam.K1()));
  Json::Value json_K2 = MatxToJSON(cv::Mat(cam.K2()));
  Json::Value json_P1 = MatxToJSON(cv::Mat(cam.P1()));
  Json::Value json_P2 = MatxToJSON(cv::Mat(cam.P2()));
  Json::Value json_D1 = MatxToJSON(cam.D1());
  Json::Value json_D2 = MatxToJSON(cam.D2());
  Json::Value json_R = MatxToJSON(cv::Mat(cam.R()));
  Json::Value json_T = MatxToJSON(cv::Mat(cam.T()));

  core::PrefService* prefs = core::PrefService::GetInstance();
  if (!prefs) {
    return core::RETURN_OK;
  }
  Json::Value *dict = prefs->GetDict(std::string());

  (*dict)["K1"] = json_K1;
  (*dict)["K2"] = json_K2;
  (*dict)["P1"] = json_P1;
  (*dict)["P2"] = json_P2;
  (*dict)["D1"] = json_D1;
  (*dict)["D2"] = json_D2;
  (*dict)["R"] = json_R;
  (*dict)["T"] = json_T;

  prefs->WriteToConfig("../im/calib_param");

  return core::RETURN_OK;
}

}  // namespace demo
