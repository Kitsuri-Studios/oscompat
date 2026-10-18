
#include "wdl_util.h"

#include <android/api-level.h>
#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool wdl_util_starts_with(const char *str, const char *start) {
  while (*str && *str == *start) {
    str++;
    start++;
  }

  return '\0' == *start;
}

bool wdl_util_ends_with(const char *str, const char *ending) {
  size_t str_len = strlen(str);
  size_t ending_len = strlen(ending);

  if (ending_len > str_len) return false;

  return 0 == strcmp(str + (str_len - ending_len), ending);
}

size_t wdl_util_trim_ending(char *start) {
  char *end = start + strlen(start);
  while (start < end && isspace((int)(*(end - 1)))) {
    end--;
    *end = '\0';
  }
  return (size_t)(end - start);
}

static int wdl_util_get_api_level_from_build_prop(void) {
  char buf[128];
  int api_level = -1;

  FILE *fp = fopen("/system/build.prop", "r");
  if (NULL == fp) goto end;

  while (fgets(buf, sizeof(buf), fp)) {
    if (wdl_util_starts_with(buf, "ro.build.version.sdk=")) {
      api_level = atoi(buf + 21);
      break;
    }
  }
  fclose(fp);

end:
  return (api_level > 0) ? api_level : -1;
}

int wdl_util_get_api_level(void) {
  static int api_level = -1;
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

  int val = __atomic_load_n(&api_level, __ATOMIC_ACQUIRE);
  if (val < 0) {
    pthread_mutex_lock(&lock);
    val = __atomic_load_n(&api_level, __ATOMIC_RELAXED);
    if (val < 0) {
      val = android_get_device_api_level();
      if (val < 0) val = wdl_util_get_api_level_from_build_prop();
      if (val < __ANDROID_API_J__) val = __ANDROID_API_J__;
      __atomic_store_n(&api_level, val, __ATOMIC_RELEASE);
    }
    pthread_mutex_unlock(&lock);
  }
  return val;
}

size_t wdl_util_get_pagesize(void) {
  static size_t pagesize = 0;
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

  size_t val = __atomic_load_n(&pagesize, __ATOMIC_ACQUIRE);
  if (0 == val) {
    pthread_mutex_lock(&lock);
    val = __atomic_load_n(&pagesize, __ATOMIC_RELAXED);
    if (0 == val) {
      val = (size_t)getpagesize();
      __atomic_store_n(&pagesize, val, __ATOMIC_RELEASE);
    }
    pthread_mutex_unlock(&lock);
  }
  return val;
}
