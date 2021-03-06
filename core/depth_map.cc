// Copyright (c) 2014 The Caroline authors. All rights reserved.
// Use of this source file is governed by a MIT license that can be found in the
// LICENSE file.
/// @author Glazachev Vladimir <glazachev.vladimir@gmail.com>

#include "core/depth_map.h"

#include <memory>
#include <utility>

#include "core/cameras.h"
#include "core/triangulation_eigen.h"
#include "opencv2/core/core.hpp"

namespace core {

DepthMap::DepthMap(int width, int height) {
  width_ = width;
  height_ = height;


  depth_map_.resize(width_ * height_);

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      depth_map_[width_ * j + i] = kBadDepth;
    }
  }
}

void DepthMap::SetDepth(int x, int y, double depth) {
  if (x < 0 || y < 0) {
    return;
  }

  if ((x > width_) || (y > height_)) {
    return;
  }

  depth_map_[width_ * y + x] = depth;
}

cv::Mat DepthMap::AsCVMat() const {
  cv::Mat mat(width(), height(), CV_32F);
  for (int i = 0; i < width(); i++) {
    for (int j = 0; j < height(); j++) {
      mat.at<float>(i, j, 0) = Depth(i, j);
    }
  }

  return mat;
}

double DepthMap::Depth(int x, int y) const {
  if (x < 0 || y < 0) {
    return kBadDepth;
  }

  if ((x > width_) || (y > height_)) {
    return kBadDepth;
  }

  return depth_map_[width_ * y + x];
}

std::unique_ptr<DepthMap> DepthMap::BuildMap(
    const std::vector<std::pair<cv::Point2d, cv::Point2d> > &flow,
    const Cameras &cam, int w, int h) {

  if ((w * h) != flow.size()) {
    return nullptr;
  }

  std::unique_ptr<DepthMap> map(new DepthMap(w, h));
  std::unique_ptr<Triangulation> triangulator =
      std::unique_ptr<Triangulation>(new TriangulationEigen());
  triangulator->SetCameraMatrices(cam);

  for (size_t i = 0; i < flow.size(); i++) {
    cv::Point2d p = flow.at(i).first;

    double depth = triangulator->TriangulateDepth(p, flow.at(i).second);

    map->SetDepth(static_cast<int>(p.x), static_cast<int>(p.y), depth);
  }

  return map;
}

}  // namespace core
