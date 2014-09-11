#!/bin/bash
if [ $1 = "-c" ]; then
	scons target=arm-unknown-linux build_config=Debug build_path_prefix=True -c
else
	scons target=arm-unknown-linux build_config=Debug build_path_prefix=True
fi

cp Targets/arm-unknown-linux/Debug/lib/libMagOmxIL_* ~/nfs/MagOmx_IL/
cp Targets/arm-unknown-linux/Debug/lib/libMagOmxILTest_* ~/nfs/MagOmx_IL/comps/
cp Targets/arm-unknown-linux/Debug/bin/MagOmxILTest_Playback ~/nfs/MagOmx_IL/
