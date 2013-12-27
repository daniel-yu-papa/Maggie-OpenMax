#ifndef __MAGPLAYERLOADER_ANDROID_H__
#define __MAGPLAYERLOADER_ANDROID_H__

/** Base path of the player factory modules */
#define PLAYER_FACTORY_PATH1    "/vendor/lib/player"
#define PLAYER_FACTORY_PATH2    "/system/lib/player"
#define PLAYER_FACTORY_PREFIX   "player."

namespace android {

class MagPlayerLoader {
public:
    static const int kLoadablePlayerStart   = 1000;
    static const int kLoadablePlayerEnd     = 2000;

    static status_t initialize();
};

}; // namespace android

#endif // ANDROID_MEDIAPLAYERLOADER_H
