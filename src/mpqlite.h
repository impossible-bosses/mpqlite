#include "common.h"

struct String MpqliteRead(const char* mpqFilePath, const char* fileName);
int MpqliteWrite(const char* mpqFilePath, const char* fileName, const struct String data, int overwrite);
