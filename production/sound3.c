#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <pthread.h>
#include "../src/backgroundSound.h"
#include "../src/bulletSound.h"


void* playSound(void* arg) {
    unsigned char* audio_data = (unsigned char*)arg;
    SDL_RWops* rw = SDL_RWFromMem(audio_data, backsound_cut_mp3_len);  // Usa el tamaño correcto
    Mix_Chunk *sound = Mix_LoadWAV_RW(rw, 1);
    if (!sound) {
        printf("Error loading sound: %s\n", Mix_GetError());
        return NULL;
    }
    Mix_PlayChannel(-1, sound, 0);
    SDL_Delay(60000); // Play for 5 seconds
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

    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, playSound, backsound_cut_mp3);  // Pasa el array de bytes
    pthread_join(thread1, NULL);

    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}
