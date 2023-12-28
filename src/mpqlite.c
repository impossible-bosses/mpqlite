#include "mpqlite.h"

#include <stdio.h>
#include <StormLib.h>

struct String MpqliteRead(const char* mpqFilePath, const char* fileName)
{
    struct String result = {
        .size = 0,
        .str = NULL
    };

    HANDLE hMpq;
    if (!SFileOpenArchive(mpqFilePath, 0, 0, &hMpq)) {
        LOG_ERROR("Failed to open MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
        return result;
    }

    SFILE_FIND_DATA findData;
    HANDLE hFindResult = SFileFindFirstFile(hMpq, "*", &findData, NULL);
    if (hFindResult == NULL) {
        LOG_ERROR("No files found in MPQ archive %s\n", mpqFilePath);
        return result;
    }

    while (true) {
        if (strcmp(findData.cFileName, fileName) == 0) {
            HANDLE hFile;
            if (!SFileOpenFileEx(hMpq, fileName, 0, &hFile)) {
                LOG_ERROR("Failed to open MPQ archive file %s, error %lu\n",
                          fileName, GetLastError());
                return result;
            }

            result.size = findData.dwFileSize;
            result.str = (char*)malloc(result.size);
            if (result.str == NULL) {
                LOG_ERROR("Failed to allocate memory during MPQ read\n");
                return result;
            }

            DWORD bytesRead;
            if (!SFileReadFile(hFile, result.str, result.size, &bytesRead, 0)) {
                LOG_ERROR("Failed to read MPQ archive file %s, error %lu\n",
                          fileName, GetLastError());
                if (!SFileCloseFile(hFile)) {
                    LOG_ERROR("Failed to close MPQ archive file %s, error %lu\n",
                              fileName, GetLastError());
                }
                free(result.str);
                result.str = NULL;
                return result;
            }

            if (bytesRead != result.size) {
                LOG_ERROR("Failed to read entire MPQ archive file %s\n, bytesRead %lu",
                          fileName, bytesRead);
                free(result.str);
                result.str = NULL;
                return result;
            }

            if (!SFileCloseFile(hFile)) {
                LOG_ERROR("Failed to close MPQ archive file %s, error %lu\n",
                          fileName, GetLastError());
            }

            break;
        }

        if (!SFileFindNextFile(hFindResult, &findData)) {
            break;
        }
    }

    if (!SFileCloseArchive(hMpq)) {
        LOG_ERROR("Failed to close MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
    }

    if (result.str == NULL) {
        LOG_ERROR("File \"%s\" not found in MPQ \"%s\"\n", fileName, mpqFilePath);
        return result;
    }

    return result;
}

int MpqliteWrite(const char* mpqFilePath, const char* fileName, const struct String data, int overwrite)
{
    HANDLE hMpq;
    const int result = SFileOpenArchive(mpqFilePath, 0, 0, &hMpq);
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
            if (!overwrite) {
                LOG_ERROR("File %s already in MPQ archive %s, not overwriting\n", fileName, mpqFilePath);
                if (!SFileCloseArchive(hMpq)) {
                    LOG_ERROR("Failed to close MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
                }
                return false;
            }

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
    if (!SFileCreateFile(hMpq, fileName, 0, (DWORD)data.size, 0, dwFlags, &hNewFile)) {
        LOG_ERROR("Failed to re-create MPQ archive file %s, error %lu\n", fileName, GetLastError());
        return false;
    }

    if (!SFileWriteFile(hNewFile, data.str, (DWORD)data.size, MPQ_COMPRESSION_BZIP2)) {
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

    return true;
}
