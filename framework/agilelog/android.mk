#!/bin/bash
if [ $1 = "-c" ]; then
	scons target=arm-android-linux build_config=Debug -c
else
	scons target=arm-android-linux build_config=Debug
fi
