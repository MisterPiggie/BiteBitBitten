#include "../types/types.h"
#include <stdbool.h>


bool write_all(int fd, const void *buf, size_t len);
bool create_resume_file(TR_torrent *tr, char *resume_filepath);
bool read_resume_file(TR_torrent *tr, char *resume_file_path);
