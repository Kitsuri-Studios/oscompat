#if defined(_WIN32)
#define USAGE_EXPORT __declspec(dllexport)
#else
#define USAGE_EXPORT
#endif

USAGE_EXPORT int oscompat_usage_add(int a, int b) {
	return a + b;
}

USAGE_EXPORT const char* oscompat_usage_message(void) {
	return "oscompat";
}
