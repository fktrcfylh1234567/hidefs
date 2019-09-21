#include <fuse.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include "hidefs.h"
#include "utils.h"
#include "hidefs_operations.h"

static bool processArgs(int argc, char* argv[], HideFS_Args* out) {
    out->isDaemon = true;
    out->fuseArgc = 0;

    // pass executable name through
    out->fuseArgv[0] = argv[0];
    ++out->fuseArgc;

    // leave a space for mount point, as FUSE expects the mount point before any flags
    out->fuseArgv[1] = nullptr;
    ++out->fuseArgc;
    opterr = 0;

    int res;
    bool got_p = false;

#define COMMON_OPTS "nonempty,use_ino,attr_timeout=0,entry_timeout=0,negative_timeout=0"

    while ((res = getopt(argc, argv, "hpfec:l:")) != -1) {
        // hidefs not running as a daemon
        if (res == 'f') {
            out->isDaemon = false;
            out->fuseArgv[out->fuseArgc++] = "-f";
        }

        // hidefs running as a public filesystem
        if (res == 'p') {
            out->fuseArgv[out->fuseArgc++] = "-o";
            out->fuseArgv[out->fuseArgc++] = "allow_other,default_permissions," COMMON_OPTS;
            got_p = true;
        }
    }

    if (!got_p) {
        out->fuseArgv[out->fuseArgc++] = "-o";
        out->fuseArgv[out->fuseArgc++] = COMMON_OPTS;
    }
#undef COMMON_OPTS

    if (optind + 1 <= argc) {
        out->mountPoint = argv[optind++];
        out->fuseArgv[1] = out->mountPoint;
    } else {
        fprintf(stderr, "Missing mountpoint\n");
        return false;
    }

    // If there are still extra unparsed arguments, pass them onto FUSE..
    if (optind < argc) {
        while (optind < argc) {
            out->fuseArgv[out->fuseArgc++] = argv[optind];
            ++optind;
        }
    }

    if (!isAbsolutePath(out->mountPoint)) {
        fprintf(stderr, "You must use absolute paths "
                        "(beginning with '/') for %s\n",
                out->mountPoint);
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    char* input = new char[2048]; // 2ko MAX input for configuration

    umask(0);
    fuse_operations hidefs_operations{};

    // in case this code is compiled against a newer FUSE library and new
    // members have been added to fuse_operations, make sure they get set to 0
    memset(&hidefs_operations, 0, sizeof(fuse_operations));
    hidefs_operations.init = hidefs_init;
    hidefs_operations.getattr = hidefs_getattr;
    hidefs_operations.access = hidefs_access;
    hidefs_operations.readlink = hidefs_readlink;
    hidefs_operations.readdir = hidefs_readdir;
    hidefs_operations.mknod = hidefs_mknod;
    hidefs_operations.mkdir = hidefs_mkdir;
    hidefs_operations.symlink = hidefs_symlink;
    hidefs_operations.unlink = hidefs_unlink;
    hidefs_operations.rmdir = hidefs_rmdir;
    hidefs_operations.rename = hidefs_rename;
    hidefs_operations.link = hidefs_link;
    hidefs_operations.chmod = hidefs_chmod;
    hidefs_operations.chown = hidefs_chown;
    hidefs_operations.truncate = hidefs_truncate;
    hidefs_operations.utimens = hidefs_utimens;
    hidefs_operations.flag_utime_omit_ok = 1;
    hidefs_operations.open = hidefs_open;
    hidefs_operations.read = hidefs_read;
    hidefs_operations.write = hidefs_write;
    hidefs_operations.statfs = hidefs_statfs;
    hidefs_operations.release = hidefs_release;
    hidefs_operations.fsync = hidefs_fsync;
    hidefs_operations.setxattr = hidefs_setxattr;
    hidefs_operations.getxattr = hidefs_getxattr;
    hidefs_operations.listxattr = hidefs_listxattr;
    hidefs_operations.removexattr = hidefs_removexattr;

    for (auto& i : hidefsArgs->fuseArgv)
        i = nullptr; // libfuse expects null args..

    if (processArgs(argc, argv, hidefsArgs)) {
        printf("hidefs starting at %s.\n", hidefsArgs->mountPoint);
        delete[] input;
        printf("chdir to %s\n", hidefsArgs->mountPoint);
        chdir(hidefsArgs->mountPoint);
        savefd = open(".", 0);

        fuse_main(hidefsArgs->fuseArgc,
                  const_cast<char**>(hidefsArgs->fuseArgv), &hidefs_operations, nullptr);

        printf("hidefs closing.\n");
    }
}
