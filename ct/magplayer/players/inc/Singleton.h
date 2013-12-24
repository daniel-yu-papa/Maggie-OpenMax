#ifndef __MAG_SINGLETON_H__
#define __MAG_SINGLETON_H__

#include "Mag_base.h"

template <typename TYPE>
class MagSingleton{
public:
    static TYPE &getInstance(){
        MAG_STATIC_MUTEX_Acquire(mLock);
        TYPE *instance = msInstance;

        if (NULL == instance){
            instance = new TYPE();
            msInstance = instance;
        }
        MAG_STATIC_MUTEX_Release(mLock);
        return *instance;
    }
protected:
    MagSingleton(){
    }

    ~MagSingleton(){
        msInstance = NULL;
    }
private:
    MagSingleton(const MagSingleton &);
    MagSingleton& operator = (const MagSingleton &);
    static MagStaticMutex mLock;
    static TYPE *msInstance;
};

#define MAG_SINGLETON_STATIC_INSTANCE(TYPE)                 \
    MAG_STATIC_MUTEX_INITIALIZER(template<> MagStaticMutex MagSingleton< TYPE >::mLock); \
    template<> TYPE* MagSingleton< TYPE >::msInstance(NULL);           \
    template class MagSingleton< TYPE >;

#endif