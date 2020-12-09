#include <stdio.h>

#include <stb_sprintf.h>
#include <StormLib.h>

#include "common.h"

bool MpqliteWriteFile(const char* mpqFilePath, const char* fileName, const char* srcPath)
{
    const struct String srcPathString = ToString(srcPath);
    struct String srcData = ReadEntireFile(srcPathString);
    if (srcData.str == NULL) {
        LOG_ERROR("Failed to read patch source file %s\n", srcPath);
        return false;
    }

    HANDLE hMpq;
    const bool result = SFileOpenArchive(mpqFilePath, 0, 0, &hMpq);
    if (!result) {
        LOG_ERROR("Failed to open MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
        return false;
    }

    SFILE_FIND_DATA findData;
    HANDLE hFindResult = SFileFindFirstFile(hMpq, "*", &findData, NULL);
    if (hFindResult == NULL) {
        LOG_ERROR("No files found in MPQ archive %s\n", mpqFilePath);
        return false;
    }

    while (true) {
        if (strcmp(findData.cFileName, fileName) == 0) {
            if (!SFileRemoveFile(hMpq, fileName, 0)) {
                LOG_ERROR("Failed to remove MPQ archive file for overwrite: %s, error %lu\n",
                          fileName, GetLastError());
                return false;
            }
        }

        if (!SFileFindNextFile(hFindResult, &findData)) {
            break;
        }
    }

    DWORD dwFlags = MPQ_FILE_COMPRESS;
    // if (encrypt) {
    //     dwFlags |= MPQ_FILE_ENCRYPTED;
    // }
    HANDLE hNewFile;
    if (!SFileCreateFile(hMpq, fileName, 0, (DWORD)srcData.size, 0, dwFlags, &hNewFile)) {
        LOG_ERROR("Failed to re-create MPQ archive file %s, error %lu\n", fileName, GetLastError());
        return false;
    }

    if (!SFileWriteFile(hNewFile, srcData.str, (DWORD)srcData.size, MPQ_COMPRESSION_BZIP2)) {
        LOG_ERROR("Failed to write MPQ archive file %s, error %lu\n", fileName, GetLastError());
        return false;
    }

    if (!SFileFinishFile(hNewFile)) {
        LOG_ERROR("Failed to call finish MPQ archive file %s, error %lu\n",
                  fileName, GetLastError());
        return false;
    }

    if (!SFileCloseArchive(hMpq)) {
        LOG_ERROR("Failed to close MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
    }

    LOG_INFO("Wrote file \"%s\" into MPQ \"%s\" file \"%s\"\n", srcPath, mpqFilePath, fileName);
    return true;
}

bool MpqliteReadFile(const char* mpqFilePath, const char* fileName, const char* dstPath)
{
    HANDLE hMpq;
    const bool result = SFileOpenArchive(mpqFilePath, 0, 0, &hMpq);
    if (!result) {
        LOG_ERROR("Failed to open MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
        return false;
    }

    SFILE_FIND_DATA findData;
    HANDLE hFindResult = SFileFindFirstFile(hMpq, "*", &findData, NULL);
    if (hFindResult == NULL) {
        LOG_ERROR("No files found in MPQ archive %s\n", mpqFilePath);
        return false;
    }

    const size_t bufferSize = MEGABYTES(128);
    size_t fileSize = 0;
    void* buffer = malloc(bufferSize);
    bool found = false;

    while (true) {
        if (findData.dwFileSize > bufferSize) {
            LOG_ERROR("fileSize (%lu) > bufferSize (%llu)\n", findData.dwFileSize, bufferSize);
            return false;
        }

        if (strcmp(findData.cFileName, fileName) == 0) {
            HANDLE hFile;
            if (!SFileOpenFileEx(hMpq, fileName, 0, &hFile)) {
                LOG_ERROR("Failed to open MPQ archive file %s, error %lu\n",
                          fileName, GetLastError());
                return false;
            }

            DWORD bytesRead;
            if (!SFileReadFile(hFile, buffer, findData.dwFileSize, &bytesRead, 0)) {
                LOG_ERROR("Failed to read MPQ archive file %s, error %lu\n",
                          fileName, GetLastError());
                if (!SFileCloseFile(hFile)) {
                    LOG_ERROR("Failed to close MPQ archive file %s, error %lu\n",
                              fileName, GetLastError());
                }
                return false;
            }

            if (bytesRead != findData.dwFileSize) {
                LOG_ERROR("Failed to read entire MPQ archive file %s\n, bytesRead %lu",
                          fileName, bytesRead);
                return false;
            }

            if (!SFileCloseFile(hFile)) {
                LOG_ERROR("Failed to close MPQ archive file %s, error %lu\n",
                          fileName, GetLastError());
            }

            fileSize = findData.dwFileSize;
            found = true;
            break;
        }

        if (!SFileFindNextFile(hFindResult, &findData)) {
            break;
        }
    }

    if (!SFileCloseArchive(hMpq)) {
        LOG_ERROR("Failed to close MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
    }

    if (!found) {
        LOG_ERROR("File \"%s\" not found in MPQ \"%s\"\n", fileName, mpqFilePath);
        return false;
    }

    const struct String outPath = ToString(dstPath);
    if (!WriteEntireFile(outPath, fileSize, buffer, false)) {
        LOG_ERROR("Failed to write to output file \"%s\"\n", dstPath);
        return false;
    }

    free(buffer);

    LOG_INFO("Output file \"%s\" from MPQ \"%s\" file \"%s\"\n", dstPath, mpqFilePath, fileName);
    return true;
}

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
        const char* dstPath = argv[4];
        if (!MpqliteReadFile(mpqFilePath, fileName, dstPath)) {
            LOG_ERROR("Failed to extract file \"%s\" from MPQ \"%s\"\n", fileName, mpqFilePath);
            LOG_FLUSH();
            return 1;
        }
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

#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>
#undef STB_SPRINTF_IMPLEMENTATION
