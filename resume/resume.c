#include "resume.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


bool create_resume_file(TR_torrent *tr, char *resume_filepath)
{
    char tmp_buf[1024];
    char pig_buf[1024];
    int fd = -1;
    int dirfd = -1;
    uint32_t len = strlen(tr->download_path);

    snprintf(tmp_buf, 1024, "%s/%s.tmp", resume_filepath, tr->info->name);
    fd = open(tmp_buf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        return false;

    if (!write_all(fd, &len, sizeof(len)))
        goto fail;
    if (!write_all(fd, tr->download_path, len))
        goto fail;

    if (!write_all(fd, &tr->bitfield_length, sizeof(tr->bitfield_length)))
        goto fail;
    if (!write_all(fd, tr->bitfield, tr->bitfield_length))
        goto fail;

    if (fsync(fd) == -1)
        goto fail;

    close(fd);
    fd = -1;

    snprintf(pig_buf, 1024, "%s/%s.pig", resume_filepath, tr->info->name);
    if (rename(tmp_buf, pig_buf) == -1)
        goto fail;

    dirfd = open(resume_filepath, O_RDONLY);
    if (dirfd == -1)
        goto fail;

    if (fsync(dirfd) == -1)
        goto fail;

    close(dirfd);

    return true;

fail:
    if (fd != -1)
        close(fd);

    if (dirfd != -1)
        close(dirfd);

    return false;
}

bool write_all(int fd, const void *buf, size_t len)
{
    const char *p = buf;
    ssize_t n;

    while (len > 0)
    {
        n = write(fd, p, len);
        if (n <= 0)
            return false;

        p += n;
        len -= n;
    }
    return true;
}

bool read_resume_file(TR_torrent *tr, char *resume_file_path)
{
    int fd;
    uint32_t path_len;

    fd = open(resume_file_path, O_RDONLY);
    if (fd == -1)
        return false;

    if (!read_all(fd, &path_len, 4))
        goto fail;
     
    if (!read_all(fd, tr->download_path, path_len))
        goto fail;

    if (!read_all(fd, &tr->bitfield_length, 4))
        goto fail;
    if (!read_all(fd, tr->bitfield, tr->bitfield_length)) 
        goto fail;

    close(fd);

    return true;

fail:
    close(fd);
    return false;
}

bool read_all(int fd, void *buf, size_t len)
{
    char *p = buf;
    ssize_t n;

    while (len > 0)
    {
        n = read(fd, p, len);
        if (n <= 0)
            return false;

        p += n;
        len -= n;
    }
    return true;
}
