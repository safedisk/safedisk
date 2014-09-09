
#pragma once

#include "types.h"

// Does a write until all data is written or error
bool write_fully(int fd, const char* buf, int size);
// Does a read until all data is read or error
bool read_fully(int fd, char* buf, int size);

