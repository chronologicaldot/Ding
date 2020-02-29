// (c) 2019 Nicolaus Anderson

#ifndef APP_AUDIO_WRITER
#define APP_AUDIO_WRITER

#include "../ding/ding_types.h"
#include <vector>
#include <string>
#include <QFile>

//! Audio File Writer
/*
	This file writer writes two channels.
*/
struct AudioFileWriter
{
	typedef std::vector<ding::volume_t>  Buffer_t;

	AudioFileWriter();
	Buffer_t&  getBuffer();
	void  setSampleRate( quint32 );
	bool  write( QFile& );
	const char*  errorString();

private:
	bool  writeWavFile( QFile& );

	// Members ---------------------
	Buffer_t  buffer;
	quint32  sampleRate;
	std::string  errorMessage;
};

#endif // APP_AUDIO_WRITER
