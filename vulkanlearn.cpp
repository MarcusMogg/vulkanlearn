// vulkanlearn.cpp: 定义应用程序的入口点。
//

#include "vulkanlearn.h"

using namespace std;

int main() {
  vkengine::Engine e;
  e.Start();
  e.Shutdown();
  return 0;
}