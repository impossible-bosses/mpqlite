#!/usr/bin/bash

OUTPUT_EXE=mpqlite

help()
{
	echo build.sh takes 1 argument: build/clean/run/test/setup/release
}

build()
{
	mkdir -p build
	cd build

	INCLUDE_DIRS="-I ../libs/StormLib/include -I ../libs/stb_sprintf"
	LIB_DIRS="-L ../libs/StormLib/lib/debug -L /usr/lib/x86_64-linux-gnu"
	# TODO ew, stdc++
	LIBS="-lstorm-arm -lz -lbz2 -lstdc++"
	ZIG_BUILD_LINE="g++ ../src/main.c -DAPP_LINUX -o $OUTPUT_EXE $INCLUDE_DIRS $LIB_DIRS $LIBS"

	echo $ZIG_BUILD_LINE
	$ZIG_BUILD_LINE
	if [ $? -ne 0 ]; then
		exit 1
	fi
}

clean()
{
	echo clean unimplemented
	exit 1
}

run()
{
	echo run unimplemented
	exit 1
}

test()
{
	echo test unimplemented
	exit 1
}

setup()
{
	echo No setup required
}

release()
{
	echo release unimplemented
	exit 1
}

if [ "$1" == "help" ]; then
	help
elif [ "$1" == "build" ]; then
	build
elif [ "$1" == "clean" ]; then
	clean
elif [ "$1" == "run" ]; then
	run
elif [ "$1" == "test" ]; then
	test
elif [ "$1" == "setup" ]; then
	setup
elif [ "$1" == "release" ]; then
	release
else
	help
fi

