//
// Created by fktrc on 21.09.2019.
//

#ifndef FUSE_DRIVER_CMAKE_HIDEFS_H
#define FUSE_DRIVER_CMAKE_HIDEFS_H

static int savefd;

const int MaxFuseArgs = 32;
struct HideFS_Args {
    char* mountPoint;
    bool isDaemon;
    const char* fuseArgv[MaxFuseArgs];
    int fuseArgc;
};

static HideFS_Args* hidefsArgs = new HideFS_Args;

#endif //FUSE_DRIVER_CMAKE_HIDEFS_H
