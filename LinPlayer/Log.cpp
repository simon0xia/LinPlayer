#include <time.h>
#include "Log.h"
#include <windows.h>

void CSingletonLog::Init(const char *path)
{
	if (path){
		sprintf_s(logName, FileNameLen, path);
	} else {
		const time_t t = time(NULL);
		struct tm* current_time = localtime(&t);
		sprintf_s(logName, FileNameLen, "%04d-%02d-%02d_%02d%02d%02d.log",
			current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday,
			current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
	}
}

void CSingletonLog::FlashLog(const char *msg,...)
{
	if (!msg)
	{
		return;
	}
	va_list args;

	char LogTime[100];
	const time_t t = time(NULL);
	struct tm* current_time = localtime(&t);
	sprintf_s(LogTime, "%02d-%02d %02d:%02d:%02d ",
		current_time->tm_mon + 1, current_time->tm_mday,
		current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
	
	cs.lock();
	fopen_s(&fp, logName, "a");
	if (!fp)
	{
		return;
	}
	fprintf_s(fp, LogTime);
	va_start(args, msg);
	vfprintf_s(fp, msg, args);
	va_end(args);
	fflush(fp);
	fclose(fp);
	fp = NULL;
	cs.unlock();
}