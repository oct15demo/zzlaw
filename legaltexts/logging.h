#define trace(...) log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::trace, __VA_ARGS__)
#define debug(...) log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::debug, __VA_ARGS__)
#define info(...)  log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::info, __VA_ARGS__)
#define warn(...)  log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::warn, __VA_ARGS__)
#ifdef LOGGER_ERROR //define in .cpp to avoid conflict with src/framework/XMLErrorReporter function 'error'
	#define error(...) log(spdlog::source_loc{__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)}, spdlog::level::err, __VA_ARGS__)
#endif
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
		logptr->set_level(spdlog::level::warn);
		logptr->set_pattern("[%H:%M:%S:%e] [%l] [%!] %v [%s:%#]");
		return *logptr;
	} else {
		return *logptr;
	}
}
