#include "myaudioplay.h"

MyAudioPlay::MyAudioPlay()
{

}

MyAudioPlay::~MyAudioPlay()
{

}

double MyAudioPlay::_volume=30;

bool MyAudioPlay::write(const uchar *data, int datasize)
{
    Q_UNUSED(data);
    Q_UNUSED(datasize);
    return true;
}

int MyAudioPlay::getFree()
{
    return 1;
}

void MyAudioPlay::setAudioPara(int sampleRate, int sampleSize, int format, int channels)
{
    _sampleRate=sampleRate;
    _sampleSize=sampleSize;
    _format=format;
    _channels=channels;
}



