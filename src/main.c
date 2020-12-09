#include <stdio.h>

#include <stb_sprintf.h>
#include <StormLib.h>

#include "common.h"

bool MpqliteExtractFile(const char* mpqFilePath, const char* fileName)
{
    LOG_INFO("Extracting file \"%s\" from MPQ \"%s\"\n", fileName, mpqFilePath);

    HANDLE hMpq;
    const bool result = SFileOpenArchive(mpqFilePath, 0, 0, &hMpq);
    if (!result) {
        LOG_ERROR("Failed to open MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
        return false;
    }

    HANDLE hFile;
    if (!SFileOpenFileEx(hMpq, fileName, 0, &hFile)) {
        LOG_ERROR("Failed to open MPQ archive file %s, error %lu\n", fileName, GetLastError());
        return false;
    }

    DWORD fileSize;
    if (!SFileGetFileSize(hFile, &fileSize)) {
        LOG_ERROR("Failed to get MPQ archive file size, file %s, error %lu\n", fileName, GetLastError());
        return false;
    }

    LOG_INFO("size %d\n", fileSize);

    /*DWORD bytesRead;
    if (!SFileReadFile(hFile, buffer, findData.dwFileSize, &bytesRead, 0)) {
        LOG_ERROR("Failed to read MPQ archive file %s, error %lu\n",
                  fileName, GetLastError());
        if (!SFileCloseFile(hFile)) {
            LOG_ERROR("Failed to close MPQ archive file %s, error %lu\n",
                      fileName, GetLastError());
        }
        return false;
    }*/

    if (!SFileCloseFile(hFile)) {
        LOG_ERROR("Failed to close MPQ archive file %s, error %lu\n", fileName, GetLastError());
    }

    if (!SFileCloseArchive(hMpq)) {
        LOG_ERROR("Failed to close MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
    }

    return true;
}

#if 0
bool ExtractMapData(const struct String mpqFilePath, const struct String abilityDataPath)
{
    char mpqFilePathC[MAX_PATH];
    if (!ToCString(mpqFilePath, MAX_PATH, mpqFilePathC)) {
        LOG_ERROR("Failed to convert MPQ path to C string: %.*s\n",
            (int)mpqFilePath.size, mpqFilePath.str);
        return false;
    }

    HANDLE hMpq;
    const bool result = SFileOpenArchive(mpqFilePathC, 0, 0, &hMpq);
    if (!result) {
        LOG_ERROR("Failed to open MPQ archive %.*s, error %lu\n",
            (int)mpqFilePath.size, mpqFilePath.str, GetLastError());
        return false;
    }

    SFILE_FIND_DATA findData;
    HANDLE hFindResult = SFileFindFirstFile(hMpq, "*", &findData, NULL);
    if (hFindResult == NULL) {
        LOG_ERROR("No files found in MPQ archive %.*s\n", (int)mpqFilePath.size, mpqFilePath.str);
        return false;
    }

    size_t numAbilities = 0;
    struct Abilities* abilities = malloc(sizeof(struct Abilities));
    memset(abilities, 0, sizeof(struct Abilities));

    const size_t bufferSize = MEGABYTES(128);
    void* buffer = malloc(bufferSize);

    size_t abilitiesTxtSize = 0;
    void* abilitiesTxtData = NULL;
    size_t abilitiesMapTxtSize = 0;
    void* abilitiesMapTxtData = NULL;
    size_t abilitiesBinSize = 0;
    void* abilitiesBinData = NULL;

    while (true) {
        const char* fileName = findData.cFileName;
        const DWORD fileSize = findData.dwFileSize;
        if (fileSize > bufferSize) {
            LOG_ERROR("fileSize (%lu) > bufferSize (%llu)\n", fileSize, bufferSize);
            return false;
        }

        enum MapMpqFile file = MPQFILE_COUNT;
        for (int i = 0; i < C_ARRAY_LENGTH(MAP_MPQ_FILE_INFO); i++) {
            if (strcmp(MAP_MPQ_FILE_INFO[i].mpqPath, fileName) == 0) {
                file = (enum MapMpqFile)i;
                break;
            }
        }

        if (file != MPQFILE_COUNT) {
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

            if (!SFileCloseFile(hFile)) {
                LOG_ERROR("Failed to close MPQ archive file %s, error %lu\n",
                          fileName, GetLastError());
            }

            if (bytesRead != fileSize) {
                LOG_ERROR("Failed to read entire MPQ archive file %s\n, bytesRead %lu",
                          fileName, bytesRead);
                return false;
            }

            switch (file) {
                case MPQFILE_ABILITIES_TXT: {
                    abilitiesTxtSize = (size_t)fileSize;
                    abilitiesTxtData = malloc(fileSize);
                    memcpy(abilitiesTxtData, buffer, fileSize);
                } break;
                case MPQFILE_ABILITIES_MAP_TXT: {
                    abilitiesMapTxtSize = (size_t)fileSize;
                    abilitiesMapTxtData = malloc(fileSize);
                    memcpy(abilitiesMapTxtData, buffer, fileSize);
                } break;
                case MPQFILE_ABILITIES_BIN: {
                    abilitiesBinSize = (size_t)fileSize;
                    abilitiesBinData = malloc(fileSize);
                    memcpy(abilitiesBinData, buffer, fileSize);
                } break;
                default: {
                } break;
            }

            char outPathBuf[MAX_PATH];
            const int outPathLen = stbsp_snprintf(outPathBuf, MAX_PATH, "%.*s/%s",
                (int)PATH_DIR_EXTRACTED.size, PATH_DIR_EXTRACTED.str,
                MAP_MPQ_FILE_INFO[file].extractPath);
            if (outPathLen < 0 || outPathLen > MAX_PATH) {
                LOG_ERROR("Failed to create output path for MPQ file extraction\n");
                return false;
            }

            struct String outPath;
            outPath.size = outPathLen;
            outPath.str = outPathBuf;
            LOG_INFO("Extracting \"%s\" to \"%.*s\"\n", fileName, (int)outPath.size, outPath.str);
            if (!WriteEntireFile(outPath, fileSize, buffer, false)) {
                LOG_ERROR("Failed to extract\n");
            }
        }

        if (!SFileFindNextFile(hFindResult, &findData)) {
            break;
        }
    }

    free(buffer);

    if (!SFileFindClose(hFindResult)) {
        LOG_ERROR("Failed to close MPQ find result handle for %.*s\n", (int)mpqFilePath.size, mpqFilePath.str);
        return false;
    }

    if (!SFileCloseArchive(hMpq)) {
        LOG_ERROR("Failed to close MPQ archive %.*s, error %lu\n", (int)mpqFilePath.size, mpqFilePath.str, GetLastError());
    }

    if (!ExtractAbilitiesText(abilitiesTxtSize, abilitiesTxtData, &numAbilities, abilities)) {
        LOG_ERROR("Failed to load ability text data\n");
        return false;
    }

    if (!ExtractAbilityMapsText(abilitiesMapTxtSize, abilitiesMapTxtData, &numAbilities, abilities)) {
        LOG_ERROR("Failed to load ability mapping text data\n");
        return false;
    }

    if (!ExtractAbilitiesBinary(abilitiesBinSize, abilitiesBinData, &numAbilities, abilities)) {
        LOG_ERROR("Failed to load ability binary data\n");
        return false;
    }

    const int abiBufMaxSize = MEGABYTES(128);
    int abiBufSize;
    char* abiBuf = malloc(abiBufMaxSize);
    if (!SaveAbilities(numAbilities, abilities, abiBufMaxSize, &abiBufSize, abiBuf)) {
        LOG_ERROR("Failed to save abilities to string\n");
        return false;
    }

    LOG_INFO("Writing all parsed ability data to %.*s\n",
        (int)abilityDataPath.size, abilityDataPath.str);
    if (!WriteEntireFile(abilityDataPath, abiBufSize, abiBuf, false)) {
        LOG_ERROR("Failed to write all abilities file %.*s\n",
            (int)abilityDataPath.size, abilityDataPath.str);
        return false;
    }

    free(abiBuf);
    free(abilities);

    return true;
}

int PatchString(struct String* dst, size_t iStart, size_t iEnd, const struct String src)
{
    DEBUG_ASSERT(iStart < iEnd);

    const size_t sizeEnd = dst->size - iEnd;
    memmove(dst->str + iStart + src.size, dst->str + iEnd, sizeEnd);
    memcpy(dst->str + iStart, src.str, src.size);
    dst->size = iStart + src.size + sizeEnd;

    return (int)src.size - (int)(iEnd - iStart);
}

bool PatchAbilitiesText(size_t maxSize, size_t* size, char* buffer,
                        size_t numAbilities, const struct Abilities* abilities)
{
    const int tempBufferMaxSize = MEGABYTES(1);
    char* tempBuffer = malloc(tempBufferMaxSize);

    struct String string = {
        .size = *size,
        .str = buffer
    };

    while (string.size > 0) {
        const struct String line = StringNextInSplit(&string, '\n');
        if (line.size == 0) {
            continue;
        }

        if (line.str[line.size - 1] == '\r') {
            LOG_ERROR("Detected CR line ending\n");
            return false;
        }

        char id[4];
        if (IsLineTextId(line, id) && id[0] == 'A') {
            const struct AbilityData* patchData = NULL;
            for (size_t i = 0; i < numAbilities; i++) {
                if (strncmp(id, abilities->abilityData[i].id, 4) == 0) {
                    patchData = &abilities->abilityData[i];
                }
            }


            if (patchData != NULL) {
                const size_t nextAbilityInd = StringFindFirst(string, '[');

                if (patchData->hotkey != 0) {
                    const struct String keyHotkey = ToString("Hotkey=");
                    const size_t hotkeyInd = StringFindFirstSubstring(string, keyHotkey);
                    if (hotkeyInd < nextAbilityInd) {
                        const size_t hotkeyCharInd = hotkeyInd + keyHotkey.size;
                        if (hotkeyCharInd >= string.size) {
                            LOG_ERROR("Bad hotkey keyword for %.*s\n", 4, id);
                            return false;
                        }

                        const char hotkeyCurrent = string.str[hotkeyCharInd];
                        if (hotkeyCurrent != '\n') {
                            if (hotkeyCurrent < 'A' || hotkeyCurrent > 'Z') {
                                LOG_ERROR("Invalid hotkey %c for %.*s\n", hotkeyCurrent, 4, id);
                                return false;
                            }

                            if (hotkeyCurrent != patchData->hotkey) {
                                string.str[hotkeyCharInd] = patchData->hotkey;
                                LOG_INFO("%.*s patched hotkey (%c -> %c)\n", 4, id,
                                         hotkeyCurrent, patchData->hotkey);
                            }
                        }
                    }
                }

                if (patchData->buttonX != -1 && patchData->buttonY != -1) {
                    const struct String keyButtonpos = ToString("Buttonpos=");
                    const size_t buttonposInd = StringFindFirstSubstring(string, keyButtonpos);
                    if (buttonposInd < nextAbilityInd) {
                        const size_t buttonposStartInd = buttonposInd + keyButtonpos.size;
                        const size_t buttonposEndInd = StringFindFirst(StringSlice(string, buttonposStartInd, string.size), '\n') + buttonposStartInd;
                        const struct String buttonpos = StringSlice(string, buttonposStartInd, buttonposEndInd);

                        const int tempSize = stbsp_snprintf(tempBuffer, tempBufferMaxSize,
                            "%d,%d", patchData->buttonX, patchData->buttonY);
                        if (tempSize < 0 || tempSize >= tempBufferMaxSize) {
                            LOG_ERROR("Button string print failed");
                            return false;
                        }
                        struct String newButtonpos;
                        newButtonpos.size = tempSize;
                        newButtonpos.str = tempBuffer;

                        if (!StringEqual(buttonpos, newButtonpos)) {
                            const int delta = PatchString(&string, buttonposStartInd, buttonposEndInd, newButtonpos);
                            if (*size + delta > maxSize) {
                                LOG_ERROR("PatchString exceeded max size\n");
                                return false;
                            }
                            *size += delta;
                            LOG_INFO("%.*s patched buttonpos\n", 4, id);
                        }
                    }
                }

                if (patchData->name.size > 0) {
                    const struct String keyName = ToString("Name=");
                    const size_t nameInd = StringFindFirstSubstring(string, keyName);
                    if (nameInd < nextAbilityInd) {
                        const size_t nameStartInd = nameInd + keyName.size;
                        const size_t nameEndInd = StringFindFirst(StringSlice(string, nameStartInd, string.size), '\n') + nameStartInd;

                        const struct String name = StringSlice(string, nameStartInd, nameEndInd);
                        if (!StringEqual(name, patchData->name)) {
                            const int delta = PatchString(&string, nameStartInd, nameEndInd, patchData->name);
                            if (*size + delta > maxSize) {
                                LOG_ERROR("PatchString exceeded max size\n");
                                return false;
                            }
                            *size += delta;
                            LOG_INFO("%.*s patched name\n", 4, id);
                        }
                    }
                }

                if (patchData->tip[0].size > 0) {
                    const struct String keyTip = ToString("Tip=");
                    const size_t tipInd = StringFindFirstSubstring(string, keyTip);
                    if (tipInd < nextAbilityInd) {
                        const size_t tipStartInd = tipInd + keyTip.size;
                        const size_t tipEndInd = StringFindFirst(StringSlice(string, tipStartInd, string.size), '\n') + tipStartInd;
                        const struct String tipString = StringSlice(string, tipStartInd, tipEndInd);

                        const bool quote = StringFindFirst(patchData->tip[0], '"') == patchData->tip[0].size && StringFindFirst(patchData->tip[0], ',') != patchData->tip[0].size;
                        int tempBufferSize = 0;
                        if (quote) {
                            Output(tempBuffer, tempBufferMaxSize, &tempBufferSize, "\"%.*s\"",
                                   (int)patchData->tip[0].size, patchData->tip[0].str);
                        }
                        else {
                            Output(tempBuffer, tempBufferMaxSize, &tempBufferSize, "%.*s",
                                   (int)patchData->tip[0].size, patchData->tip[0].str);
                        }

                        if (patchData->tip[1].size > 0) {
                            Output(tempBuffer, tempBufferMaxSize, &tempBufferSize, ",%.*s",
                                   (int)patchData->tip[1].size, patchData->tip[1].str);
                            if (patchData->tip[2].size > 0) {
                                Output(tempBuffer, tempBufferMaxSize, &tempBufferSize, ",%.*s",
                                       (int)patchData->tip[2].size, patchData->tip[2].str);
                            }
                        }

                        const struct String newTipString = {
                            .size = tempBufferSize,
                            .str = tempBuffer
                        };

                        if (!StringEqual(tipString, newTipString)) {
                            const int delta = PatchString(&string, tipStartInd, tipEndInd, newTipString);
                            if (*size + delta > maxSize) {
                                LOG_ERROR("PatchString exceeded max size\n");
                                return false;
                            }
                            *size += delta;
                            LOG_INFO("%.*s patched tip\n", 4, id);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool PatchAbilitiesBinary(size_t maxSize, size_t* size, char* buffer,
                          size_t numAbilities, const struct Abilities* abilities)
{
    struct w3objIterator it;
    if (!w3objIteratorInit(&it, *size, (uint8_t*)buffer)) {
        LOG_ERROR("Failed to initialize w3obj iterator\n");
        return false;
    }

    while (true) {
        const struct w3objObject* object;
        if (!w3objIteratorNextObject(&it, &object)) {
            LOG_ERROR("Failed to get next object with w3obj iterator\n");
            return false;
        }
        if (object == NULL) {
            break;
        }

        const struct AbilityData* patchData = NULL;
        for (size_t i = 0; i < numAbilities; i++) {
            if (strncmp(object->originalId, abilities->abilityData[i].id, 4) == 0) {
                patchData = &abilities->abilityData[i];
            }
        }

        while (true) {
            const struct w3objMod* mod;
            struct w3objModValue value;
            if (!w3objIteratorNextMod(&it, &mod, &value)) {
                LOG_ERROR("Failed to get next mod with w3obj iterator\n");
                return false;
            }
            if (mod == NULL) {
                break;
            }

            const uint32_t levelIndex = mod->level - 1;
            for (int i = 0; i < C_ARRAY_LENGTH(ABILITY_LEVEL_FIELDS); i++) {
                const struct AbilityDataLevelField field = ABILITY_LEVEL_FIELDS[i];
                if (strncmp(mod->id, field.binaryName, 4) == 0) {
                    if (value.dataType != field.dataType) {
                        LOG_ERROR("Ability %.*s, mod %.*s, field data type mismatch %d vs %d\n",
                            4, object->originalId, 4, mod->id, value.dataType, field.dataType);
                        return false;
                    }

                    const uint8_t* dst = (uint8_t*)patchData + field.dataOffset;
                    switch (field.dataType) {
                    case FDT_INT: {
                        // TODO unimplemented
                    } break;
                    case FDT_FLOAT: {
                        // TODO unimplemented
                    } break;
                    case FDT_STRING: {
                        const struct String* dstString = (const struct String*)dst;
                        struct String newString = dstString[levelIndex];

                        int numLfs = 0;
                        for (size_t j = 1; j < newString.size; j++) {
                            if (newString.str[j] == '\n' && newString.str[j - 1] != '\r') {
                                numLfs++;
                            }
                        }
                        if (numLfs > 0) {
                            const size_t newNewStringSize = newString.size + numLfs;
                            struct String newNewString = StringAlloc(newNewStringSize);
                            DEBUG_ASSERT(newNewString.str != NULL);

                            int ind = 0;
                            for (size_t j = 0; j < newString.size; j++) {
                                DEBUG_ASSERT(ind < newNewStringSize);
                                if (newString.str[j] == '\n' && (j == 0 || newString.str[j - 1] != '\r')) {
                                    newNewString.str[ind++] = '\r';
                                    newNewString.str[ind++] = '\n';
                                }
                                else {
                                    newNewString.str[ind++] = newString.str[j];
                                }
                            }
                            DEBUG_ASSERT(ind == newNewStringSize);

                            newString = newNewString;
                            // TODO memory leak
                        }

                        if (!StringEqual(value.stringValue, newString)) {
                            struct String bufferString = { .size = *size, .str = buffer };
                            const size_t iStart = (size_t)(value.stringValue.str - buffer);
                            const int delta = PatchString(&bufferString, iStart, iStart + value.stringValue.size, newString);
                            if (*size + delta > maxSize) {
                                LOG_ERROR("PatchString exceeded max size\n");
                                return false;
                            }
                            *size += delta;
                            it.size += delta;
                            it.d += delta;
                            LOG_INFO("patched %s(%lu) for %.*s\n", field.name, mod->level, 4, object->originalId);
                        }
                    } break;
                    }
                }
            }
        }
    }

    return true;
}

bool PatchMapData(const struct String mpqFilePath, const struct String abilityDataPath,
                  const struct String jCodePath)
{
    const struct String abilityDataString = ReadEntireFile(abilityDataPath);
    if (abilityDataString.str == NULL) {
        LOG_ERROR("Failed to load ability data file %.*s\n",
            (int)abilityDataPath.size, abilityDataPath.str);
        return false;
    }

    size_t numAbilities;
    struct Abilities* abilities = malloc(sizeof(struct Abilities));
    if (!LoadAbilities(abilityDataString, &numAbilities, abilities)) {
        LOG_ERROR("Failed to load abilities from file\n");
        return false;
    }

    char mpqFilePathC[MAX_PATH];
    if (!ToCString(mpqFilePath, MAX_PATH, mpqFilePathC)) {
        LOG_ERROR("Failed to convert MPQ path to C string: %.*s\n",
            (int)mpqFilePath.size, mpqFilePath.str);
        return false;
    }

    HANDLE hMpq;
    bool result = SFileOpenArchive(mpqFilePathC, 0, 0, &hMpq);
    if (!result) {
        LOG_ERROR("Failed to open MPQ archive %.*s, error %lu\n",
            (int)mpqFilePath.size, mpqFilePath.str, GetLastError());
        return false;
    }

    SFILE_FIND_DATA findData;
    HANDLE hFindResult = SFileFindFirstFile(hMpq, "*", &findData, NULL);
    if (hFindResult == NULL) {
        LOG_ERROR("No files found in MPQ archive %.*s\n", (int)mpqFilePath.size, mpqFilePath.str);
        return false;
    }

    const size_t bufferSize = MEGABYTES(128);
    void* buffer = malloc(bufferSize);

    while (true) {
        const char* fileName = findData.cFileName;
        const DWORD fileSize = findData.dwFileSize;
        if (fileSize > bufferSize) {
            LOG_ERROR("fileSize (%lu) > bufferSize (%llu)\n", fileSize, bufferSize);
            return false;
        }

        enum MapMpqFile file = MPQFILE_COUNT;
        for (int i = 0; i < C_ARRAY_LENGTH(MAP_MPQ_FILE_INFO); i++) {
            if (strcmp(MAP_MPQ_FILE_INFO[i].mpqPath, fileName) == 0) {
                file = (enum MapMpqFile)i;
                break;
            }
        }

        if (file != MPQFILE_COUNT) {
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

            if (!SFileCloseFile(hFile)) {
                LOG_ERROR("Failed to close MPQ archive file %s, error %lu\n",
                          fileName, GetLastError());
            }

            if (bytesRead != fileSize) {
                LOG_ERROR("Failed to read entire MPQ archive file %s\n, bytesRead %lu",
                          fileName, bytesRead);
                return false;
            }

            bool overwrite = false;
            bool encrypt = false;
            size_t newSize = fileSize;
            switch (file) {
                case MPQFILE_ABILITIES_TXT: {
                    if (!PatchAbilitiesText(bufferSize, &newSize, buffer, numAbilities, abilities)) {
                        LOG_ERROR("Failed to patch ability text data\n");
                        return false;
                    }
                    WriteEntireFile(ToString("./patched/abilities.pat.txt"), newSize, buffer, false);
                    overwrite = true;
                    encrypt = true;
                } break;
                case MPQFILE_ABILITIES_MAP_TXT: {
                } break;
                case MPQFILE_ABILITIES_BIN: {
                    if (!PatchAbilitiesBinary(bufferSize, &newSize, buffer, numAbilities, abilities)) {
                        LOG_ERROR("Failed to patch ability binary data\n");
                        return false;
                    }
                    WriteEntireFile(ToString("./patched/abilities.pat.bin"), newSize, buffer, false);
                    overwrite = true;
                    encrypt = true;
                } break;
                case MPQFILE_JASS: {
                    const struct String newJ = ReadEntireFile(jCodePath);
                    if (newJ.str == NULL) {
                        LOG_ERROR("Failed to read JASS code for patch\n");
                        return false;
                    }
                    if (newJ.size > bufferSize) {
                        LOG_ERROR("JASS code file too big for patch\n");
                        return false;
                    }

                    memcpy(buffer, newJ.str, newJ.size);
                    newSize = newJ.size;
                    WriteEntireFile(ToString("./patched/war3map.j"), newSize, buffer, false);
                    overwrite = true;
                    encrypt = true;
                } break;
                default: {
                } break;
            }

            if (overwrite) {
                LOG_INFO("Overwriting \"%s\"\n", fileName);

                if (!SFileRemoveFile(hMpq, fileName, 0)) {
                    LOG_ERROR("Failed to remove MPQ archive file for overwrite: %s, error %lu\n",
                              fileName, GetLastError());
                    return false;
                }

                DWORD dwFlags = MPQ_FILE_COMPRESS;
                if (encrypt) {
                    dwFlags |= MPQ_FILE_ENCRYPTED;
                }
                HANDLE hNewFile;
                if (!SFileCreateFile(hMpq, fileName, 0, (DWORD)newSize, 0, dwFlags, &hNewFile)) {
                    LOG_ERROR("Failed to re-create MPQ archive file %s, error %lu\n",
                              fileName, GetLastError());
                    return false;
                }

                if (!SFileWriteFile(hNewFile, buffer, (DWORD)newSize, MPQ_COMPRESSION_BZIP2)) {
                    LOG_ERROR("Failed to write MPQ archive file %s, error %lu\n",
                              fileName, GetLastError());
                    return false;
                }

                if (!SFileFinishFile(hNewFile)) {
                    LOG_ERROR("Failed to call finish MPQ archive file %s, error %lu\n",
                              fileName, GetLastError());
                    return false;
                }
            }
        }

        if (!SFileFindNextFile(hFindResult, &findData)) {
            break;
        }
    }

    free(buffer);

    if (!SFileFindClose(hFindResult)) {
        LOG_ERROR("Failed to close MPQ find result handle for %.*s\n",
            (int)mpqFilePath.size, mpqFilePath.str);
        return false;
    }

#if 0
    if (!SFileCompactArchive(hMpq, NULL, 0)) {
        LOG_ERROR("Failed to compact MPQ archive %s, error %lu\n", mpqFilePath, GetLastError());
        return false;
    }
#endif

    if (!SFileCloseArchive(hMpq)) {
        LOG_ERROR("Failed to close MPQ archive %.*s, error %lu\n",
            (int)mpqFilePath.size, mpqFilePath.str, GetLastError());
        return false;
    }

    return true;
}
#endif

int main(int argc, char* argv[])
{
    if (argc != 3) {
        LOG_ERROR("mpqlite takes 2 arguments: mpqlite <MPQ path> <file-to-extract>\n");
        return 1;
    }

    if (!MpqliteExtractFile(argv[1], argv[2])) {
        LOG_ERROR("Failed to extract file %s from MPQ %s\n", argv[2], argv[1]);
        return 1;
    }

    return 0;
}

#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>
#undef STB_SPRINTF_IMPLEMENTATION
