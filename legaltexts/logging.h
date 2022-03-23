#define trace(...) log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::trace, __VA_ARGS__)
#define debug(...) log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::debug, __VA_ARGS__)
#define info(...)  log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::info, __VA_ARGS__)
#define warn(...)  log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::warn, __VA_ARGS__)
#define error(...) log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::err, __VA_ARGS__)
#define critical(...) log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::critical, __VA_ARGS__)

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

#define format fmt::format //avoid using entire namespace

//spdlog::logger getLog();

/* Alternative implementation is to declare getLog() as above and put the code  *
 * below in a separate source file. In that case neither the variable nor the   *
 * function need to be static.                                                  */

static std::shared_ptr<spdlog::logger> logptr;

static spdlog::logger getLog(){
	logptr = spdlog::get("log");
	if(logptr == nullptr){
		logptr = spdlog::stdout_color_mt("log");
		logptr->set_level(spdlog::level::trace);
		logptr->set_pattern("[%H:%M:%S:%e] [%l] [%!] %v [%s:%#]");
		return *logptr;
	} else {
		return *logptr;
	}
}
