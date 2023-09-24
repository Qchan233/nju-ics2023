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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>
#include <common.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_NUM, TK_HEX, TK_REG,
  TK_PLUS = '+', TK_MINUS = '-', TK_STAR = '*', TK_SLASH = '/',
  TK_NEQ, TK_AND,
  TK_LP = '(', TK_RP = ')'
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"0x[0-9a-f]+", TK_HEX},
  {"$[0-9a-z]+", TK_REG},
  {"[0-9]+", TK_NUM},    

  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // star
  {"\\/", '/'},         // slash
  {"\\(", '('},         // left parenthesis
  {"\\)", ')'},         // right parenthesis
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},         // not equal
  {"&&", TK_AND}         // logic and
};


int precedence(int token){
  switch (token)
  {
    case TK_EQ:
      return 0;
    case TK_PLUS:
    case TK_MINUS:
      return 1;
    case TK_STAR:
    case TK_SLASH:
      return 2;
    case -1:  
      return 2147483647;
    default:
      return -1;
  }
}

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

#define TOKEN_STR 32
typedef struct token {
  int type;
  char str[TOKEN_STR];
} Token;

static Token tokens[256] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static int token_count;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  token_count = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case '+':
          case '-':
          case '*':
          case '/':
          case '(':
          case ')':
            tokens[token_count].type = rules[i].token_type;
            token_count++;
            break;
          case TK_NUM:
            tokens[token_count].type = TK_NUM;
            memset(tokens[token_count].str, 0, TOKEN_STR);
            strncpy(tokens[token_count].str, substr_start, substr_len);
            token_count++;
            break;
          case TK_NOTYPE:
            break;
          default: break;
        }
        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}
 
bool expr_error;

bool check_parentheses(int p, int q){
  if (tokens[p].type != TK_LP){
    return false;
  }
  int count = 0;
  int i = 0;
  for(i=p;i<=q;i++){
    if (tokens[i].type == TK_LP){
      count++;
    }
    else if (tokens[i].type == TK_RP){
      count--;
    }

    if (count == 0 && i < q){
      return false;
    }
  }

  if (count != 0){
    expr_error = true;
  }
  return true;
}

word_t eval(int p, int q){
  if (p > q) {
    expr_error = true;
    return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    return atoi(tokens[p].str);
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    int i = 0;
    int op = -1;
    int op_pos = -1;

    int paren_depth = 0;

    for(i = p; i <= q; i++){
      int ttype = tokens[i].type;
      if (ttype == TK_LP){
        paren_depth += 1;
      }
      else if (ttype == TK_RP){
        paren_depth -= 1;
      }
      
      if (ttype == TK_PLUS || ttype == TK_MINUS || ttype == TK_STAR || ttype == TK_SLASH){
        if (precedence(ttype) <= precedence(op) && paren_depth == 0 ){
          op = ttype;
          op_pos = i;
        }
      }
    }

    if (op_pos < 0 || paren_depth != 0){
      expr_error = true;
      return 0;
    }

    word_t val1 = eval(p, op_pos - 1);
    word_t val2 = eval(op_pos + 1, q);

    switch (op) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': 
        if (val2 == 0){
          expr_error = true;
          return 0;
        }
        return val1 / val2;
      default:
        expr_error = true;
        return 0;
    }
  }
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  expr_error = false;
  int p = 0;
  int q = token_count - 1;

  int result = eval(p, q);
  if (expr_error){
    *success = false;
    return 0;
  }
  *success = true;
  
  return result;
}
