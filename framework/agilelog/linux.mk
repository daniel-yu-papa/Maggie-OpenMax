#!/bin/bash
if [ $1 = "-c" ]; then
	scons target=arm-unknown-linux build_config=Debug -c
else
	scons target=arm-unknown-linux build_config=Debug
fi
