#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

size_t *open_offsets;

void init_fs() {
  open_offsets = (size_t *)malloc(sizeof(size_t) * sizeof(file_table) / sizeof(file_table[0]));
  // TODO: initialize the size of /dev/fb
}

int fs_close(int fd){
  return 0;
}

int fs_open(const char *pathname, int flags, int mode){
  int i;
  for (i = 0; i < sizeof(file_table) / sizeof(file_table[0]); i++){
    if (strcmp(pathname, file_table[i].name) == 0){
      open_offsets[i] = file_table[i].disk_offset;
      return i;
    }
  }
  panic("No such file: %s", pathname);
}

size_t fs_read(int fd, void *buf, size_t len){
  ramdisk_read(buf, open_offsets[fd], len);
  open_offsets[fd] += len;
  return len;
}

size_t fs_write(int fd, void *buf, size_t len){
  ramdisk_write(buf, open_offsets[fd], len);
  open_offsets[fd] += len;
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  switch (whence){
    case SEEK_SET:
      open_offsets[fd] = file_table[fd].disk_offset + offset;
      break;
    case SEEK_CUR:
      open_offsets[fd] += offset;
      break;
    case SEEK_END:
      open_offsets[fd] = file_table[fd].disk_offset + file_table[fd].size + offset;
      break;
    default:
      panic("Invalid whence: %d", whence);
  }

  if (open_offsets[fd] > file_table[fd].disk_offset + file_table[fd].size){
    printf("file_offsets= %d\n", open_offsets[fd]);
    panic("Exceed file size: %d", file_table[fd].size);
  }
  return open_offsets[fd];
}
