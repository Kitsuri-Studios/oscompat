#ifndef OSCOMPAT_OPTIONS_H
#define OSCOMPAT_OPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#define OS_NO_ARGUMENT 0
#define OS_REQUIRED_ARGUMENT 1
#define OS_OPTIONAL_ARGUMENT 2

/* Long option description: name, argument policy, optional flag target, and value to return. */
struct OsOption {
	const char* name;
	int hasArg;
	int* flag;
	int val;
};

/* Nonzero enables diagnostic messages on the standard error stream. */
extern int osOptErr;
/* Index of the next argv element to be processed. */
extern int osOptInd;
/* Option letter that triggered the most recent error. */
extern int osOptOpt;
/* Argument associated with the current option, or NULL when none is present. */
extern char* osOptArg;

/* Reset the scanner so a new argument vector can be parsed from the beginning. */
void os_getopt_reset(void);

/* Parse the next short option from argv according to optString, following POSIX getopt(3): scanning stops at the first non-option, argv is never permuted, and a leading ':' in optString suppresses diagnostics and reports a missing argument as ':'. */
int os_getopt(int argc, char* const argv[], const char* optString);

/* Parse the next short or long option according to optString and longOpts, following POSIX getopt_long semantics; long names accept unambiguous abbreviations and an '=value' suffix. */
int os_getopt_long(int argc, char* const argv[], const char* optString, const struct OsOption* longOpts, int* longIndex);

#ifdef __cplusplus
}
#endif

#endif
