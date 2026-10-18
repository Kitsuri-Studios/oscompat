#if !defined(_WIN32) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif

#include <oscompat/process.h>
#include <oscompat/common.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static void os_append_output(char* out, size_t outSize, const char* data, size_t length, size_t* written) {
	size_t room;
	size_t copy;

	if (outSize == 0 || *written + 1 >= outSize) {
		return;
	}
	room = outSize - 1 - *written;
	copy = (length < room) ? length : room;
	memcpy(out + *written, data, copy);
	*written += copy;
}

#if OSCOMPAT_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static size_t os_win_quote_length(const char* arg, int* needsQuote) {
	size_t length = 0;
	size_t backslashes = 0;
	int quote = 0;
	const char* probe;

	if (*arg == '\0') {
		quote = 1;
	}
	for (probe = arg; *probe != '\0'; probe++) {
		if (*probe == ' ' || *probe == '\t') {
			quote = 1;
		}
	}
	for (probe = arg; ; probe++) {
		if (*probe == '\\') {
			backslashes++;
			continue;
		}
		if (*probe == '"') {
			length += backslashes * 2 + 1;
			backslashes = 0;
		} else if (*probe == '\0') {
			if (quote) {
				length += backslashes * 2;
			}
			break;
		} else {
			length += backslashes + 1;
			backslashes = 0;
		}
	}
	if (quote) {
		length += 2;
	}
	*needsQuote = quote;
	return length;
}

static void os_win_append_arg(char* destination, size_t* position, const char* arg, int quote) {
	size_t backslashes = 0;
	size_t index;
	const char* probe;

	if (quote) {
		destination[(*position)++] = '"';
	}
	for (probe = arg; ; probe++) {
		if (*probe == '\\') {
			backslashes++;
			continue;
		}
		if (*probe == '"') {
			for (index = 0; index < backslashes * 2 + 1; index++) {
				destination[(*position)++] = '\\';
			}
			destination[(*position)++] = '"';
			backslashes = 0;
		} else if (*probe == '\0') {
			if (quote) {
				for (index = 0; index < backslashes * 2; index++) {
					destination[(*position)++] = '\\';
				}
			}
			break;
		} else {
			for (index = 0; index < backslashes; index++) {
				destination[(*position)++] = '\\';
			}
			backslashes = 0;
			destination[(*position)++] = *probe;
		}
	}
	if (quote) {
		destination[(*position)++] = '"';
	}
}

static char* os_win_command_line(const char* const* argv) {
	size_t total = 1;
	size_t position = 0;
	char* commandLine;
	int index;

	for (index = 0; argv[index] != NULL; index++) {
		int quote = 0;
		total += os_win_quote_length(argv[index], &quote) + 1;
	}
	commandLine = (char*)malloc(total);
	if (commandLine == NULL) {
		return NULL;
	}
	for (index = 0; argv[index] != NULL; index++) {
		int quote = 0;
		(void)os_win_quote_length(argv[index], &quote);
		if (index > 0) {
			commandLine[position++] = ' ';
		}
		os_win_append_arg(commandLine, &position, argv[index], quote);
	}
	commandLine[position] = '\0';
	return commandLine;
}

static int os_win_start(const char* const* argv, HANDLE stdoutHandle, HANDLE stderrHandle, HANDLE* processHandle) {
	STARTUPINFOA startup;
	PROCESS_INFORMATION info;
	char* commandLine = os_win_command_line(argv);

	if (commandLine == NULL) {
		errno = ENOMEM;
		return -1;
	}
	ZeroMemory(&startup, sizeof startup);
	startup.cb = sizeof startup;
	startup.dwFlags = STARTF_USESTDHANDLES;
	startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startup.hStdOutput = stdoutHandle;
	startup.hStdError = stderrHandle;
	ZeroMemory(&info, sizeof info);
	if (!CreateProcessA(NULL, commandLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startup, &info)) {
		os_set_system_error();
		free(commandLine);
		return -1;
	}
	CloseHandle(info.hThread);
	*processHandle = info.hProcess;
	free(commandLine);
	return 0;
}

int os_run_capture(const char* const* argv, char* out, size_t outSize, int* exitCode) {
	SECURITY_ATTRIBUTES attributes;
	HANDLE readHandle = NULL;
	HANDLE writeHandle = NULL;
	HANDLE processHandle = NULL;
	DWORD waitResult;
	DWORD status = 0;
	char buffer[4096];
	size_t written = 0;
	int result = -1;

	if (argv == NULL || argv[0] == NULL || exitCode == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (out != NULL && outSize == 0) {
		errno = EINVAL;
		return -1;
	}
	if (outSize > 0 && out == NULL) {
		errno = EINVAL;
		return -1;
	}
	ZeroMemory(&attributes, sizeof attributes);
	attributes.nLength = sizeof attributes;
	attributes.bInheritHandle = TRUE;
	if (!CreatePipe(&readHandle, &writeHandle, &attributes, 0)) {
		os_set_system_error();
		return -1;
	}
	if (!SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0)) {
		os_set_system_error();
		CloseHandle(readHandle);
		CloseHandle(writeHandle);
		return -1;
	}
	if (os_win_start(argv, writeHandle, GetStdHandle(STD_ERROR_HANDLE), &processHandle) != 0) {
		CloseHandle(readHandle);
		CloseHandle(writeHandle);
		return -1;
	}
	CloseHandle(writeHandle);
	writeHandle = NULL;
	for (;;) {
		DWORD count = 0;
		if (!ReadFile(readHandle, buffer, (DWORD)sizeof buffer, &count, NULL)) {
			DWORD lastError = GetLastError();
			if (lastError != ERROR_BROKEN_PIPE && lastError != ERROR_HANDLE_EOF) {
				SetLastError(lastError);
				os_set_system_error();
				goto cleanup;
			}
			break;
		}
		if (count == 0) {
			break;
		}
		os_append_output(out, outSize, buffer, (size_t)count, &written);
	}
	waitResult = WaitForSingleObject(processHandle, INFINITE);
	if (waitResult != WAIT_OBJECT_0) {
		os_set_system_error();
		goto cleanup;
	}
	if (!GetExitCodeProcess(processHandle, &status)) {
		os_set_system_error();
		goto cleanup;
	}
	if (outSize > 0) {
		out[written] = '\0';
	}
	*exitCode = (int)status;
	result = 0;
cleanup:
	if (readHandle != NULL) {
		CloseHandle(readHandle);
	}
	if (writeHandle != NULL) {
		CloseHandle(writeHandle);
	}
	if (processHandle != NULL) {
		CloseHandle(processHandle);
	}
	return result;
}

int os_run(const char* const* argv, int* exitCode) {
	SECURITY_ATTRIBUTES attributes;
	HANDLE nullHandle;
	HANDLE processHandle = NULL;
	DWORD status = 0;
	int result = -1;

	if (argv == NULL || argv[0] == NULL || exitCode == NULL) {
		errno = EINVAL;
		return -1;
	}
	ZeroMemory(&attributes, sizeof attributes);
	attributes.nLength = sizeof attributes;
	attributes.bInheritHandle = TRUE;
	nullHandle = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &attributes, OPEN_EXISTING, 0, NULL);
	if (nullHandle == INVALID_HANDLE_VALUE) {
		os_set_system_error();
		return -1;
	}
	if (os_win_start(argv, nullHandle, nullHandle, &processHandle) != 0) {
		CloseHandle(nullHandle);
		return -1;
	}
	CloseHandle(nullHandle);
	if (WaitForSingleObject(processHandle, INFINITE) != WAIT_OBJECT_0) {
		os_set_system_error();
		goto cleanup;
	}
	if (!GetExitCodeProcess(processHandle, &status)) {
		os_set_system_error();
		goto cleanup;
	}
	*exitCode = (int)status;
	result = 0;
cleanup:
	if (processHandle != NULL) {
		CloseHandle(processHandle);
	}
	return result;
}

int os_process_id(void) {
	return (int)GetCurrentProcessId();
}

#else

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void os_child_abort(int statusWriteFd) {
	int code = errno;
	if (statusWriteFd >= 0) {
		ssize_t ignored = write(statusWriteFd, &code, sizeof code);
		(void)ignored;
	}
	_exit(127);
}

static pid_t os_spawn_child(const char* const* argv, int outFd, int errFd, int statusReadFd, int statusWriteFd) {
	pid_t pid = fork();

	if (pid < 0) {
		return -1;
	}
	if (pid == 0) {
		if (statusReadFd >= 0) {
			close(statusReadFd);
		}
		if (outFd >= 0 && outFd != STDOUT_FILENO) {
			if (dup2(outFd, STDOUT_FILENO) < 0) {
				os_child_abort(statusWriteFd);
			}
			close(outFd);
		}
		if (errFd >= 0) {
			if (errFd == outFd) {
				if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
					os_child_abort(statusWriteFd);
				}
			} else if (errFd != STDERR_FILENO) {
				if (dup2(errFd, STDERR_FILENO) < 0) {
					os_child_abort(statusWriteFd);
				}
				close(errFd);
			}
		}
		execvp(argv[0], (char* const*)argv);
		os_child_abort(statusWriteFd);
		_exit(127);
	}
	return pid;
}

static int os_wait_child(pid_t pid, int* status) {
	pid_t result;

	do {
		result = waitpid(pid, status, 0);
	} while (result < 0 && errno == EINTR);
	return (result < 0) ? -1 : 0;
}

static int os_read_exec_status(int statusReadFd, int* failed) {
	int code = 0;
	ssize_t count;

	*failed = 0;
	do {
		count = read(statusReadFd, &code, sizeof code);
	} while (count < 0 && errno == EINTR);
	if (count > 0) {
		*failed = 1;
		return code;
	}
	return 0;
}

static void os_decode_status(int status, int* exitCode) {
	if (WIFEXITED(status)) {
		*exitCode = WEXITSTATUS(status);
	} else {
		*exitCode = -1;
	}
}

int os_run_capture(const char* const* argv, char* out, size_t outSize, int* exitCode) {
	int outPipe[2];
	int statusPipe[2];
	pid_t pid;
	size_t written = 0;
	int status = 0;
	int readError = 0;
	int execError = 0;
	int execFailed = 0;
	char buffer[4096];

	if (argv == NULL || argv[0] == NULL || exitCode == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (out != NULL && outSize == 0) {
		errno = EINVAL;
		return -1;
	}
	if (outSize > 0 && out == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (pipe(outPipe) != 0) {
		return -1;
	}
	if (fcntl(outPipe[0], F_SETFD, FD_CLOEXEC) < 0) {
		int saved = errno;
		close(outPipe[0]);
		close(outPipe[1]);
		errno = saved;
		return -1;
	}
	if (pipe(statusPipe) != 0) {
		int saved = errno;
		close(outPipe[0]);
		close(outPipe[1]);
		errno = saved;
		return -1;
	}
	if (fcntl(statusPipe[1], F_SETFD, FD_CLOEXEC) < 0) {
		int saved = errno;
		close(outPipe[0]);
		close(outPipe[1]);
		close(statusPipe[0]);
		close(statusPipe[1]);
		errno = saved;
		return -1;
	}
	pid = os_spawn_child(argv, outPipe[1], -1, statusPipe[0], statusPipe[1]);
	if (pid < 0) {
		int saved = errno;
		close(outPipe[0]);
		close(outPipe[1]);
		close(statusPipe[0]);
		close(statusPipe[1]);
		errno = saved;
		return -1;
	}
	close(outPipe[1]);
	close(statusPipe[1]);
	for (;;) {
		ssize_t count = read(outPipe[0], buffer, sizeof buffer);
		if (count < 0) {
			if (errno == EINTR) {
				continue;
			}
			readError = errno;
			break;
		}
		if (count == 0) {
			break;
		}
		os_append_output(out, outSize, buffer, (size_t)count, &written);
	}
	close(outPipe[0]);
	execError = os_read_exec_status(statusPipe[0], &execFailed);
	close(statusPipe[0]);
	if (os_wait_child(pid, &status) != 0) {
		if (execFailed) {
			errno = execError;
			return -1;
		}
		if (readError != 0) {
			errno = readError;
			return -1;
		}
		return -1;
	}
	if (execFailed) {
		errno = execError;
		return -1;
	}
	if (readError != 0) {
		errno = readError;
		return -1;
	}
	if (outSize > 0) {
		out[written] = '\0';
	}
	os_decode_status(status, exitCode);
	return 0;
}

int os_run(const char* const* argv, int* exitCode) {
	int devNull;
	int statusPipe[2];
	pid_t pid;
	int status = 0;
	int execError = 0;
	int execFailed = 0;

	if (argv == NULL || argv[0] == NULL || exitCode == NULL) {
		errno = EINVAL;
		return -1;
	}
	devNull = open("/dev/null", O_WRONLY);
	if (devNull < 0) {
		return -1;
	}
	if (pipe(statusPipe) != 0) {
		int saved = errno;
		close(devNull);
		errno = saved;
		return -1;
	}
	if (fcntl(statusPipe[1], F_SETFD, FD_CLOEXEC) < 0) {
		int saved = errno;
		close(devNull);
		close(statusPipe[0]);
		close(statusPipe[1]);
		errno = saved;
		return -1;
	}
	pid = os_spawn_child(argv, devNull, devNull, statusPipe[0], statusPipe[1]);
	if (pid < 0) {
		int saved = errno;
		close(devNull);
		close(statusPipe[0]);
		close(statusPipe[1]);
		errno = saved;
		return -1;
	}
	close(devNull);
	close(statusPipe[1]);
	execError = os_read_exec_status(statusPipe[0], &execFailed);
	close(statusPipe[0]);
	if (os_wait_child(pid, &status) != 0) {
		if (execFailed) {
			errno = execError;
			return -1;
		}
		return -1;
	}
	if (execFailed) {
		errno = (execError != 0) ? execError : EIO;
		return -1;
	}
	os_decode_status(status, exitCode);
	return 0;
}

int os_process_id(void) {
	return (int)getpid();
}

#endif
