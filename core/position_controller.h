// Copyright (c) 2014 The Caroline authors. All rights reserved.
// Use of this source file is governed by a MIT license that can be found in the
// LICENSE file.
/// @author Aleksandr Derbenev <alex@technoworks.ru>

#ifndef CORE_POSITION_CONTROLLER_H_
#define CORE_POSITION_CONTROLLER_H_

#include <cstdint>
#include <memory>

#include "core/position.h"

#include "core/time_controller.h"

namespace Json {

class Value;

}  // namespace Json

namespace core {

class PositionController {
 public:
  static std::unique_ptr<PositionController> Create(
      TimeController* time_controller,
      const Json::Value& settings);

  explicit PositionController(TimeController* time_controller);
  virtual ~PositionController();

  virtual cv::Point3d GetLocation() const = 0;
  virtual Quaternion GetRotation() const = 0;

  cv::Point3d location() const {
    UpdateCacheIfNeeded();
    return location_;
  }

  Quaternion rotation() const {
    UpdateCacheIfNeeded();
    return rotation_;
  }

  Position position() const {
    UpdateCacheIfNeeded();
    return position_;
  }

  TimeController* time_controller() const {
    return time_controller_;
  }

  void set_time_controller(TimeController* time_controller) {
    time_controller_ = time_controller;
  }

 private:
  void UpdateCacheIfNeeded() const;

  TimeController* time_controller_;
  mutable uint64_t current_time_;
  mutable cv::Point3d location_;
  mutable Quaternion rotation_;
  mutable Position position_;
};

}  // namespace core

#endif  // CORE_POSITION_CONTROLLER_H_

