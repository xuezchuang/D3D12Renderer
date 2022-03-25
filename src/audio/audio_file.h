#pragma once

#include <xaudio2.h>


// Little endian.
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

struct audio_file
{
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    WAVEFORMATEXTENSIBLE wfx;

    DWORD dataChunkSize;
    DWORD dataChunkPosition;

    bool valid() { return fileHandle != INVALID_HANDLE_VALUE; }
};

audio_file openAudioFile(const fs::path& path);
void closeAudioFile(audio_file& file);

bool readChunkData(const audio_file& file, void* buffer, DWORD buffersize, DWORD bufferoffset);