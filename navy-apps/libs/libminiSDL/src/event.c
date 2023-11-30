#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[16];
  while(NDL_PollEvent(&buf, 0) == 0){
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
    for (i=0; i < sizeof(keyname) / sizeof(char*)){
      if(strcmp(buf+3, keyname[i])){
        event.key.keysym.sym = i;
        return 1;
      }
    }
  }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
