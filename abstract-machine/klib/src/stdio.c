#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}

char* itoa(int num, char* str, int base)
{
    char* index = "0123456789ABCDEF";
    int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
      if (num == INT32_MIN){
        strcpy(str, "-2147483648");
        return str;
      }
        isNegative = true;
        num = -num;
    }

    
    while (num != 0) {
        int rem = ((unsigned int) num) % base;
        str[i++] = index[rem];
        num = num / base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
    // Reverse the string
    reverse(str, i);
 
    return str;
}

char* uitoa(unsigned int num, char* str, int base)
{
    char* index = "0123456789ABCDEF";
    int i = 0;
 
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    
    while (num != 0) {
        int rem = ((unsigned int) num) % base;
        str[i++] = index[rem];
        num = num / base;
    }
  
    str[i] = '\0'; // Append string terminator
    // Reverse the string
    reverse(str, i);
    return str;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap);
#define BUFFER_SIZE 2048
int printf(const char *fmt, ...) {
  va_list args;
  char buffer[BUFFER_SIZE];
  va_start(args, fmt);
  size_t count = vsnprintf(buffer, BUFFER_SIZE, fmt, args);
  putstr(buffer);
  va_end(args);
  return count;
}
#undef BUFFER_SIZE

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  size_t count = vsnprintf(out, -1, fmt, args);
  va_end(args);
  return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

unsigned int read_int(const char *start, size_t* position){
  unsigned int num = 0;
  unsigned int index = 0;

  while ('0' <= start[index] && start[index] <= '9'){
    num = num * 10 + start[index] - '0';  
    (*position)++;
    index++;
  }
  return num;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  size_t fmt_count = 0;
  size_t out_count = 0;
  size_t limit = n - 1;
  char current;

  while(((current = fmt[fmt_count++]) != '\0') && out_count < limit){

    if (current == '%'){
      if (fmt[fmt_count] == '\0'){
        return out_count;    // Consider the format is not complete and not write %
      }
    
      // unsigned int format_length = read_int(&fmt[fmt_count], &fmt_count);
      unsigned int format_length = 0;

      size_t dlen = 0;
      char pad = '0';
      char buffer[16];
      switch (current = fmt[fmt_count++])
      {
      case 'd':
        int value = va_arg(ap, int);
        itoa(value, buffer, 10);
        dlen = strlen(buffer);
        while(format_length > dlen){
          format_length--;
          out[out_count++] = pad;
        }
        strcpy(out + out_count, buffer);
        format_length = 0;
        break;
      case 'p':
      case 'x':
        unsigned int value2 = va_arg(ap, int);
        uitoa(value2, buffer, 16);
        dlen = strlen(buffer);
        while(format_length > dlen){
          format_length--;
          out[out_count++] = pad;
        }
        strcpy(out + out_count, buffer);
        format_length = 0;
        break;
      case 's':
        char* str = va_arg(ap, char*);
        dlen = strlen(str);
        strcpy(out + out_count, str);
        break; 
      case 'c':
        char c = va_arg(ap, int) & 0xFF;
        out[out_count++] = c;
        break; 
      case '%':
        out[out_count++] = '%';
      break;
      default:
        return out_count;
        break;
      }
      out_count += dlen;
    }
    else{
      out[out_count++] = current;
    }
  }
  out[out_count] = '\0';
  return out_count;
}

#endif
