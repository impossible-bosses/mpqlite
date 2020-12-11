#include <stdio.h>

#include "mpqlite.h"

int main(int argc, char* argv[])
{
	const struct String random = ReadEntireFile(ToString("random.txt"));
	if (random.str == NULL) {
		LOG_ERROR("Failed to read random.txt\n");
		return 1;
	}

	// Read
	const struct String randomMpq = MpqliteRead("random.mpq", "random.txt");
	if (randomMpq.str == NULL) {
		LOG_ERROR("MpqliteRead failed on random.mpq\n");
		return 1;
	}

	if (!StringEqual(random, randomMpq)) {
		LOG_ERROR("random.mpq/random.txt data mismatch\n");
		return 1;
	}

	// Write and read back
	if (!MpqliteWrite("empty.mpq", "random.txt", random, false)) {
		LOG_ERROR("MpqliteWrite failed on empty.mpq\n");
		return 1;
	}

	const struct String emptyMpq = MpqliteRead("random.mpq", "random.txt");
	if (emptyMpq.str == NULL) {
		LOG_ERROR("MpqliteRead failed on empty.mpq\n");
		return 1;
	}

	if (!StringEqual(emptyMpq, random)) {
		LOG_ERROR("empty.mpq/random.txt data mismatch\n");
		return 1;
	}

	LOG_INFO("All tests passed\n");
    LOG_FLUSH();
    return 0;
}

#include "common.c"
#include "mpqlite.c"
