#include <stdio.h>
#include <time.h>

#if defined _WIN32
#include <windows.h>
#include "forge.h"

void imagine_current_time_get(uint32 *seconds, uint32 *fractions)
{
	static LARGE_INTEGER frequency;
	static boolean init = FALSE;
	LARGE_INTEGER counter;

	if(!init)
	{
		init = TRUE;
		QueryPerformanceFrequency(&frequency);
	}

	QueryPerformanceCounter(&counter);
	if(seconds != NULL)
		*seconds = (uint32)(counter.QuadPart / frequency.QuadPart);
	if(fractions != NULL)
		*fractions = (uint32)((((ULONGLONG) 0xffffffffU) * (counter.QuadPart % frequency.QuadPart)) / frequency.QuadPart);
}

#else

#include <sys/time.h>
#include "forge.h"

void imagine_current_time_get(uint32 *seconds, uint32 *fractions)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	if(seconds != NULL)
	    *seconds = tv.tv_sec;
	if(fractions != NULL)
		*fractions = tv.tv_usec * 1E-6 * (double) (uint32)~0;
}

#endif

double imagine_delta_time_compute(uint new_seconds, uint new_fractions, uint old_seconds, uint old_fractions)
{
	return (double)new_seconds - (double)old_seconds + ((double)new_fractions - (double)old_fractions) / (double)(0xffffffff);
}

int64 imagine_current_system_time_get()
{
	time_t rawtime;
	time(&rawtime);
	return (int64)rawtime;
}

void imagine_current_date_local(int64 time, uint *seconds, uint *minutes, uint *hours, uint *week_days, uint *month_days, uint *month, uint *year)
{
	time_t rawtime;
	struct tm *timeinfo;
	rawtime = (time_t)time;
	timeinfo = localtime(&time);
	if(NULL != seconds)
		*seconds = timeinfo->tm_sec;
	if(NULL != minutes)
		*minutes = timeinfo->tm_min;
	if(NULL != hours)
		*hours = timeinfo->tm_hour;
	if(NULL != week_days)
		*week_days = timeinfo->tm_wday;
	if(NULL != month_days)
		*month_days = timeinfo->tm_mday;
	if(NULL != month)
		*month = timeinfo->tm_mon;
	if(NULL != year)
		*year = timeinfo->tm_year + 1900;
}

#ifdef _WIN32

#define IMAGIVE_WINDOWS_EPOC_ADDITION 12591160000
#define IMAGIVE_WINDOWS_FRACTIONS 10000000

void imagine_time_epoc_current(int64 *seconds, uint64 *fractions)
{
	VOID(WINAPI *i_GetSystemTimePreciseAsFileTime)(_Out_ LPFILETIME lpSystemTimeAsFileTime) = NULL;
	static boolean loaded = FALSE;
	union{FILETIME st; uint64 ui;} filetime;
	uint64 f64;
//	GetSystemTimePreciseAsFileTime(&filetime.st);

	if (loaded)
	{
		void *dll;
#ifdef UNICODE
		unsigned int i;
		short u_text[32];
		char *text = "Kernel32.lib";
		for (i = 0; i < 32 && text[i] != 0; i++)
			u_text[i] = (char)text[i];
		u_text[i] = 0;

		dll = LoadLibrary(u_text);
#else
		dll = LoadLibrary("Kernel32.lib");
#endif
		if(dll != NULL)
			i_GetSystemTimePreciseAsFileTime = (void(__stdcall *)(LPFILETIME lpSystemTimeAsFileTime))GetProcAddress(dll, "GetSystemTimePreciseAsFileTime");
		loaded = TRUE;
	}
	if (i_GetSystemTimePreciseAsFileTime != NULL)
		i_GetSystemTimePreciseAsFileTime(&filetime.st);
	else
		GetSystemTimeAsFileTime(&filetime.st);

	*seconds = (filetime.ui / IMAGIVE_WINDOWS_FRACTIONS) - IMAGIVE_WINDOWS_EPOC_ADDITION;
	f64 = (filetime.ui % IMAGIVE_WINDOWS_FRACTIONS);
	f64 *= 0x0000000100000000;
	*fractions = (f64 / IMAGIVE_WINDOWS_FRACTIONS) * 0x0000000100000000;
}

#endif
/*
void imagine_time_epoc_test()
{
	VOID(WINAPI *i_GetSystemTimePreciseAsFileTime)(_Out_ LPFILETIME lpSystemTimeAsFileTime) = NULL;
	static boolean loaded = FALSE;
	SYSTEMTIME SystemTime;
	union{FILETIME st; uint64 ui;} FileTime;
	SystemTime.wDay = 1;
	SystemTime.wMonth = 1;
	SystemTime.wYear = 2000;
	SystemTime.wHour = 0;
	SystemTime.wMinute = 0;
	SystemTime.wSecond = 0;
	SystemTime.wMilliseconds = 0;
	SystemTime.wDayOfWeek = 0;
//		FileTimeToSystemTime(&FileTime.st, &SystemTime);
	if(loaded)
	{
		void *dll;
#ifdef UNICODE
		unsigned int i;
		short u_text[32];
		char *text = "User32.dll";
		for (i = 0; i < 32 && text[i] != 0; i++)
			u_text[i] = (char)text[i];
		u_text[i] = 0;

		dll = LoadLibrary(u_text);
#else
		dll = LoadLibrary("User32.dll");
#endif
		if(dll != NULL)
			i_GetSystemTimePreciseAsFileTime = (void(__stdcall *)(LPFILETIME lpSystemTimeAsFileTime))GetProcAddress(dll, "GetSystemTimePreciseAsFileTime");
		loaded = TRUE;
	}
	if(i_GetSystemTimePreciseAsFileTime != NULL)
		i_GetSystemTimePreciseAsFileTime(&FileTime.st);
	else
		GetSystemTimeAsFileTime(&FileTime.st);
//	SystemTimeToFileTime(&SystemTime, &FileTime.st);
	printf("File time %llu %llu\n", FileTime.st.dwLowDateTime, FileTime.st.dwHighDateTime);
	printf("64 bit value %llu\n", FileTime.ui);
	FileTimeToSystemTime(&FileTime.st, &SystemTime);
	printf("time %u\n", (uint)SystemTime.wDay);
	printf("time %u\n", (uint)SystemTime.wMonth);
	printf("time %u\n", (uint)SystemTime.wYear);
	printf("time %u\n", (uint)SystemTime.wHour);
	printf("time %u\n", (uint)SystemTime.wMinute);
	printf("time %u\n", (uint)SystemTime.wSecond);
	printf("time %u\n", (uint)SystemTime.wMilliseconds);
	printf("time %u\n", (uint)SystemTime.wDayOfWeek);
	SystemTimeToFileTime(&SystemTime, &FileTime.st);
	printf("File time %llu %llu\n", FileTime.st.dwLowDateTime, FileTime.st.dwHighDateTime);
	SystemTime.wDay = 1;
	SystemTime.wMonth = 1;
	SystemTime.wYear = 2000;
	SystemTime.wHour = 0;
	SystemTime.wMinute = 0;
	SystemTime.wSecond = 0;
	SystemTime.wMilliseconds = 0;
	for(SystemTime.wDayOfWeek = 0; SystemTime.wDayOfWeek < 8; SystemTime.wDayOfWeek++)
		if(SystemTimeToFileTime(&SystemTime, &FileTime.st))
			printf("File time %llu %llu, %u \n", FileTime.st.dwLowDateTime, FileTime.st.dwHighDateTime, (uint)SystemTime.wDayOfWeek);
}
*/