#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

void bulletSound()
{
    SDL_Init(SDL_INIT_AUDIO);
    
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;

    SDL_LoadWAV("../src/laser-zap-90575.wav", &wavSpec, &wavBuffer, &wavLength);

    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

    int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_PauseAudioDevice(deviceId, 0);

    SDL_Delay(5000);

    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
    SDL_Quit();

}

int main(int argc, char* argv[]) {
    bulletSound();
    return 0;
}