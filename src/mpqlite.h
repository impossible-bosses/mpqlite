#include <stdbool.h>

#include "common.h"

struct String MpqliteRead(const char* mpqFilePath, const char* fileName);
bool MpqliteWrite(const char* mpqFilePath, const char* fileName, const struct String data);
