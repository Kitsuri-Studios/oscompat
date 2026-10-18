#include <oscompat/options.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

int osOptErr = 1;
int osOptInd = 1;
int osOptOpt = 0;
char* osOptArg = NULL;

static char* osOptCursor = NULL;

void os_getopt_reset(void) {
	osOptInd = 1;
	osOptOpt = 0;
	osOptArg = NULL;
	osOptCursor = NULL;
}

static int os_parse_long_option(int argc, char* const argv[], const char* optString,
	const struct OsOption* longOpts, int* longIndex, const char* name) {
	const char* equals = strchr(name, '=');
	size_t nameLength = (equals != NULL) ? (size_t)(equals - name) : strlen(name);
	const struct OsOption* match = NULL;
	int matchIndex = -1;
	int matchCount = 0;
	char* value = NULL;
	int index;

	for (index = 0; longOpts != NULL && longOpts[index].name != NULL; index++) {
		size_t optionLength = strlen(longOpts[index].name);
		if (optionLength < nameLength || strncmp(longOpts[index].name, name, nameLength) != 0) {
			continue;
		}
		if (optionLength == nameLength) {
			match = &longOpts[index];
			matchIndex = index;
			matchCount = 1;
			break;
		}
		if (matchCount == 0) {
			match = &longOpts[index];
			matchIndex = index;
		}
		matchCount++;
	}

	osOptCursor = NULL;
	if (match == NULL || matchCount > 1) {
		osOptInd++;
		osOptOpt = 0;
		osOptArg = NULL;
		if (osOptErr && *optString != ':') {
			if (matchCount > 1) {
				fprintf(stderr, "oscompat: option '--%s' is ambiguous\n", name);
			} else {
				fprintf(stderr, "oscompat: unrecognized option '--%s'\n", name);
			}
		}
		return '?';
	}

	if (match->hasArg == OS_NO_ARGUMENT) {
		if (equals != NULL) {
			osOptInd++;
			osOptOpt = 0;
			osOptArg = NULL;
			if (osOptErr && *optString != ':') {
				fprintf(stderr, "oscompat: option '--%s' does not take an argument\n", match->name);
			}
			return '?';
		}
		osOptArg = NULL;
		osOptInd++;
	} else if (match->hasArg == OS_REQUIRED_ARGUMENT) {
		if (equals != NULL) {
			value = (char*)(equals + 1);
			osOptInd++;
		} else if (osOptInd + 1 < argc) {
			value = argv[osOptInd + 1];
			osOptInd += 2;
		} else {
			osOptInd++;
			osOptOpt = 0;
			osOptArg = NULL;
			if (*optString == ':') {
				return ':';
			}
			if (osOptErr) {
				fprintf(stderr, "oscompat: option '--%s' requires an argument\n", match->name);
			}
			return '?';
		}
		osOptArg = value;
	} else {
		osOptArg = (equals != NULL) ? (char*)(equals + 1) : NULL;
		osOptInd++;
	}

	if (longIndex != NULL) {
		*longIndex = matchIndex;
	}
	if (match->flag != NULL) {
		*match->flag = match->val;
		return 0;
	}
	return match->val;
}

static int os_scan_options(int argc, char* const argv[], const char* optString,
	const struct OsOption* longOpts, int* longIndex, int allowLong) {
	const char* scan;
	const char* optionChar;
	int letter;

	if (osOptCursor == NULL || *osOptCursor == '\0') {
		if (osOptInd >= argc) {
			osOptCursor = NULL;
			return -1;
		}
		if (allowLong && argv[osOptInd][0] == '-' && argv[osOptInd][1] == '-' && argv[osOptInd][2] != '\0') {
			return os_parse_long_option(argc, argv, optString, longOpts, longIndex, argv[osOptInd] + 2);
		}
		osOptCursor = argv[osOptInd];
		if (osOptCursor[0] != '-' || osOptCursor[1] == '\0') {
			osOptCursor = NULL;
			return -1;
		}
		if (osOptCursor[1] == '-' && osOptCursor[2] == '\0') {
			osOptInd++;
			osOptCursor = NULL;
			return -1;
		}
		osOptCursor++;
	}

	letter = (unsigned char)*osOptCursor;
	osOptCursor++;
	scan = optString;
	while (*scan == ':' || *scan == '+' || *scan == '-') {
		scan++;
	}
	optionChar = (letter == ':') ? NULL : strchr(scan, letter);
	if (optionChar == NULL) {
		if (*osOptCursor == '\0') {
			osOptInd++;
		}
		osOptOpt = letter;
		osOptArg = NULL;
		if (osOptErr && *optString != ':') {
			fprintf(stderr, "oscompat: illegal option -- %c\n", letter);
		}
		return '?';
	}
	optionChar++;
	if (*optionChar != ':') {
		osOptArg = NULL;
		if (*osOptCursor == '\0') {
			osOptInd++;
		}
	} else {
		if (*osOptCursor != '\0') {
			osOptArg = osOptCursor;
			osOptCursor = NULL;
			osOptInd++;
		} else {
			osOptInd++;
			if (osOptInd >= argc) {
				osOptCursor = NULL;
				osOptArg = NULL;
				osOptOpt = letter;
				if (*optString == ':') {
					return ':';
				}
				if (osOptErr) {
					fprintf(stderr, "oscompat: option requires an argument -- %c\n", letter);
				}
				return '?';
			}
			osOptArg = argv[osOptInd];
			osOptInd++;
			osOptCursor = NULL;
		}
	}
	return letter;
}

int os_getopt(int argc, char* const argv[], const char* optString) {
	if (argv == NULL || optString == NULL) {
		errno = EINVAL;
		return -1;
	}
	return os_scan_options(argc, argv, optString, NULL, NULL, 0);
}

int os_getopt_long(int argc, char* const argv[], const char* optString,
	const struct OsOption* longOpts, int* longIndex) {
	if (argv == NULL || optString == NULL) {
		errno = EINVAL;
		return -1;
	}
	return os_scan_options(argc, argv, optString, longOpts, longIndex, 1);
}
