/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "wdl_ns_layout.h"

#if defined(__ANDROID__)

#include <stdlib.h>
#include <string.h>
#include <sys/system_properties.h>

#define WDL_NS_MAX 64
#define WDL_NS_LEN 512

typedef struct {
    char items[WDL_NS_MAX][WDL_NS_LEN];
    size_t count;
} wdl_path_list;

typedef struct {
    const char *name_;
    bool is_isolated_;
    bool is_greylist_enabled_;
    wdl_path_list ld_library_paths_;
    wdl_path_list default_library_paths_;
    wdl_path_list permitted_paths_;
} android_namespace_8;

typedef struct {
    const char *name_;
    bool is_isolated_;
    bool is_greylist_enabled_;
    wdl_path_list ld_library_paths_;
    wdl_path_list default_library_paths_;
    wdl_path_list permitted_paths_;
    wdl_path_list allowed_libs_;
} android_namespace_10;

typedef struct {
    char name_[256];
    bool is_isolated_;
    bool is_greylist_enabled_;
    bool is_also_used_as_anonymous_;
    wdl_path_list ld_library_paths_;
    wdl_path_list default_library_paths_;
    wdl_path_list permitted_paths_;
    wdl_path_list allowed_libs_;
} android_namespace_11;

typedef struct {
    char name_[256];
    bool is_isolated_;
    bool is_exempt_list_enabled_;
    bool is_also_used_as_anonymous_;
    wdl_path_list ld_library_paths_;
    wdl_path_list default_library_paths_;
    wdl_path_list permitted_paths_;
    wdl_path_list allowed_libs_;
} android_namespace_12;

int wdl_ns_api(void) {
    char p[92];
    if (__system_property_get("ro.build.version.sdk", p) <= 0) return 0;
    return atoi(p);
}

int wdl_ns_ver(void) {
    int a = wdl_ns_api();
    if (a >= 31) return 12;
    if (a >= 30) return 11;
    if (a >= 29) return 10;
    return 8;
}

static wdl_path_list *ld_paths(struct android_namespace_t *ns) {
    switch (wdl_ns_ver()) {
        case 12: return &((android_namespace_12 *)ns)->ld_library_paths_;
        case 11: return &((android_namespace_11 *)ns)->ld_library_paths_;
        case 10: return &((android_namespace_10 *)ns)->ld_library_paths_;
        default: return &((android_namespace_8 *)ns)->ld_library_paths_;
    }
}

void wdl_ns_set_iso(struct android_namespace_t *ns, bool iso) {
    if (!ns) return;
    switch (wdl_ns_ver()) {
        case 12: ((android_namespace_12 *)ns)->is_isolated_ = iso; break;
        case 11: ((android_namespace_11 *)ns)->is_isolated_ = iso; break;
        case 10: ((android_namespace_10 *)ns)->is_isolated_ = iso; break;
        default: ((android_namespace_8 *)ns)->is_isolated_ = iso; break;
    }
}

bool wdl_ns_add_paths(struct android_namespace_t *ns, const char *const *paths, size_t n) {
    wdl_path_list *list = ld_paths(ns);
    if (!list || !paths) return false;
    for (size_t i = 0; i < n; i++) {
        if (list->count >= WDL_NS_MAX) return false;
        strncpy(list->items[list->count], paths[i], WDL_NS_LEN - 1);
        list->items[list->count][WDL_NS_LEN - 1] = '\0';
        list->count++;
    }
    return true;
}

#else

int wdl_ns_api(void) { return 0; }
int wdl_ns_ver(void) { return 0; }
void wdl_ns_set_iso(struct android_namespace_t *ns, bool iso) { (void)ns; (void)iso; }
bool wdl_ns_add_paths(struct android_namespace_t *ns, const char *const *paths, size_t n) {
    (void)ns; (void)paths; (void)n; return false;
}

#endif
