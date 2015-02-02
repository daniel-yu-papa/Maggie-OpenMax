#!/bin/bash
if [ $1 = "-c" ]; then
	scons target=arm-unknown-linux build_config=Debug build_path_prefix=True -c
else
	scons target=arm-unknown-linux build_config=Debug build_path_prefix=True
fi
