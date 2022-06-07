#include "engine.h"

#include "macro.h"

using namespace vkengine;

void Engine::Start() {
  GContext.StartSystem();
  LogInfo("start");
}

void Engine::Shutdown() {
  LogInfo("stop");
  GContext.ShutdownSystem();
}

Engine::~Engine() {}