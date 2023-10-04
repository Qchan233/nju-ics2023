#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len] != '\0'){
    len++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
    size_t i;

    for (i = 0; src[i] != '\0'; i++)
        dst[i] = src[i];

    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dst[i] = src[i];
    for ( ; i < n; i++)
        dst[i] = '\0';

    return dst;
}

char *strcat(char *dst, const char *src) {
    size_t dest_len = strlen(dst);
    size_t i;

    for (i = 0 ; src[i] != '\0' ; i++)
        dst[dest_len + i] = src[i];
    dst[dest_len + i] = '\0';

    return dst;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1){
    if (*s1 != *s2){
      break;
    }
    s1++;
    s2++;
  }
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while ( n && *s1 && ( *s1 == *s2 ) )
  {
      ++s1;
      ++s2;
      --n;
  }
  if ( n == 0 )
  {
      return 0;
  }
  else
  {
      return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
  }
}

void *memset(void *s, int c, size_t n) {
  unsigned char* pos = (unsigned char*) s;
  size_t i;
  for (i=0; i<n; i++){
    pos[i] = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;

    if (d==s) return dst;
    if (d < s || d >= s + n) {
        // 如果没有重叠，从左到右复制喵
        while (n--) {
            *d++ = *s++;
        }
    } else {
        // 如果有重叠，从右到左复制喵
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }

    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  size_t i;
  char* cout = (char*) out;
  char* cin = (char*) in;

  for (i=0; i<n; i++){
    cout[i] = cin[i];
  }
  return out;

}

int memcmp(const void *s1, const void *s2, size_t n) {
    if(!n)
      return(0);

    while(--n && *(char*)s1 == *(char*)s2 ) {
        s1 = (char*)s1 + 1;
        s2 = (char*)s2 + 1;
    }

    return(*((unsigned char*)s1) - *((unsigned char*)s2));
}

#endif
