#include "sdlaudioplay.h"
#include <QDebug>

SDLAudioPlay::SDLAudioPlay()
{

}

SDLAudioPlay::~SDLAudioPlay()
{
    SDLAudioPlay::close();
}


static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;


//回调函数
void  fill_audio(void *udata,Uint8 *stream,int len){

    SDL_memset(stream, 0, len);

    if(audio_len==0)
        return;

    len=(len>audio_len?audio_len:len);

    SDL_MixAudio(stream,audio_pos,len,MyAudioPlay::_volume);
    audio_pos += len;
    audio_len -= len;
//    qDebug() << SDL_GetError() << Qt::endl;

}

bool SDLAudioPlay::open()
{
    close();
    std::unique_lock<std::mutex> guard(_mutex);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

    MyAudioPlay::_volume=30;

    _sdlAudioSpec.freq = _sampleRate;
    //AV_SAMPLE_FMT_S16=1
    switch (_format)
    {
    case	AV_SAMPLE_FMT_S16:
        _sdlAudioSpec.format = AUDIO_S16SYS;
        break;
    case	AV_SAMPLE_FMT_S32:
        _sdlAudioSpec.format = AUDIO_S32SYS;
        break;
    case	AV_SAMPLE_FMT_FLT:
        _sdlAudioSpec.format = AUDIO_F32SYS;
        break;
    default:
        qDebug("audio device format was not surported!\n");
        break;
    }
    _sdlAudioSpec.channels = _channels;
    _sdlAudioSpec.silence = 0;
    _sdlAudioSpec.samples = _sampleSize;
    _sdlAudioSpec.callback = fill_audio;
    _sdlAudioSpec.userdata = 0;

    if(SDL_OpenAudio(&_sdlAudioSpec,nullptr)){
        return false;
    }
//    _deviceId = SDL_OpenAudioDevice(nullptr,0,&_sdlAudioSpec, nullptr,SDL_AUDIO_ALLOW_ANY_CHANGE);
//    if (_deviceId < 2){
//        qDebug() << "open audio device failed " << Qt::endl;
//        return false;
//    }

    SDL_PauseAudio(0);
//    SDL_PauseAudioDevice(_deviceId,0);

    return true;
}




bool SDLAudioPlay::write(const uchar *data, int datasize)
{
    if(datasize<=0){
        return false;
    }

    SDL_PauseAudio(0);

    audio_chunk = (Uint8 *) data;
    //Audio buffer length
    audio_len =datasize;

    audio_pos = audio_chunk;

    //等待数据播完
    while(audio_len>0)
        SDL_Delay(1);
    return true;
}

void SDLAudioPlay::close()
{
    std::unique_lock<std::mutex> guard(_mutex);

    SDL_CloseAudio();

    //释放SDL
    SDL_Quit();
}

void SDLAudioPlay::setVolume(qreal data)
{
    std::unique_lock<std::mutex> guard(_mutex);
    _volume=data;
}

void SDLAudioPlay::flushBuffer()
{
    std::unique_lock<std::mutex> guard(_mutex);
//    SDL_PauseAudio(1);
//    SDL_Delay(10);
}

