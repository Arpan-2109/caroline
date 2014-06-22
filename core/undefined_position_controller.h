// Copyright (c) 2014 The Caroline authors. All rights reserved.
// Use of this source file is governed by a MIT license that can be found in the
// LICENSE file.
// Author: Aleksandr Derbenev <13alexac@gmail.com>

#ifndef CORE_UNDEFINED_POSITION_CONTROLLER_H_
#define CORE_UNDEFINED_POSITION_CONTROLLER_H_

#include "core/position_controller.h"

namespace core {

class UndefinedPositionController : public PositionController {
 public:
  static std::unique_ptr<PositionController> Create(
      TimeController* time_controller,
      base::DictionaryValue* settings);
  explicit UndefinedPositionController(TimeController* time_controller);
  virtual ~UndefinedPositionController();

  virtual Point3D GetLocation() const override;
  virtual Quaternion GetRotation() const override;
};

}  // namespace core

#endif  // CORE_UNDEFINED_POSITION_CONTROLLER_H_