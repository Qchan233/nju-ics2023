#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

uint8_t keystates[83];
int SDL_PollEvent(SDL_Event *ev) {
  char buf[16];
  if(NDL_PollEvent(&buf, 16) == 0){
    return 0;
  }

  if (ev == NULL){
    return 1;
  }

  switch (buf[1])
    {
    case 'd':
      ev->type = SDL_KEYDOWN;
      break;
    case 'u':
      ev->type = SDL_KEYUP;
      break;
    default:
      break;
    }
    int i;
    for (i=0; i < sizeof(keyname) / sizeof(char*); i++){
      if(strcmp(buf+3, keyname[i]) == 0){
        ev->key.keysym.sym = i;
        keystates[i] = ev->type == SDL_KEYDOWN ? 1 : 0;
        return 1;
      }
    }
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[16];
  while(NDL_PollEvent(&buf, 16) == 0);
  switch (buf[1])
    {
    case 'd':
      event->type = SDL_KEYDOWN;
      break;
    case 'u':
      event->type = SDL_KEYUP;
      break;
    default:
      break;
    }
    int i;
    for (i=0; i < sizeof(keyname) / sizeof(char*); i++){
      if(strcmp(buf+3, keyname[i]) == 0){
        keystates[i] = event->type == SDL_KEYDOWN ? 1 : 0;
        event->key.keysym.sym = i;
        return 1;
      }
    }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys != NULL){
    *numkeys = 83;
  }
  return keystates;
}
