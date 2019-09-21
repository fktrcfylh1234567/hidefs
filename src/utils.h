//
// Created by fktrc on 21.09.2019.
//

#ifndef FUSE_DRIVER_CMAKE_UTILS_H
#define FUSE_DRIVER_CMAKE_UTILS_H

#include <dirent.h>
#include <cerrno>
#include <sys/xattr.h>
#include <pwd.h>
#include <cstdlib>

#include "hidefs.h"

static bool isAbsolutePath(const char* fileName) {
    return fileName && fileName[0] != '\0' && fileName[0] == '/';
}

static char* getAbsolutePath(const char* path) {
    char* realPath = new char[strlen(path) + strlen(hidefsArgs->mountPoint) + 1];

    strcpy(realPath, hidefsArgs->mountPoint);
    if (realPath[strlen(realPath) - 1] == '/')
        realPath[strlen(realPath) - 1] = '\0';
    strcat(realPath, path);
    return realPath;
}

static char* getRelativePath(const char* path) {
    if (path[0] == '/') {
        if (strlen(path) == 1) {
            return strdup(".");
        }
        const char* substr = &path[1];
        return strdup(substr);
    }

    return strdup(path);
}

static char* getcallername() {
    char filename[100];
    sprintf(filename, "/proc/%d/cmdline", fuse_get_context()->pid);
    FILE* proc;
    char cmdline[256] = "";

    if ((proc = fopen(filename, "rt")) == nullptr)
        return nullptr;
    else {
        fread(cmdline, sizeof(cmdline), 1, proc);
        fclose(proc);
    }

    return strdup(cmdline);
}

static void hidefs_log(const char* path, const char* action, const int returncode) {
    const char* retname;

    if (returncode >= 0)
        retname = "SUCCESS";
    else
        retname = "FAILURE";

    char* additionalInfo = nullptr;
    char* caller_name = getcallername();
    int uid = fuse_get_context()->uid;

    printf("%s %s {%s} [ pid = %d %s uid = %d ]\n", action, path, retname, fuse_get_context()->pid, caller_name, uid);

    free(additionalInfo);
    free(caller_name);
}

#endif //FUSE_DRIVER_CMAKE_UTILS_H
