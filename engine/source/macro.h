#pragma once

#include "core/exception/assert_exception.h"
#include "core/logsystem/log_system.h"
#include "function/global/global_context.h"

#define GContext vkengine::GlobalContext::GetInstance()

#define GLog GContext.MustFindObject<vkengine::LogSystem>(kLogSystem)

#define LOG_HELPER(LOG_LEVEL, ...) GLog->Log(LOG_LEVEL, SRC, __VA_ARGS__)

#define LogDebug(...) LOG_HELPER(LogSystem::LogLevel::debug, __VA_ARGS__)
#define LogInfo(...) LOG_HELPER(LogSystem::LogLevel::info, __VA_ARGS__)
#define LogWarn(...) LOG_HELPER(LogSystem::LogLevel::warn, __VA_ARGS__)
#define LogError(...) LOG_HELPER(LogSystem::LogLevel::error, __VA_ARGS__)
#define LogFatal(...) LOG_HELPER(LogSystem::LogLevel::fatal, __VA_ARGS__)
