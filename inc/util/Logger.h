#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>

class Logger {
 public:
  // 构造函数，打开日志文件
  static void init(const std::string& filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
      throw std::runtime_error("无法打开日志文件: " + filename);
    }
  }

  // no Copy
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  // 关闭文件的析构函数
  static void destroy() {
    if (logFile.is_open()) {
      logFile.close();
    }
  }

  // 日志函数，使用变长模板参数
  template <typename... Args>
  static void log(Args&&... args) {
    std::lock_guard<std::mutex> guard(mtx);  // 确保线程安全
    logFile << getCurrentTime() << " ";
    writeToStream(logFile, std::forward<Args>(args)...);
    logFile << std::endl;
  }

 private:
  static inline std::ofstream logFile;
  static inline std::mutex mtx;

  // 获取当前时间的字符串表示
  static std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    ::localtime_r(&itt, &tm);  // POSIX
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
  }

  // 递归终止函数
  static void writeToStream(std::ostream& os) {}

  // 递归展开函数
  template <typename T, typename... Args>
  static void writeToStream(std::ostream& os, T&& first, Args&&... rest) {
    os << std::forward<T>(first);
    writeToStream(os, std::forward<Args>(rest)...);
  }
};