#ifndef SDLAUDIOPLAY_H
#define SDLAUDIOPLAY_H

#include "myaudioplay.h"

extern "C"
{
#include "SDL/SDL.h"
#include <libswresample/swresample.h>
#undef main
}

class SDLAudioPlay : public MyAudioPlay
{
public:
    SDLAudioPlay();
    ~SDLAudioPlay();


    // MyAudioPlay interface
public:
    virtual bool open() override;
    virtual void close() override;
    virtual void setVolume(qreal) override;
    virtual void flushBuffer() override;
    virtual bool write(const uchar *data, int datasize) override;

public:
    SDL_AudioSpec _sdlAudioSpec;

private:
    SDL_AudioDeviceID _deviceId;
};

#endif // SDLAUDIOPLAY_H
