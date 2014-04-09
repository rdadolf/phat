#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include "json.hh"

#include <string.h> // strerror
#include <errno.h> // errno types

#define INFO() LogInstance("INFO",__FILE__,__LINE__)
#define WARN() LogInstance("WARN",__FILE__,__LINE__)
#define ERROR() LogInstance("ERROR",__FILE__,__LINE__)

class LogState {
private:
  std::ofstream logfile_;

  LogState() {
    logfile_.open("log.txt", std::ios_base::out | std::ios_base::app);
  }
public:
  static LogState& get() {
    static LogState ls;
    return ls;
  }

  LogState &operator<< (String rhs) {
    logfile_ << rhs;
    logfile_.flush();
    return *this;
  }
};

class LogInstance {
private:
  String buffer_;
  const char *mode_, *file_;
  int line_;
public:
  LogInstance(const char *mode, const char *file, const int line) : mode_(mode), file_(file), line_(line) {};
  LogInstance& operator<< (String rhs) {
    buffer_.append(rhs);
    return *this;
  }
  LogInstance& operator<< (const char* rhs) {
    buffer_.append(rhs);
    return *this;
  }
  LogInstance& operator<< (int rhs) {
    buffer_.append(String(rhs));
    return *this;
  }
  LogInstance& operator<< (Json rhs) {
    buffer_.append(rhs.unparse());
    return *this;
  }
  ~LogInstance() {
    struct timeval tv;
    char prefix[60];
    String final_buffer;

    gettimeofday(&tv, 0);
    snprintf(prefix, 60, "%ld.%06ld [%d:%s:%d] %s: ",
      tv.tv_sec, tv.tv_usec, getpid(), file_, line_, mode_);
    final_buffer += prefix;
    final_buffer += buffer_;
    final_buffer += "\n";
    LogState::get() << final_buffer;
  }
};

#endif // __LOG_H__
