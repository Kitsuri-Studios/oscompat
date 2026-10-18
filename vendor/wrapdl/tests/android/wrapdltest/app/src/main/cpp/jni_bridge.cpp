#include <jni.h>
#include <string>

#include "test_runner.h"

extern "C" JNIEXPORT jstring JNICALL
Java_io_kitsuri_wrapdltest_MainActivity_nativeRunTests(JNIEnv *env, jobject,
                                                       jstring j_lib_dir,
                                                       jstring j_self_path) {
    const char *lib_dir = env->GetStringUTFChars(j_lib_dir, nullptr);
    const char *self_path = env->GetStringUTFChars(j_self_path, nullptr);
    std::string report = run_wrapdl_tests(lib_dir, self_path);
    env->ReleaseStringUTFChars(j_lib_dir, lib_dir);
    env->ReleaseStringUTFChars(j_self_path, self_path);
    return env->NewStringUTF(report.c_str());
}
