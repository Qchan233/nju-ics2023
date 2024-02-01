#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}


#define MAX_ARG_COUNT 16
char *argv_buf[MAX_ARG_COUNT];
static void sh_handle_cmd(const char *cmd) {
  char* cmd_copy = strdup(cmd);
  cmd_copy[strcspn(cmd_copy, "\n")] = 0;
  int argc = 0;
  char *token = strtok(cmd_copy, " ");
  while (token != NULL && argc < MAX_ARG_COUNT) {
      argv_buf[argc++] = strdup(token);
      // printf("%s\n", token);
      token = strtok(NULL, " ");
  }
  argv_buf[argc] = NULL;
  free(cmd_copy);

  execve(argv_buf[0], argv_buf, NULL);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
