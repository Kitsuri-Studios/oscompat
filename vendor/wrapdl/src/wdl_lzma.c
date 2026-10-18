
#include "wdl_lzma.h"

#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wdl_int.h"
#include "wdl_util.h"

// LZMA library pathname & symbol names
#ifndef __LP64__
#define WDL_LZMA_PATHNAME "/system/lib/liblzma.so"
#else
#define WDL_LZMA_PATHNAME "/system/lib64/liblzma.so"
#endif
#define WDL_LZMA_SYM_CRCGEN     "CrcGenerateTable"
#define WDL_LZMA_SYM_CRC64GEN   "Crc64GenerateTable"
#define WDL_LZMA_SYM_CONSTRUCT  "XzUnpacker_Construct"
#define WDL_LZMA_SYM_ISFINISHED "XzUnpacker_IsStreamWasFinished"
#define WDL_LZMA_SYM_FREE       "XzUnpacker_Free"
#define WDL_LZMA_SYM_CODE       "XzUnpacker_Code"

// LZMA data type definition
#define SZ_OK 0
typedef struct ISzAlloc ISzAlloc;
typedef const ISzAlloc *ISzAllocPtr;
struct ISzAlloc {
  void *(*Alloc)(ISzAllocPtr p, size_t size);
  void (*Free)(ISzAllocPtr p, void *address); /* address can be 0 */
};
typedef enum {
  CODER_STATUS_NOT_SPECIFIED,      /* use main error code instead */
  CODER_STATUS_FINISHED_WITH_MARK, /* stream was finished with end mark. */
  CODER_STATUS_NOT_FINISHED,       /* stream was not finished */
  CODER_STATUS_NEEDS_MORE_INPUT    /* you must provide more input bytes */
} ECoderStatus;
typedef enum {
  CODER_FINISH_ANY, /* finish at any point */
  CODER_FINISH_END  /* block must be finished at the end */
} ECoderFinishMode;

// LZMA function type definition
typedef void (*wdl_lzma_crcgen_t)(void);
typedef void (*wdl_lzma_crc64gen_t)(void);
typedef void (*wdl_lzma_construct_t)(void *, ISzAllocPtr);
typedef int (*wdl_lzma_isfinished_t)(const void *);
typedef void (*wdl_lzma_free_t)(void *);
typedef int (*wdl_lzma_code_t)(void *, uint8_t *, size_t *, const uint8_t *, size_t *, ECoderFinishMode,
                               ECoderStatus *);
typedef int (*wdl_lzma_code_q_t)(void *, uint8_t *, size_t *, const uint8_t *, size_t *, int,
                                 ECoderFinishMode, ECoderStatus *);

// LZMA function pointor
static wdl_lzma_construct_t wdl_lzma_construct = NULL;
static wdl_lzma_isfinished_t wdl_lzma_isfinished = NULL;
static wdl_lzma_free_t wdl_lzma_free = NULL;
static void *wdl_lzma_code = NULL;

// LZMA init
static void wdl_lzma_init(void) {
  void *lzma = wdl_open(WDL_LZMA_PATHNAME, WDL_TRY_FORCE_LOAD);
  if (NULL == lzma) return;

  wdl_lzma_crcgen_t crcgen = NULL;
  wdl_lzma_crc64gen_t crc64gen = NULL;
  if (NULL == (crcgen = (wdl_lzma_crcgen_t)wdl_i_sym(lzma, WDL_LZMA_SYM_CRCGEN, NULL))) goto end;
  if (NULL == (crc64gen = (wdl_lzma_crc64gen_t)wdl_i_sym(lzma, WDL_LZMA_SYM_CRC64GEN, NULL))) goto end;
  if (NULL == (wdl_lzma_construct = (wdl_lzma_construct_t)wdl_i_sym(lzma, WDL_LZMA_SYM_CONSTRUCT, NULL)))
    goto end;
  if (NULL == (wdl_lzma_isfinished = (wdl_lzma_isfinished_t)wdl_i_sym(lzma, WDL_LZMA_SYM_ISFINISHED, NULL)))
    goto end;
  if (NULL == (wdl_lzma_free = (wdl_lzma_free_t)wdl_i_sym(lzma, WDL_LZMA_SYM_FREE, NULL))) goto end;
  if (NULL == (wdl_lzma_code = wdl_i_sym(lzma, WDL_LZMA_SYM_CODE, NULL))) goto end;
  crcgen();
  crc64gen();

end:
  wdl_close(lzma);
}

// LZMA internal alloc / free
static void *wdl_lzma_internal_alloc(ISzAllocPtr p, size_t size) {
  (void)p;
  return malloc(size);
}
static void wdl_lzma_internal_free(ISzAllocPtr p, void *address) {
  (void)p;
  free(address);
}

int wdl_lzma_decompress(uint8_t *src, size_t src_size, uint8_t **dst, size_t *dst_size) {
  *dst = NULL;
  *dst_size = 0;

  // LZMA init (only once)
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  static bool inited = false;
  if (!__atomic_load_n(&inited, __ATOMIC_ACQUIRE)) {
    pthread_mutex_lock(&lock);
    if (!__atomic_load_n(&inited, __ATOMIC_RELAXED)) {
      wdl_lzma_init();
      __atomic_store_n(&inited, true, __ATOMIC_RELEASE);
    }
    pthread_mutex_unlock(&lock);
  }
  if (NULL == wdl_lzma_code) return -1;

  // LZMA state init
  long long state[4096 / sizeof(long long)];
  ISzAlloc alloc = {.Alloc = wdl_lzma_internal_alloc, .Free = wdl_lzma_internal_free};
  wdl_lzma_construct(&state, &alloc);

  // LZMA decompress
  int api_level = wdl_util_get_api_level();
  ECoderStatus status;
  size_t src_offset = 0;
  size_t dst_offset = 0;
  *dst_size = 7 * src_size;
  if (NULL == (*dst = malloc(*dst_size))) goto err;
  do {
    size_t src_remaining = src_size - src_offset;
    size_t dst_remaining = *dst_size - dst_offset;
    if (dst_remaining < 2 * src_size) {
      size_t new_dst_size = *dst_size + 2 * src_size;
      uint8_t *new_dst = realloc(*dst, new_dst_size);
      if (NULL == new_dst) goto err;
      *dst = new_dst;
      *dst_size = new_dst_size;
      dst_remaining += (2 * src_size);
    }

    int result;
    if (api_level >= __ANDROID_API_Q__) {
      wdl_lzma_code_q_t lzma_code_q = (wdl_lzma_code_q_t)wdl_lzma_code;
      result = lzma_code_q(&state, *dst + dst_offset, &dst_remaining, src + src_offset, &src_remaining, 1,
                           CODER_FINISH_ANY, &status);
    } else {
      wdl_lzma_code_t lzma_code = (wdl_lzma_code_t)wdl_lzma_code;
      result = lzma_code(&state, *dst + dst_offset, &dst_remaining, src + src_offset, &src_remaining,
                         CODER_FINISH_ANY, &status);
    }
    if (SZ_OK != result) goto err;

    src_offset += src_remaining;
    dst_offset += dst_remaining;
  } while (status == CODER_STATUS_NOT_FINISHED);
  if (!wdl_lzma_isfinished(&state)) goto err;

  // OK
  wdl_lzma_free(&state);
  *dst_size = dst_offset;
  *dst = realloc(*dst, *dst_size);
  return 0;

err:
  wdl_lzma_free(&state);
  if (NULL != *dst) {
    free(*dst);
    *dst = NULL;
  }
  *dst_size = 0;
  return -1;
}
