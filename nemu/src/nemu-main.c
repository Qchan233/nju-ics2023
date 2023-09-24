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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

//   /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}


// #include <stdio.h>
// #include <stdlib.h>
// #include "monitor/sdb/sdb.h"

// void init_regex();
// int main() {
//     FILE *file = fopen("exprs", "r");
//     if (file == NULL) {
//         printf("无法打开文件，喵~\n");
//         return 1;
//     }
//     bool success;
//     init_regex();


//     char expression[65536];
//     int expected_result;
//     while (fscanf(file, "%d %[^\n]", &expected_result, expression) != EOF) {
//         int result = expr(expression, &success);
//         if (result == expected_result) {
//             printf("测试通过，喵~ 表达式: %s, 结果: %d\n", expression, result);
//         } else {
//             printf("测试失败，喵~ 表达式: %s, 期望: %d, 实际: %d\n", expression, expected_result, result);
//         }
//     }

//     fclose(file);
//     return 0;
// }