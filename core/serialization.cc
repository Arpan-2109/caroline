// Copyright (c) 2014 The Caroline authors. All right reserved.
// Use of this source file is governed by a MIT license that can be found in the
// License file
/// @author: Vladislav Krikunov <vlad.krikunoff@yandex.ru>

#include "core/serialization.h"

#include <string>
#include <utility>
#include <vector>

#include "protocol.pb.h"  // NOLINT

namespace bitdata {

void GenDMap(const core::DepthMap& depth_map) {
  std::unique_ptr<Message> message(new Message());
  message->mutable_depth_map()->set_width(depth_map.width());
  message->mutable_depth_map()->set_height(depth_map.height());
  for (double depth : depth_map)
    message->mutable_depth_map()->add_data(depth);
  message->set_type(Message::DEPTH_MAP);
}

void GenOptFlow(const core::OpticalFlow& optical_flow) {
  std::unique_ptr<Message>message(new Message);
  for (auto pair : optical_flow) {
    OpticalFlowItem* item = message->mutable_optical_flow()->add_item();
    item->mutable_left()->set_x(pair.first.x);
    item->mutable_left()->set_y(pair.first.y);
    item->mutable_right()->set_x(pair.second.x);
    item->mutable_right()->set_y(pair.second.y);
  }
  message->set_type(Message::OPTICAL_FLOW);
}

void GenModel(core::Scene3D scene) {
  std::unique_ptr<Message>message(new Message());
  std::vector<core::Point3D> vertexes;
  std::vector<core::Triangle> faces;
  core::SceneElement some_element;
  int vertex_base_index = 0;
  for (auto scene_element : scene) {
    for (auto vertex : scene_element->mesh()->vertexes()) {
      auto transformed_vertex = scene_element->Transform(vertex);
      auto proto_vertex = message->mutable_model()->add_verticies();
      proto_vertex->set_x(vertex.x());
      proto_vertex->set_y(vertex.y());
      proto_vertex->set_z(vertex.z());
    }
    for (auto face : scene_element->mesh()->faces()) {
      auto proto_face = message->mutable_model()->add_faces();
      proto_face->set_point1(face.Point1() + vertex_base_index);
      proto_face->set_point2(face.Point2() + vertex_base_index);
      proto_face->set_point3(face.Point3() + vertex_base_index);
    }
    vertex_base_index += scene_element->mesh()->vertexes().size();
  }
  message->set_type(Message::MODEL);
}

void GenPic(const std::vector<std::pair<cv::Mat, core::Position>>& frameset) {
  std::unique_ptr<Message>message(new Message());
  const cv::Mat& left = frameset[0].first;
  const cv::Mat& right = frameset[1].first;
  message->mutable_images()->mutable_left()->
      set_width(left.size().width);
  message->mutable_images()->mutable_left()->
      set_height(left.size().height);
  message->mutable_images()->mutable_right()->
      set_width(right.size().width);
  message->mutable_images()->mutable_right()->
      set_width(right.size().width);
  for (auto it = left.begin<double>(), end = left.end<double>();
      it != end; ++it) {
    message->mutable_images()->mutable_left()->add_data(*it);
  }
  for (auto it = right.begin<double>(), end = right.end<double>();
      it != end; ++it) {
    message->mutable_images()->mutable_right()->add_data(*it);
  }
  message->set_type(Message::IMAGES);
}

void GenLog(const std::string& message) {
  std::unique_ptr<Message>proto_message(new Message());
  proto_message->mutable_log()->set_line(message);
  proto_message->set_type(Message::LOG);
}

}  // namespace bitdata
