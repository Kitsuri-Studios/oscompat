#ifndef OSCOMPAT_TIME_H
#define OSCOMPAT_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return milliseconds from a monotonic clock that never runs backwards. */
uint64_t os_time_monotonic_ms(void);

/* Return microseconds from a monotonic clock that never runs backwards. */
uint64_t os_time_monotonic_us(void);

/* Return milliseconds elapsed since the Unix epoch. */
uint64_t os_time_realtime_ms(void);

/* Block for at least the requested number of milliseconds. */
void os_sleep_ms(uint64_t ms);

/* Block for at least the requested number of microseconds. */
void os_sleep_us(uint64_t us);

#ifdef __cplusplus
}
#endif

#endif
