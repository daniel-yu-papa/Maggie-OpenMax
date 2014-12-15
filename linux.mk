#!/bin/bash
if [ $1 = "-c" ]; then
	scons target=x86-unknown-linux build_config=Debug -c
else
	scons target=x86-unknown-linux build_config=Debug
fi

#cp Targets/x86-unknown-linux/Debug/lib/libMagOmxIL_* ~/nfs/MagOmx_IL_x86
#cp Targets/x86-unknown-linux/Debug/lib/libMagOmxILTest_* ~/nfs/MagOmx_IL_x86/comps/
#cp Targets/x86-unknown-linux/Debug/bin/MagOmxILTest_Playback ~/nfs/MagOmx_IL_x86/
#cp Targets/x86-unknown-linux/Debug/lib/libMagFramework.so ~/nfs/MagOmx_IL_x86/
#cp Targets/x86-unknown-linux/Debug/lib/libMagAgilelog.so ~/nfs/MagOmx_IL_x86/
