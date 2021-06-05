#pragma once
#include <memory>
#include <spdlog/spdlog.h>

class Log {
public:
    static void Init();

    inline static std::shared_ptr<spdlog::logger> &getLogger() { return logger; };
private:
    static std::shared_ptr<spdlog::logger> logger;
    static bool isInitialized;
};

#define TRACE(...)      Log::getLogger()->trace(__VA_ARGS__)
#define INFO(...)       Log::getLogger()->info(__VA_ARGS__)
#define WARN(...)       Log::getLogger()->warn(__VA_ARGS__)
#define ERROR(...)      Log::getLogger()->error(__VA_ARGS__)
#define FATAL(...)      Log::getLogger()->fatal(__VA_ARGS__)