
#include "wdl_linker.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "wdl_int.h"
#include "wdl_iterate.h"
#include "wdl_util.h"

#define WDL_LINKER_SYM_MUTEX           "__dl__ZL10g_dl_mutex"
#define WDL_LINKER_SYM_DLOPEN_EXT_N    "__dl__ZL10dlopen_extPKciPK17android_dlextinfoPv"
#define WDL_LINKER_SYM_DO_DLOPEN_N     "__dl__Z9do_dlopenPKciPK17android_dlextinfoPv"
#define WDL_LINKER_SYM_DLOPEN_O        "__dl__Z8__dlopenPKciPKv"
#define WDL_LINKER_SYM_LOADER_DLOPEN_P "__loader_dlopen"

#ifndef __LP64__
#define LIB "lib"
#else
#define LIB "lib64"
#endif

typedef void *(*wdl_linker_dlopen_n_t)(const char *, int, const void *, void *);
typedef void *(*wdl_linker_dlopen_o_t)(const char *, int, const void *);

static pthread_mutex_t *wdl_linker_mutex = NULL;
static void *wdl_linker_dlopen = NULL;

typedef enum { MATCH_PREFIX, MATCH_SUFFIX } wdl_linker_match_type_t;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
typedef struct {
  wdl_linker_match_type_t type;
  const char *value;
} wdl_linker_match_t;
#pragma clang diagnostic pop

typedef struct {
  void *addr;
  wdl_linker_match_t *matches;
  size_t matches_cursor;
} wdl_linker_caller_t;

// https://source.android.com/docs/core/architecture/vndk/linker-namespace
// The following rules are loose and incomplete, you can add more according to your needs.
static wdl_linker_match_t wdl_linker_match_default[] = {{MATCH_SUFFIX, "/libc.so"}};
static wdl_linker_match_t wdl_linker_match_art[] = {{MATCH_SUFFIX, "/libart.so"}};
static wdl_linker_match_t wdl_linker_match_sphal[] = {{MATCH_PREFIX, "/vendor/" LIB "/egl/"},
                                                      {MATCH_PREFIX, "/vendor/" LIB "/hw/"},
                                                      {MATCH_PREFIX, "/vendor/" LIB "/"},
                                                      {MATCH_PREFIX, "/odm/" LIB "/"}};
static wdl_linker_match_t wdl_linker_match_vndk[] = {{MATCH_PREFIX, "/apex/com.android.vndk.v"},
                                                     {MATCH_PREFIX, "/vendor/" LIB "/vndk-sp/"},
                                                     {MATCH_PREFIX, "/odm/" LIB "/vndk-sp/"}};
static wdl_linker_caller_t wdl_linker_callers[] = {
    {NULL, wdl_linker_match_default, sizeof(wdl_linker_match_default) / sizeof(wdl_linker_match_t)},
    {NULL, wdl_linker_match_art, sizeof(wdl_linker_match_art) / sizeof(wdl_linker_match_t)},
    {NULL, wdl_linker_match_sphal, sizeof(wdl_linker_match_sphal) / sizeof(wdl_linker_match_t)},
    {NULL, wdl_linker_match_vndk, sizeof(wdl_linker_match_vndk) / sizeof(wdl_linker_match_t)}};

static void wdl_linker_init_symbols_impl(void) {
  // find linker from: /proc/self/maps (API level < 18) or getauxval (API level >= 18)
  void *handle = wdl_open(WDL_UTIL_LINKER_BASENAME, WDL_DEFAULT);
  if (NULL == handle) return;

  int api_level = wdl_util_get_api_level();
  if (__ANDROID_API_L__ == api_level || __ANDROID_API_L_MR1__ == api_level) {
    // == Android 5.x
    wdl_linker_mutex = (pthread_mutex_t *)wdl_i_dsym(handle, WDL_LINKER_SYM_MUTEX, NULL);
  } else if (__ANDROID_API_N__ == api_level || __ANDROID_API_N_MR1__ == api_level) {
    // == Android 7.x
    wdl_linker_dlopen = wdl_i_dsym(handle, WDL_LINKER_SYM_DLOPEN_EXT_N, NULL);
    if (NULL == wdl_linker_dlopen) {
      wdl_linker_dlopen = wdl_i_dsym(handle, WDL_LINKER_SYM_DO_DLOPEN_N, NULL);
      wdl_linker_mutex = (pthread_mutex_t *)wdl_i_dsym(handle, WDL_LINKER_SYM_MUTEX, NULL);
    }
  } else if (__ANDROID_API_O__ == api_level || __ANDROID_API_O_MR1__ == api_level) {
    // == Android 8.x
    wdl_linker_dlopen = wdl_i_dsym(handle, WDL_LINKER_SYM_DLOPEN_O, NULL);
  } else if (api_level >= __ANDROID_API_P__) {
    // >= Android 9.0
    wdl_linker_dlopen = wdl_i_sym(handle, WDL_LINKER_SYM_LOADER_DLOPEN_P, NULL);
  }

  wdl_close(handle);
}

static void wdl_linker_init_symbols(void) {
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  static bool inited = false;
  if (!__atomic_load_n(&inited, __ATOMIC_ACQUIRE)) {
    pthread_mutex_lock(&lock);
    if (!__atomic_load_n(&inited, __ATOMIC_RELAXED)) {
      wdl_linker_init_symbols_impl();
      __atomic_store_n(&inited, true, __ATOMIC_RELEASE);
    }
    pthread_mutex_unlock(&lock);
  }
}

void wdl_linker_lock(void) {
  wdl_linker_init_symbols();

  if (NULL != wdl_linker_mutex) pthread_mutex_lock(wdl_linker_mutex);
}

void wdl_linker_unlock(void) {
  if (NULL != wdl_linker_mutex) pthread_mutex_unlock(wdl_linker_mutex);
}

static void *wdl_linker_get_caller_addr(struct dl_phdr_info *info) {
  for (size_t i = 0; i < info->dlpi_phnum; i++) {
    const ElfW(Phdr) *phdr = &(info->dlpi_phdr[i]);
    if (PT_LOAD == phdr->p_type) {
      return (void *)(info->dlpi_addr + phdr->p_vaddr);
    }
  }
  return NULL;
}

static void wdl_linker_save_caller_addr(struct dl_phdr_info *info, wdl_linker_caller_t *caller,
                                        size_t cursor) {
  void *addr = wdl_linker_get_caller_addr(info);
  if (NULL != addr) {
    caller->addr = addr;
    caller->matches_cursor = cursor;
  }
}

static int wdl_linker_get_caller_addr_cb(struct dl_phdr_info *info, size_t size, void *arg) {
  (void)size, (void)arg;
  if (0 == info->dlpi_addr || NULL == info->dlpi_name) return 0;  // continue

  int ret = 1;  // OK
  for (size_t i = 0; i < sizeof(wdl_linker_callers) / sizeof(wdl_linker_callers[0]); i++) {
    wdl_linker_caller_t *caller = &wdl_linker_callers[i];
    for (size_t j = 0; j < caller->matches_cursor; j++) {
      wdl_linker_match_t *match = &caller->matches[j];
      if (MATCH_PREFIX == match->type) {
        if (wdl_util_starts_with(info->dlpi_name, match->value)) {
          wdl_linker_save_caller_addr(info, caller, j);
        }
      } else if (MATCH_SUFFIX == match->type) {
        if (wdl_util_ends_with(info->dlpi_name, match->value)) {
          wdl_linker_save_caller_addr(info, caller, j);
        }
      }
    }
    if (NULL == caller->addr || 0 != caller->matches_cursor) ret = 0;  // continue
  }
  return ret;
}

static void wdl_linker_init_caller_addr_impl(void) {
  wdl_iterate_phdr_impl(wdl_linker_get_caller_addr_cb, NULL, WDL_DEFAULT);
}

static void wdl_linker_init_caller_addr(void) {
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  static bool inited = false;
  if (!__atomic_load_n(&inited, __ATOMIC_ACQUIRE)) {
    pthread_mutex_lock(&lock);
    if (!__atomic_load_n(&inited, __ATOMIC_RELAXED)) {
      wdl_linker_init_caller_addr_impl();
      __atomic_store_n(&inited, true, __ATOMIC_RELEASE);
    }
    pthread_mutex_unlock(&lock);
  }
}

void *wdl_linker_force_dlopen(const char *filename) {
  int api_level = wdl_util_get_api_level();

  if (api_level <= __ANDROID_API_M__) {
    // <= Android 6.0
    return dlopen(filename, RTLD_NOW);
  } else {
    wdl_linker_init_symbols();
    if (NULL == wdl_linker_dlopen) {
      dlerror();
      return dlopen(filename, RTLD_NOW);
    }
    wdl_linker_init_caller_addr();

    void *handle = NULL;
    if (__ANDROID_API_N__ == api_level || __ANDROID_API_N_MR1__ == api_level) {
      // == Android 7.x
      wdl_linker_lock();
      for (size_t i = 0; i < sizeof(wdl_linker_callers) / sizeof(wdl_linker_callers[0]); i++) {
        wdl_linker_caller_t *caller = &wdl_linker_callers[i];
        if (NULL != caller->addr) {
          handle = ((wdl_linker_dlopen_n_t)wdl_linker_dlopen)(filename, RTLD_NOW, NULL, caller->addr);
          if (NULL != handle) break;
        }
      }
      wdl_linker_unlock();
    } else {
      // >= Android 8.0
      for (size_t i = 0; i < sizeof(wdl_linker_callers) / sizeof(wdl_linker_callers[0]); i++) {
        wdl_linker_caller_t *caller = &wdl_linker_callers[i];
        if (NULL != caller->addr) {
          handle = ((wdl_linker_dlopen_o_t)wdl_linker_dlopen)(filename, RTLD_NOW, caller->addr);
          if (NULL != handle) break;
        }
      }
    }
    if (NULL == handle) {
      dlerror();
      handle = dlopen(filename, RTLD_NOW);
    }
    return handle;
  }
}
