//
// Created by fktrc on 21.09.2019.
//

#ifndef FUSE_DRIVER_CMAKE_HIDEFS_OPERATIONS_H
#define FUSE_DRIVER_CMAKE_HIDEFS_OPERATIONS_H

#include "hidefs.h"

static int hidefs_readdir(const char* orig_path, void* buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info* fi) {
    DIR* dp;
    struct dirent* de;
    int res;

    (void) offset;
    (void) fi;

    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);

    dp = opendir(path);
    if (dp == nullptr) {
        res = -errno;
        hidefs_log(aPath, "readdir", -1);
        delete[] aPath;
        free(path);
        return res;
    }

    while ((de = readdir(dp)) != nullptr) {

        if (strcmp(de->d_name, "file") == 0) {
            printf("readdir - You shall not pass!\n");
            continue;
        }

        struct stat st{};
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    hidefs_log(aPath, "readdir", 0);
    delete[] aPath;
    free(path);

    return 0;
}

static int hidefs_getattr(const char* orig_path, struct stat* stbuf) {
    int res;

    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);

    if (strcmp(aPath, "/home/fktrc/fuse/file") == 0) {
        printf("getattr - You shall not pass!\n");
        return -2;
    }

    res = lstat(path, stbuf);
    hidefs_log(aPath, "getattr", res);

    delete[] aPath;
    free(path);
    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_access(const char* orig_path, int mask) {
    int res;

    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);

    if (strcmp(aPath, "/home/fktrc/fuse/file") == 0) {
        printf("access - You shall not pass!\n");
        return -2;
    }

    res = access(path, mask);
    hidefs_log(aPath, "access", res);
    delete[] aPath;
    free(path);
    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_open(const char* orig_path, struct fuse_file_info* fi) {
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);

    if (strcmp(aPath, "/home/fktrc/fuse/file") == 0) {
        printf("open - You shall not pass!\n");
        return -2;
    }

    res = open(path, fi->flags);

    // what type of open ? read, write, or read-write ?
    if (fi->flags & O_RDONLY) {
        hidefs_log(aPath, "open-readonly", res);
    } else if (fi->flags & O_WRONLY) {
        hidefs_log(aPath, "open-writeonly", res);
    } else if (fi->flags & O_RDWR) {
        hidefs_log(aPath, "open-readwrite", res);
    } else
        hidefs_log(aPath, "open", res);

    delete[] aPath;
    free(path);

    if (res == -1)
        return -errno;

    fi->fh = res;
    return 0;
}

static int hidefs_mknod(const char* orig_path, mode_t mode, dev_t rdev) {
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);

	if (strcmp(aPath, "/home/fktrc/fuse/file") == 0) {
        printf("mknod - You shall not pass!\n");
        return -2;
    }

    if (S_ISREG(mode)) {
        res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
        hidefs_log(aPath, "mknod", res);
        if (res >= 0)
            res = close(res);
    } else if (S_ISFIFO(mode)) {
        res = mkfifo(path, mode);
        hidefs_log(aPath, "mkfifo", res);
    } else {
        res = mknod(path, mode, rdev);
        if (S_ISCHR(mode)) {
            hidefs_log(aPath, "mknod", res);
        } else
            hidefs_log(aPath, "mknod", res);
    }

    delete[] aPath;

    if (res == -1) {
        free(path);
        return -errno;
    } else
        lchown(path, fuse_get_context()->uid, fuse_get_context()->gid);

    free(path);

    return 0;
}

static void* hidefs_init(struct fuse_conn_info* info) {
    fchdir(savefd);
    close(savefd);
    return nullptr;
}

static int hidefs_readlink(const char* orig_path, char* buf, size_t size) {
    int res;

    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    res = readlink(path, buf, size - 1);
    hidefs_log(aPath, "readlink", res);
    delete[] aPath;
    free(path);
    if (res == -1)
        return -errno;

    buf[res] = '\0';

    return 0;
}

static int hidefs_mkdir(const char* orig_path, mode_t mode) {
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    res = mkdir(path, mode);
    hidefs_log(path, "mkdir", res);
    delete[] aPath;

    if (res == -1) {
        free(path);
        return -errno;
    } else
        lchown(path, fuse_get_context()->uid, fuse_get_context()->gid);

    free(path);
    return 0;
}

static int hidefs_unlink(const char* orig_path) {
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    res = unlink(path);
    hidefs_log(aPath, "unlink", res);
    delete[] aPath;
    free(path);

    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_rmdir(const char* orig_path) {
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    res = rmdir(path);
    hidefs_log(aPath, "rmdir", res);
    delete[] aPath;
    free(path);
    if (res == -1)
        return -errno;
    return 0;
}

static int hidefs_symlink(const char* from, const char* orig_to) {
    int res;

    char* aTo = getAbsolutePath(orig_to);
    char* to = getRelativePath(orig_to);

    res = symlink(from, to);

    hidefs_log(aTo, "symlink", res);
    delete[] aTo;

    if (res == -1) {
        free(to);
        return -errno;
    } else
        lchown(to, fuse_get_context()->uid, fuse_get_context()->gid);

    free(to);
    return 0;
}

static int hidefs_rename(const char* orig_from, const char* orig_to) {
    int res;
    char* aFrom = getAbsolutePath(orig_from);
    char* aTo = getAbsolutePath(orig_to);
    char* from = getRelativePath(orig_from);
    char* to = getRelativePath(orig_to);
    res = rename(from, to);
    hidefs_log(aFrom, "rename", res);
    delete[] aFrom;
    delete[] aTo;
    free(from);
    free(to);

    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_link(const char* orig_from, const char* orig_to) {
    int res;

    char* aFrom = getAbsolutePath(orig_from);
    char* aTo = getAbsolutePath(orig_to);
    char* from = getRelativePath(orig_from);
    char* to = getRelativePath(orig_to);

    res = link(from, to);
    hidefs_log(aTo, "link", res);
    delete[] aFrom;
    delete[] aTo;
    free(from);

    if (res == -1) {
        delete[] to;
        return -errno;
    } else
        lchown(to, fuse_get_context()->uid, fuse_get_context()->gid);

    free(to);

    return 0;
}

static int hidefs_chmod(const char* orig_path, mode_t mode) {
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    res = chmod(path, mode);
    hidefs_log(aPath, "chmod", res);
    delete[] aPath;
    free(path);

    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_chown(const char* orig_path, uid_t uid, gid_t gid) {
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    res = lchown(path, uid, gid);

    hidefs_log(aPath, "chown", res);
    delete[] aPath;
    free(path);

    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_truncate(const char* orig_path, off_t size) {
    int res;

    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    res = truncate(path, size);
    hidefs_log(aPath, "truncate", res);
    delete[] aPath;
    free(path);

    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_utimens(const char* orig_path, const struct timespec ts[2]) {
    int res;

    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);

    res = utimensat(AT_FDCWD, path, ts, AT_SYMLINK_NOFOLLOW);

    hidefs_log(aPath, "utimens", res);
    delete[] aPath;
    free(path);

    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_read(const char* orig_path, char* buf, size_t size, off_t offset,
                       struct fuse_file_info* fi) {
    char* aPath = getAbsolutePath(orig_path);
    int res;

    hidefs_log(aPath, "read", 0);
    res = pread(fi->fh, buf, size, offset);
    if (res == -1) {
        res = -errno;
        hidefs_log(aPath, "read", -1);
    } else {
        hidefs_log(aPath, "read", 0);
    }
    delete[] aPath;
    return res;
}

static int hidefs_write(const char* orig_path, const char* buf, size_t size,
                        off_t offset, struct fuse_file_info* fi) {
    int fd;
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    (void) fi;

    fd = open(path, O_WRONLY);
    if (fd == -1) {
        res = -errno;
        hidefs_log(aPath, "write", -1);
        delete[] aPath;
        free(path);
        return res;
    } else {
        hidefs_log(aPath, "write", 0);
    }

    res = pwrite(fd, buf, size, offset);

    if (res == -1)
        res = -errno;
    else
        hidefs_log(aPath, "write", 0);

    close(fd);
    delete[] aPath;
    free(path);

    return res;
}

static int hidefs_statfs(const char* orig_path, struct statvfs* stbuf) {
    int res;
    char* aPath = getAbsolutePath(orig_path);
    char* path = getRelativePath(orig_path);
    res = statvfs(path, stbuf);
    hidefs_log(aPath, "statfs", res);
    delete[] aPath;
    free(path);
    if (res == -1)
        return -errno;

    return 0;
}

static int hidefs_release(const char* orig_path, struct fuse_file_info* fi) {
    char* aPath = getAbsolutePath(orig_path);
    (void) orig_path;

    hidefs_log(aPath, "release", 0);
    delete[] aPath;

    close(fi->fh);
    return 0;
}

static int hidefs_fsync(const char* orig_path, int isdatasync,
                        struct fuse_file_info* fi) {
    char* aPath = getAbsolutePath(orig_path);
    (void) orig_path;
    (void) isdatasync;
    (void) fi;
    hidefs_log(aPath, "fsync", 0);
    delete[] aPath;
    return 0;
}

static int hidefs_setxattr(const char* orig_path, const char* name, const char* value,
                           size_t size, int flags) {
    int res = lsetxattr(orig_path, name, value, size, flags);
    if (res == -1)
        return -errno;
    return 0;
}

static int hidefs_getxattr(const char* orig_path, const char* name, char* value,
                           size_t size) {
    int res = lgetxattr(orig_path, name, value, size);
    if (res == -1)
        return -errno;
    return res;
}

static int hidefs_listxattr(const char* orig_path, char* list, size_t size) {
    int res = llistxattr(orig_path, list, size);
    if (res == -1)
        return -errno;
    return res;
}

static int hidefs_removexattr(const char* orig_path, const char* name) {
    int res = lremovexattr(orig_path, name);
    if (res == -1)
        return -errno;
    return 0;
}

#endif //FUSE_DRIVER_CMAKE_HIDEFS_OPERATIONS_H
