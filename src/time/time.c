#if !defined(_WIN32)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#endif

#include <oscompat/common.h>
#include <oscompat/time.h>

#include <errno.h>
#include <stdint.h>
#include <time.h>

#if OSCOMPAT_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static uint64_t osQpcFrequency(void) {
	LARGE_INTEGER frequency;
	static uint64_t cached = 0;
	if (cached == 0) {
		if (QueryPerformanceFrequency(&frequency) && frequency.QuadPart > 0) {
			cached = (uint64_t)frequency.QuadPart;
		} else {
			cached = 1;
		}
	}
	return cached;
}

static uint64_t osQpcTicks(void) {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (uint64_t)counter.QuadPart;
}

uint64_t os_time_monotonic_ms(void) {
	uint64_t ticks = osQpcTicks();
	uint64_t frequency = osQpcFrequency();
	return (ticks / frequency) * 1000u + ((ticks % frequency) * 1000u) / frequency;
}

uint64_t os_time_monotonic_us(void) {
	uint64_t ticks = osQpcTicks();
	uint64_t frequency = osQpcFrequency();
	return (ticks / frequency) * 1000000u + ((ticks % frequency) * 1000000u) / frequency;
}

uint64_t os_time_realtime_ms(void) {
	FILETIME fileTime;
	ULARGE_INTEGER stamp;
	GetSystemTimePreciseAsFileTime(&fileTime);
	stamp.LowPart = fileTime.dwLowDateTime;
	stamp.HighPart = fileTime.dwHighDateTime;
	return (uint64_t)((stamp.QuadPart - 116444736000000000ULL) / 10000ULL);
}

void os_sleep_ms(uint64_t ms) {
	while (ms > 0) {
		DWORD chunk = (ms > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (DWORD)ms;
		Sleep(chunk);
		ms -= chunk;
	}
}

void os_sleep_us(uint64_t us) {
	uint64_t wholeMs;
	uint64_t remUs;
	uint64_t start;
	uint64_t target;
	if (us == 0) {
		return;
	}
	wholeMs = us / 1000u;
	remUs = us % 1000u;
	if (wholeMs > 0) {
		os_sleep_ms(wholeMs);
	}
	if (remUs > 0) {
		start = os_time_monotonic_us();
		target = start + remUs;
		while (os_time_monotonic_us() < target) {
			Sleep(0);
		}
	}
}

#else

uint64_t os_time_monotonic_ms(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

uint64_t os_time_monotonic_us(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000u + (uint64_t)ts.tv_nsec / 1000u;
}

uint64_t os_time_realtime_ms(void) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

void os_sleep_ms(uint64_t ms) {
	struct timespec request;
	struct timespec remaining;
	if (ms == 0) {
		return;
	}
	request.tv_sec = (time_t)(ms / 1000u);
	request.tv_nsec = (long)((ms % 1000u) * 1000000u);
	while (nanosleep(&request, &remaining) != 0) {
		if (errno != EINTR) {
			return;
		}
		request = remaining;
	}
}

void os_sleep_us(uint64_t us) {
	struct timespec request;
	struct timespec remaining;
	if (us == 0) {
		return;
	}
	request.tv_sec = (time_t)(us / 1000000u);
	request.tv_nsec = (long)((us % 1000000u) * 1000u);
	while (nanosleep(&request, &remaining) != 0) {
		if (errno != EINTR) {
			return;
		}
		request = remaining;
	}
}

#endif
