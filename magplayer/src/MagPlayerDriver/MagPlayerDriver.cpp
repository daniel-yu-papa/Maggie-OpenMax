/*
 * Copyright (c) 2015  Daniel Yu <daniel_yu2015@outlook.com>
 *
 * This file is part of Maggie-OpenMax project.
 *
 * Maggie-OpenMax is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Maggie-OpenMax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Maggie-OpenMax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "MagPlayerDriver.h"
#include "MagPlayerDriverFactory.h"

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "Magply_Driver"

MagPlayerDriver::MagPlayerDriver(void *client, notify_client_callback_f cb):
                                                    mPrivData(client),
                                                    mEventCB(cb),
                                                    mPlayerDriver(NULL){
}

MagPlayerDriver::~MagPlayerDriver(){
    AGILE_LOGD("Enter!");

    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    delete dirver;
    MagPlayerDriverFactory &factory = MagPlayerDriverFactory::getInstance();
    delete dynamic_cast<MagPlayerDriverFactory *>(&factory);

    AGILE_LOGD("Exit!");
}
    
_status_t        MagPlayerDriver::setDataSource(const char *url){
    AGILE_LOGD("enter!");
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->setDataSource(url);
}

_status_t        MagPlayerDriver::setDataSource(unsigned int fd, long long offset, long long length){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->setDataSource(fd, offset, length);
}

_status_t        MagPlayerDriver::prepare(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->prepare();
}

_status_t        MagPlayerDriver::prepareAsync(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->prepareAsync();
}

_status_t        MagPlayerDriver::start(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->start();
}

_status_t        MagPlayerDriver::stop(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->stop();
}

_status_t        MagPlayerDriver::pause(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->pause();
}

bool             MagPlayerDriver::isPlaying(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->isPlaying();
}

_status_t        MagPlayerDriver::seekTo(int msec){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->seekTo(msec);
}

_status_t        MagPlayerDriver::flush(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->flush();
}

_status_t        MagPlayerDriver::fast(int speed){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->fast(speed);
}

_status_t        MagPlayerDriver::getCurrentPosition(int* msec){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->getCurrentPosition(msec);
}

_status_t        MagPlayerDriver::getDuration(int* msec){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->getDuration(msec);
}

_status_t        MagPlayerDriver::reset(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->reset();
}

_status_t        MagPlayerDriver::setVolume(float leftVolume, float rightVolume){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->setVolume(leftVolume, rightVolume);
}

_status_t        MagPlayerDriver::setParameter(int key, void *request){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->setParameter(key, request);
}

_status_t        MagPlayerDriver::getParameter(int key, void **reply){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->getParameter(key, reply);
}

_status_t        MagPlayerDriver::invoke(const unsigned int methodID, void *request, void **reply){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->invoke(methodID, request, reply);
}

ui32             MagPlayerDriver::getVersion(){
    MagPlayerDriverImplBase *dirver = getPlayerDriver();
    return dirver->getVersion();
}

MagPlayerDriverImplBase *MagPlayerDriver::getPlayerDriver(){
    if (mPlayerDriver)
        return mPlayerDriver;
    else{
        MagPlayerDriverFactory &factory = MagPlayerDriverFactory::getInstance();
        mPlayerDriver = factory.create(mPrivData, mEventCB);
        return mPlayerDriver;
    }
}