// Copyright (c) 2014 The Caroline authors. All rights reserved.
// Use of this source file is governed by a MIT license that can be found in the
// LICENSE file.
// Author: Aleksandr Derbenev <13alexac@gmail.com>

#include <fcntl.h>
#include <unistd.h>

#include <iostream>  // NOLINT
#include <memory>
#include <string>

#include "base/logging.h"

namespace base {

class LoggerPosix : public Logger {
 public:
  LoggerPosix();
  virtual ~LoggerPosix();

  virtual void PostMessage(const std::string& message) override;

 private:
  LoggerPosix(const LoggerPosix&);
  LoggerPosix& operator=(const LoggerPosix&);
};

// static
std::shared_ptr<Logger> Logger::GetInstance() {
  static std::shared_ptr<Logger> instance(new LoggerPosix());
  return instance;
}

LoggerPosix::LoggerPosix() {}

LoggerPosix::~LoggerPosix() {}

void LoggerPosix::PostMessage(const std::string& message) {
  std::cout << message << std::flush;
  if (file().empty())
    return;

  int fd = open(file().c_str(),
      O_APPEND | O_CREAT | O_EXLOCK | O_SYMLINK | O_CLOEXEC);
  if (fd < 0) {
    perror("Can't open log file: ");
    return;
  }

  ssize_t offset = write(fd, message.c_str(), message.size());
  if (offset == -1) {
    perror("Can't write to log: ");
    if (close(fd) == -1)
      perror("Can't close log: ");
    return;
  }
  while (offset != message.size()) {
    ssize_t ret = write(fd, message.c_str() + offset, message.size() - offset);
    if (ret == -1) {
      perror("Can't write to log: ");
      if (close(fd) == -1)
        perror("Can't close log: ");
      return;
    }
    offset += ret;
  }

  if (close(fd))
    perror("Can't close log: ");
}

}  // namespace base
