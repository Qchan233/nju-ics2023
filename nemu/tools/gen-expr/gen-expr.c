/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char *current;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

#define MAX_DEPTH 3

uint32_t choose(uint32_t n){
  return rand() % n;
}

static void gen(char* s){
  int length = strlen(s);
  sprintf(current, "%s", s);
  current += length;
}

static void gen_num(){
  uint32_t num = choose(1000);
  char temp[5];
  sprintf(temp, "%u", num);
  int length = strlen(temp);
  sprintf(current, "%s", temp);
  current += length;
}

static void gen_rand_op(){
  int op_type = choose(4);
  char op;
  switch (op_type)
  {
    case 0:
      op = '+';
      break;
    case 1:
      op = '-';
      break;
    case 2:
      op = '*';
      break;
    case 3:
      op = '/';
      break;
  }
  sprintf(current, "%c", op);
  current += 1;
}


static void gen_rand_expr(int depth) {
  if (depth > MAX_DEPTH){
    gen_num();
    return;
  }

  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen("("); gen_rand_expr(depth + 1); gen(")"); break;
    case 2: gen_rand_expr(depth + 1); gen_rand_op(); gen_rand_expr(depth + 1); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    current = buf;
    gen_rand_expr(0);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
