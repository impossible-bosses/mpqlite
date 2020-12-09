#pragma once

#include <stdint.h>

#define KILOBYTES(bytes) ((bytes) * 1024LL)
#define MEGABYTES(bytes) (KILOBYTES(bytes) * 1024LL)
#define GIGABYTES(bytes) (MEGABYTES(bytes) * 1024LL)

// https://stackoverflow.com/questions/4415524/common-array-length-macro-for-c
#define C_ARRAY_LENGTH(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define LOG_ERROR(format, ...) fprintf(stderr, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) fprintf(stdout, format, ##__VA_ARGS__)
#define LOG_FLUSH() fflush(stderr); fflush(stdout)

#define DEBUG_PANIC(format, ...) \
LOG_ERROR("PANIC! %s:%d (%s)\n", __FILE__, __LINE__, __func__); \
LOG_ERROR(format, ##__VA_ARGS__); \
LOG_FLUSH(); \
*(int*)0 = 0

#define DEBUG_ASSERTF(expression, format, ...) if (!(expression)) { \
LOG_ERROR("Assertion failed at %s:%d (%s)\n", __FILE__, __LINE__, __func__); \
DEBUG_PANIC(format, ##__VA_ARGS__); }
#define DEBUG_ASSERT(expression) DEBUG_ASSERTF(expression, "")

struct String
{
	size_t size;
	char* str;
};

bool CharIsWhitespace(char c);

struct String ToString(const char* cString);
bool ToCString(const struct String string, size_t bufferSize, char buffer[]);

bool StringEqual(const struct String string1, const struct String string2);
bool StringEqualCaseInsensitive(const struct String string1, const struct String string2);

struct String StringSlice(const struct String string, size_t start, size_t end);
size_t StringFindFirst(const struct String string, char c);
size_t StringFindFirstFrom(const struct String string, char c, size_t start);
size_t StringFindFirstSubstring(const struct String string, const struct String substring);
struct String StringTrim(const struct String string);

struct String StringNextInSplit(struct String* string, char sep);

bool StringToIntBase10(struct String string, int* intBase10);
bool StringToFloat(struct String string, float* f);

struct String ReadEntireFile(const struct String path);
bool WriteEntireFile(const struct String path, size_t size, const void* data, bool append);

bool KmDeleteFile(const struct String path, bool failIfNotFound);
bool KmCopyFile(const struct String srcPath, const struct String dstPath, bool failIfExists);

bool CreateDirRecursive(const struct String path);

struct String StringAlloc(size_t size);
void StringFree(const struct String string);
