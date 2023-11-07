#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char* itoa(int num,char* str,int radix)
{
    if (num == -2147483648){
      strcpy(str, "-2147483648");
      return str;
    }
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//索引表
    int i=0,j,k;//i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置;j用来指示调整顺序时的交换。
 
    //获取要转换的整数的绝对值
    if(num<0)//要转换成十进制数并且是负数
    {
      num = -num;     // 将num的绝对值赋给unum
      str[i++] = '-'; // 在字符串最前面设置为'-'号，并且索引加1
    }
 
    //转换部分，注意转换后是逆序的
    do
    {
        str[i++]=index[num%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
        num/=radix;//unum去掉最后一位 
    }while(num);//直至unum为0退出循环
 
    str[i]='\0';//在字符串最后添加'\0'字符，c语言字符串以'\0'结束。
 
    //将顺序调整过来
    if(str[0]=='-') k=1;//如果是负数，符号不用调整，从符号后面开始调整
    else k=0;//不是负数，全部都要调整
 
    char temp;//临时变量，交换两个值时用到
    for(j=k;j<=(i-1)/2;j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];//头部赋值给临时变量
        str[j]=str[i-1+k-j];//尾部赋值给头部
        str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
    }
 
    return str;//返回转换后的字符串
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap);
#define BUFFER_SIZE 2048
int printf(const char *fmt, ...) {
  char buffer[BUFFER_SIZE];
  va_list args;
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

      size_t dlen = 0;
      switch (fmt[fmt_count++])
      {
      case 'd':
        char buffer[16];
        int value = va_arg(ap, int);
        itoa(value, buffer, 10);
        dlen = strlen(buffer);
        strcpy(out + out_count, buffer);
        break;
      case 's':
        char* str = va_arg(ap, char*);
        dlen = strlen(str);
        strcpy(out + out_count, str);
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
