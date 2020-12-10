#include "common.h"

#if APP_WIN32
#include <Windows.h>

#elif APP_LINUX
#include <stdlib.h>
#include <sys/stat.h>

#else
#error "Unsupported platform"
#endif

const struct String emptyString = {0};

bool CharIsWhitespace(char c)
{
    return c == '\n' || c == '\r' || c == '\t' || c == ' ';
}

struct String ToString(const char* cString)
{
    const struct String result = {
        .size = strlen(cString),
        .str = (char*)cString,
    };
    return result;
}

bool ToCString(const struct String string, size_t bufferSize, char buffer[])
{
    if (string.size >= bufferSize) {
        return false;
    }

    memcpy(buffer, string.str, string.size);
    buffer[string.size] = '\0';
    return true;
}

bool StringEqual(const struct String string1, const struct String string2)
{
    if (string1.size != string2.size) {
        return false;
    }

    for (size_t i = 0; i < string1.size; i++) {
        if (string1.str[i] != string2.str[i]) {
            return false;
        }
    }

    return true;
}

bool StringEqualCaseInsensitive(const struct String string1, const struct String string2)
{
    if (string1.size != string2.size) {
        return false;
    }

    for (size_t i = 0; i < string1.size; i++) {
        char c1 = string1.str[i];
        if ('A' <= c1 && c1 <= 'Z') {
            c1 = c1 - 'A' + 'a';
        }
        char c2 = string2.str[i];
        if ('A' <= c2 && c2 <= 'Z') {
            c2 = c2 - 'A' + 'a';
        }
        if (c1 != c2) {
            return false;
        }
    }

    return true;
}

struct String StringSlice(const struct String string, size_t start, size_t end)
{
    DEBUG_ASSERT(start < string.size);
    DEBUG_ASSERT(end <= string.size);
    DEBUG_ASSERT(end >= start);

    struct String result = {
        .size = end - start,
        .str = string.str + start, 
    };
    return result;
}

size_t StringFindFirstFrom(const struct String string, char c, size_t start)
{
    for (size_t i = start; i < string.size; i++) {
        if (string.str[i] == c) {
            return i;
        }
    }

    return string.size;
}

size_t StringFindFirst(const struct String string, char c)
{
    return StringFindFirstFrom(string, c, 0);
}

// TODO slow, naive implementation
size_t StringFindFirstSubstring(const struct String string, const struct String substring)
{
    for (size_t i = 0; i < string.size; i++) {
        bool match = true;
        for (size_t j = 0; j < substring.size; j++) {
            const size_t ind = i + j;
            if (ind >= string.size) {
                match = false;
                break;
            }
            if (string.str[ind] != substring.str[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return i;
        }
    }

    return string.size;
}

struct String StringTrim(const struct String string)
{
    int newStart = 0;
    while (newStart < string.size && CharIsWhitespace(string.str[newStart])) {
        newStart++;
    }
    if (newStart >= string.size) {
        return emptyString;
    }

    int newEnd = (int)string.size - 1;
    while (newEnd >= 0 && CharIsWhitespace(string.str[newEnd])) {
        newEnd--;
    }
    if (newEnd < 0) {
        return emptyString;
    }

    return StringSlice(string, newStart, newEnd);
}

struct String StringNextInSplit(struct String* string, char sep)
{
    if (string->size == 0) {
        struct String next = {0};
        return next;
    }

    const size_t nextInd = StringFindFirst(*string, sep);
    struct String next = {
        .size = nextInd,
        .str = string->str
    };

    if (nextInd != string->size) {
        string->size--;
    }
    string->size -= nextInd;
    string->str += nextInd + 1;

    return next;
}

bool StringToIntBase10(struct String string, int* intBase10)
{
    if (string.size == 0) {
        return false;
    }

    bool negative = false;
    *intBase10 = 0;
    for (uint32_t i = 0; i < string.size; i++) {
        const char c = string.str[i];
        if (i == 0 && c == '-') {
            negative = true;
            continue;
        }
        if (c < '0' || c > '9') {
            return false;
        }
        *intBase10 = (*intBase10) * 10 + (int)(c - '0');
    }

    if (negative) {
        *intBase10 = -(*intBase10);
    }
    return true;
}

bool StringToFloat(struct String string, float* f)
{
    size_t dotIndex = 0;
    while (dotIndex < string.size && string.str[dotIndex] != '.') {
        dotIndex++;
    }

    int whole = 0;
    bool wholeNegative = false;
    if (dotIndex > 0) {
        const struct String stringWhole = StringSlice(string, 0, dotIndex);
        if (!StringToIntBase10(stringWhole, &whole)) {
            return false;
        }
        wholeNegative = string.str[0] == '-';
    }

    *f = (float)whole;

    int frac = 0;
    if (dotIndex + 1 < string.size) {
        const struct String fracString = StringSlice(string, dotIndex + 1, string.size);
        if (!StringToIntBase10(fracString, &frac)) {
            return false;
        }

        frac = wholeNegative ? -frac : frac;
        float fractional = (float)frac;
        for (size_t i = 0; i < fracString.size; i++) {
            fractional /= 10.0f;
        }
        *f += fractional;
    }

    return true;
}

uint32_t SafeTruncateUInt64(uint64_t value)
{
    DEBUG_ASSERT(value <= 0xFFFFFFFF);
    uint32_t result = (uint32_t)value;
    return result;
}

struct String ReadEntireFile(const struct String path)
{
    struct String result = {
        .size = 0,
        .str = NULL
    };

    char pathC[MAX_PATH];
    if (!ToCString(path, MAX_PATH, pathC)) {
        return result;
    }

#if APP_WIN32
    HANDLE hFile = CreateFile(pathC, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return result;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        return result;
    }

    uint32_t fileSize32 = SafeTruncateUInt64(fileSize.QuadPart);
    result.str = malloc(fileSize32);
    if (result.str == NULL) {
        return result;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, result.str, fileSize32, &bytesRead, NULL) || fileSize32 != bytesRead) {
        free(result.str);
        result.str = NULL;
        return result;
    }

    result.size = fileSize32;
    CloseHandle(hFile);

#elif APP_LINUX
    FILE* filePtr = fopen(pathC, "rb");
    if (filePtr == NULL) {
        return result;
    }

    fseek(filePtr, 0, SEEK_END);
    const size_t size = ftell(filePtr);
    rewind(filePtr);

    result.str = malloc(size);
    if (result.str == NULL) {
        return result;
    }

    if (fread(result.str, size, 1, filePtr) != 1) {
        free(result.str);
        result.str = NULL;
        return result;
    }

    result.size = size;
    fclose(filePtr);

#endif

    return result;
}

bool WriteEntireFile(const struct String path, size_t size, const void* data, bool append)
{
    char pathC[MAX_PATH];
    if (!ToCString(path, MAX_PATH, pathC)) {
        return false;
    }

#if APP_WIN32
    HANDLE hFile = CreateFile(pathC, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (append) {
        DWORD dwPos = SetFilePointer(hFile, 0, NULL, FILE_END);
        if (dwPos == INVALID_SET_FILE_POINTER) {
            return false;
        }
    }
    else {
        DWORD dwPos = SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        if (dwPos == INVALID_SET_FILE_POINTER) {
            return false;
        }
        if (SetEndOfFile(hFile) == 0) {
            return false;
        }
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, data, (DWORD)size, &bytesWritten, NULL)) {
        return false;
    }

    CloseHandle(hFile);
    return bytesWritten == (DWORD)size;

#elif APP_LINUX
    FILE* filePtr;
    if (append) {
        filePtr = fopen(pathC, "ab");
    }
    else {
        filePtr = fopen(pathC, "w");
    }

    const size_t written = fwrite(data, size, 1, filePtr);
    fclose(filePtr);

    return written == 1;

#endif
}

bool KmDeleteFile(const struct String path, bool failIfNotFound)
{
    char pathC[MAX_PATH];
    if (!ToCString(path, MAX_PATH, pathC)) {
        return false;
    }

#if APP_WIN32
    const BOOL result = DeleteFile(pathC);
    if (result == 0) {
        const DWORD error = GetLastError();
        if (error != ERROR_FILE_NOT_FOUND) {
            return false;
        }
    }

    return true;

#elif APP_LINUX
    LOG_ERROR("Unimplemented on linux\n");
    return false;

#endif
}

bool KmCopyFile(const struct String srcPath, const struct String dstPath, bool failIfExists)
{
    char srcPathC[MAX_PATH];
    if (!ToCString(srcPath, MAX_PATH, srcPathC)) {
        return false;
    }
    char dstPathC[MAX_PATH];
    if (!ToCString(dstPath, MAX_PATH, dstPathC)) {
        return false;
    }

#if APP_WIN32
    const BOOL result = CopyFile(srcPathC, dstPathC, failIfExists);
    return result != 0;

#elif APP_LINUX
    LOG_ERROR("Unimplemented on linux\n");
    return false;

#endif
}

bool CreateDirRecursive(const struct String path)
{
#if APP_WIN32
    char pathBuf[MAX_PATH];
    size_t nextSlash = 0;
    while (true) {
        nextSlash = StringFindFirstFrom(path, '/', nextSlash + 1);
        if (nextSlash == path.size) {
            break;
        }

        const struct String partialPath = StringSlice(path, 0, nextSlash);
        memcpy(pathBuf, partialPath.str, partialPath.size);
        pathBuf[partialPath.size] = '\0';
        const BOOL result = CreateDirectoryA(partialPath.str, NULL);
        if (result == 0) {
            const DWORD error = GetLastError();
            if (error != ERROR_ALREADY_EXISTS) {
                return false;
            }
        }
    }

    return true;

#elif APP_LINUX
    LOG_ERROR("Unimplemented on linux\n");
    return false;

#endif
}

struct String StringAlloc(size_t size)
{
    struct String result = { .size = size, .str = malloc(size) };
    return result;
}

void StringFree(const struct String string)
{
    free(string.str);
}
