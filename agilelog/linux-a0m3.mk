#!/bin/bash
if [ $1 = "-c" ]; then
	scons target=arm-a0m3-linux build_config=Debug -c
else
	scons target=arm-a0m3-linux build_config=Debug
fi
