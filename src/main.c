#include <stdio.h>

#include "common.h"
#include "mpqlite.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        LOG_ERROR("mpqlite takes at least 1 argument: mpqlite <read/write> <other-args>\n");
        LOG_FLUSH();
        return 1;
    }

    const struct String MODE_READ = ToString("read");
    const struct String MODE_WRITE = ToString("write");
    const struct String mode = ToString(argv[1]);

    if (StringEqual(mode, MODE_READ)) {
        if (argc != 5) {
            LOG_ERROR("mpqlite extract takes 3 arguments: mpqlite read <MPQ path> <file-in-mpq> <output-path>\n");
            LOG_FLUSH();
            return 1;
        }

        const char* mpqFilePath = argv[2];
        const char* fileName = argv[3];
        const struct String result = MpqliteRead(mpqFilePath, fileName);
        if (result.str == NULL) {
            LOG_ERROR("Failed to extract file \"%s\" from MPQ \"%s\"\n", fileName, mpqFilePath);
            LOG_FLUSH();
            return 1;
        }

        const struct String outPath = ToString(argv[4]);
        if (!WriteEntireFile(outPath, result.size, result.str, false)) {
            LOG_ERROR("Failed to write to output file \"%.*s\"\n", outPath.size, outPath.str);
            LOG_FLUSH();
            return 1;
        }
        free(result.str);

        LOG_INFO("Output file \"%s\" from MPQ \"%s\" file \"%s\"\n", dstPath, mpqFilePath, fileName);
    }
    else if (StringEqual(mode, MODE_WRITE)) {
        if (argc != 5) {
            LOG_ERROR("mpqlite patch takes 3 arguments: mpqlite write <MPQ path> <file-in-mpq> <input-path>\n");
            LOG_FLUSH();
            return 1;
        }

        const char* mpqFilePath = argv[2];
        const char* fileName = argv[3];
        const char* srcPath = argv[4];
        if (!MpqliteWriteFile(mpqFilePath, fileName, srcPath)) {
            LOG_ERROR("Failed to patch file \"%s\" in MPQ \"%s\"\n", fileName, mpqFilePath);
            LOG_FLUSH();
            return 1;
        }
    }
    else {
        LOG_ERROR("mpqlite takes 1 mode argument: mpqlite <extract/patch> <other-args>\n");
        LOG_FLUSH();
        return 1;
    }

    LOG_FLUSH();
    return 0;
}

#include "common.c"
#include "mpqlite.c"

#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>
#undef STB_SPRINTF_IMPLEMENTATION
