#include <stdio.h>

#include "mpqlite.h"

int main(int argc, char* argv[])
{
	const struct String random = ReadEntireFile(ToString("random.txt"));
	if (random.str == NULL) {
		LOG_ERROR("Failed to read random.txt\n");
		return 1;
	}

	const struct String randomMpq = MpqliteRead("random.mpq", "random.txt");
	if (randomMpq.str == NULL) {
		LOG_ERROR("Failed to read random.mpq data\n");
		return 1;
	}

	if (!StringEqual(random, randomMpq)) {
		LOG_ERROR("random.mpq/random.txt data mismatch\n");
		return 1;
	}

    LOG_FLUSH();
    return 0;
}

#include "common.c"
#include "mpqlite.c"
