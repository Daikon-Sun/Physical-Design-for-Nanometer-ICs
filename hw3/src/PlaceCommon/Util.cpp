#include "Util.h"
#include <cstdarg>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

#include <sys/resource.h>

string format(const char *fmt, ...) {
  va_list arg;

  // get string length & allocate memory
  va_start(arg,fmt);
  size_t len = vsnprintf(NULL, 0, fmt, arg);
  string buf(len, '\0');
  va_end(arg);

  // write string to buf
  va_start(arg,fmt);
  vsnprintf((char *)buf.c_str(), len+1, fmt, arg);
  va_end(arg);

  return buf;
}
double getPeakMemoryUsage() {
  ifstream statusFile("/proc/self/status");
  float vmPeak;
  string line;

  while(getline(statusFile, line)) {
    if(sscanf(line.c_str(), "VmPeak: %f kB", &vmPeak) == 1)
      return vmPeak/1024; // return in MB
  }

  return 0;
}
double getCurrentMemoryUsage() {
  ifstream statusFile("/proc/self/status");
  float vmSize;
  string line;

  while(getline(statusFile, line)) {
    if(sscanf(line.c_str(), "VmSize: %f kB", &vmSize) == 1)
      return vmSize/1024; // return in MB
  }

  return 0;
}
string getFileBasename(const string& filePathName) {
  string::size_type pos = filePathName.rfind("/");
  if(pos != string::npos) {
    return filePathName.substr(pos+1);
  }
  return filePathName;
}
string getFileName(const string& filePathName) {
  string basename = getFileBasename(filePathName);
  string::size_type pos = basename.find(".");
  return basename.substr(0, pos);
}
string itos(int num) {
  stringstream ss;
  ss << num;
  return ss.str();
}
string dtos(double num) {
  stringstream ss;
  ss << num;
  return ss.str();
}
