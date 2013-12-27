#define LOG_TAG "MagPlayerLoader"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include <utils/Vector.h>
#include <utils/String8.h>

#include <media/MediaPlayerInterface.h>
#include <MediaPlayerFactory.h>

#include "magplayer_loader.h"

namespace android {

const int MediaPlayerLoader::kLoadablePlayerStart;
const int MediaPlayerLoader::kLoadablePlayerEnd;

typedef MediaPlayerFactory::IFactory* (*createFactoryFunc)(player_type playerType);

struct PlayerFactoryData {
    player_type playerType;
    // Function to create an instance of player factory.
    MediaPlayerFactory::IFactory* (*createFactory)(player_type playerType);
    // Handle of shared library.
    void* dso;
    int id;
    char tag[PATH_MAX];
};

static Vector<PlayerFactoryData*> gPlayerModules;

static bool listDir(const char* dirpath, Vector<String8>* files) {
    if (dirpath == NULL || files == NULL) {
        return false;
    }

    DIR* dir1 = opendir(dirpath);
    if (dir1 != NULL) {
        struct dirent entry;
        struct dirent* ptr = NULL;
        while (true) {
            if (readdir_r(dir1, &entry, &ptr) != 0) {
                break;
            }

            if (ptr == NULL) {
                break;
            }

            files->push_back(String8(entry.d_name));
        }
        closedir(dir1);
    }

    return true;
}

static bool getIndexAndTag(const char* path, int* n, char tag[PATH_MAX]) {
    if (path == NULL || n == NULL) return false;
    const char* str = path;
    const int prefixLen = strlen(PLAYER_FACTORY_PREFIX);
    if (strncmp(str, PLAYER_FACTORY_PREFIX, prefixLen) != 0) {
        return false;
    }
    str += prefixLen;
    char* endptr;
    errno = 0;
    *n = strtoul(str, &endptr, 10);
    if (errno != 0) return false;
    if ((*endptr) != '.') return false;
    ++endptr;
    str = strchr(endptr, '.');
    if (str == NULL) return false;
    int len = str - endptr;
    memcpy(tag, endptr, len);
    tag[len] = 0;
    return true;
}

static bool loadModule(
        const char* dirpath,
        const char* filepath,
        PlayerFactoryData* entry) {
    if (dirpath == NULL || filepath == NULL || entry == NULL) {
        return false;
    }

    // Skip ".", ".." or hidden files.
    if (filepath[0] == '.') {
        return false;
    }

    if (!getIndexAndTag(filepath, &entry->id, entry->tag)) {
        ALOGW("Module path %s is invalid", filepath);
        return false;
    }

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", dirpath, filepath);

    void* handle;
    handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        ALOGE("dlopen failed: %s %s", path, dlerror());
        return false;
    }

    ALOGI("Loading %s", path);

    entry->createFactory = (createFactoryFunc) dlsym(handle, "createFactory");

    if (entry->createFactory == NULL) {
        ALOGW("Module %s does not contain createFactory symbol", path);
        dlclose(handle);
        return false;
    }

    entry->dso = handle;
    return true;
}

static int string8_compare(const String8* lhs, const String8* rhs) {
    return lhs->compare(*rhs);
}

static void loadModulesInDir(const char* dirpath) {
    Vector<String8> files;
    listDir(dirpath, &files);
    files.sort(string8_compare);

    for (Vector<String8>::iterator i = files.begin();
            i != files.end();
            ++i) {
        PlayerFactoryData* entry = new PlayerFactoryData();
        if (loadModule(dirpath, i->string(), entry)) {
            gPlayerModules.push_back(entry);
        } else {
            delete entry;
        }
    }
}

status_t MediaPlayerLoader::initialize() {
    // Enumerate player factory module in /vendor/lib/player.
    loadModulesInDir(PLAYER_FACTORY_PATH1);

    // Enumerate player factory module in /system/lib/player.
    loadModulesInDir(PLAYER_FACTORY_PATH2);

    int type = kLoadablePlayerStart;
    for (Vector<PlayerFactoryData*>::iterator i = gPlayerModules.begin();
            i != gPlayerModules.end();
            ++i) {

        // Try a range of player_type values in case of conflict.
        while (type < kLoadablePlayerEnd) {
            player_type playerType = static_cast<player_type>(type++);

            MediaPlayerFactory::IFactory* factory = (*i)->createFactory(playerType);
            if (factory == NULL) {
                ALOGE("Failed to create player factory");
                break;
            }

            if (MediaPlayerFactory::registerFactory(factory, playerType) == OK) {
                (*i)->playerType = playerType;
                break;
            } else {
                // Player type already registered. Try a new type.
                delete factory;
            }
        }
    }

    return OK;
}

}; // namespace android
