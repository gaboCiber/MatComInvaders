#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <pthread.h>
#include <unistd.h>
#include "../src/laser.h"


void* playBackgroundSound(void* arg) {
    char* file = (char*)arg;
    Mix_Chunk *sound = Mix_LoadWAV(file);
    if (!sound) {
        printf("Error loading sound: %s\n", Mix_GetError());
        return NULL;
    }
    Mix_PlayChannel(-1, sound, 0);
    SDL_Delay(41200); // Play for 5 seconds
    Mix_FreeChunk(sound);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Error initializing SDL_mixer: %s\n", Mix_GetError());
        return -1;
    }

    pthread_t thread1, thread2, thread3;
    pthread_create(&thread1, NULL, playBackgroundSound, "../src/backsound.wav");
    pthread_create(&thread2, NULL, playBackgroundSound, "../src/backsound.wav");
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}