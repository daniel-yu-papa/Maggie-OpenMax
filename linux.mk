#!/bin/bash
if [ $1 = "-c" ]; then
	scons target=x86-unknown-linux build_config=Debug -c
else
	scons target=x86-unknown-linux build_config=Debug
fi
