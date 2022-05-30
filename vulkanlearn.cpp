// vulkanlearn.cpp: 定义应用程序的入口点。
//

#include "vulkanlearn.h"

#include <iostream>

#include "app/hellotriangleapplication.h"
#include "app/objmodelapplication.h"

using namespace std;
using namespace vklearn;

int main() {
  ObjModelApplication app;

  app.Run();

  return 0;
}