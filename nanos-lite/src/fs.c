#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_FB] =     {"/dev/fb", 0, 0, invalid_read, fb_write},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, serial_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

WriteFn get_write_fn(int fd){
  return file_table[fd].write;
}

ReadFn get_read_fn(int fd){
  return file_table[fd].read;
}

size_t *open_offsets;
int screen_width;
int screen_height;
void init_fs() {
  open_offsets = (size_t *)malloc(sizeof(size_t) * sizeof(file_table) / sizeof(file_table[0]));
  int buf[2];
  dispinfo_read(buf, 0, 2 * sizeof(int));
  screen_width = buf[0];
  screen_height = buf[1];
  file_table[FD_FB].size = screen_width * screen_height * sizeof(uint32_t);
  // printf("screen_width: %d, screen_height: %d\n", screen_width, screen_height);
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
  size_t remaining = file_table[fd].size - (open_offsets[fd] - file_table[fd].disk_offset);
  len = len > remaining ? remaining : len;
  ramdisk_read(buf, open_offsets[fd], len);
  open_offsets[fd] += len;
  return len;
}

size_t fs_write(int fd, void *buf, size_t len){
  assert(open_offsets[fd] + len <= file_table[fd].disk_offset + file_table[fd].size);
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
    panic("Exceed file size: %d", file_table[fd].size);
  }
  return open_offsets[fd] - file_table[fd].disk_offset;
}
