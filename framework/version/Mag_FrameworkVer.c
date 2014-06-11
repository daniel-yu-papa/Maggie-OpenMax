#include "Mag_FrameworkVer.h"
#include <string.h>

#define kFrameworkVersion "v0.0.1"

void Mag_getFrameWorkVer(char **ppVer){
	strcpy(*ppVer, kFrameworkVersion);
}