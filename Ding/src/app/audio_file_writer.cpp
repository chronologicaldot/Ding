// (c) 2019 Nicolaus Anderson

#include "audio_file_writer.h"
#include <QDataStream>

AudioFileWriter::AudioFileWriter()
	: buffer()
	, sampleRate(44100)
	, errorMessage()
{}

AudioFileWriter::Buffer_t&
AudioFileWriter::getBuffer() {
	return buffer;
}

void
AudioFileWriter::setSampleRate( quint32  rate ) {
	sampleRate = rate;
}

bool
AudioFileWriter::write( QFile&  file ) {
	// Currently, only one file format is supported.
	return writeWavFile(file);
}

bool
AudioFileWriter::writeWavFile( QFile&  file ) {
	// We take inspiration from AudioFile by Adam Stark
	// https://github.com/adamstark/AudioFile/blob/master/AudioFile.cpp

	if ( buffer.size() == 0 ) {
		errorMessage = "Buffer was not set";
		return false;
	}

	QDataStream  out{&file};
	out.setByteOrder(QDataStream::LittleEndian);
	const qint32  bytesPerSample = 3; // Bit Depth = 24, with 8 bits per byte
	const qint32  numChannels = 2;
	const qint32  dataSize = buffer.size() * numChannels * bytesPerSample;

	// The file size in bytes is the header chunk size (4, not counting RIFF and WAVE)
	// + the format chunk size (24) + the metadata part of the data chunk plus the actual data chunk size
	//out << "RIFF";
	out.writeRawData( "RIFF", 4 );
	qint32  fileSizeInBytes = 4 + 24 + 8 + dataSize;
	out << fileSizeInBytes;
	//out << "WAVE";
	out.writeRawData( "WAVE", 4 );

	// Format Chunk --------------------
	//out << "fmt ";
	out.writeRawData( "fmt ", 4 );
	out << qint32( 16 );	// Format chunk size. PCM (Pulse-code mudulation aka raw) == 16 (2-byte integer)
	out << qint16( 1 );		// Audio Format == 1 for PCM (2-byte integer)
	out << qint16( numChannels );
	out << qint32( sampleRate );
	out << qint32( numChannels * sampleRate * bytesPerSample );	// Bytes per Second
	out << qint16( numChannels * bytesPerSample );				// Bytes per block
	out << qint16( bytesPerSample * 8 );	// Bit Depth

	// Data Chunk ----------------------
	//out << "data";
	out.writeRawData( "data", 4 );
	out << dataSize;

	size_t  sampleIdx = 0;
	qint32  channelIdx = 0;
	for (; sampleIdx < buffer.size(); ++sampleIdx) {
		for ( channelIdx = 0; channelIdx < numChannels; ++channelIdx ) {
			const quint32  sample = (quint32) (buffer[sampleIdx] * 8388608); // Recall buffer type is double
			const quint8  part1 = sample & 0xff;
			const quint8  part2 = (sample >> 8) & 0xff;
			const quint8  part3 = (sample >> 16) & 0xff;
			out << part1 << part2 << part3;
		}
	}
	return true;
}

const char*
AudioFileWriter::errorString() {
	return errorMessage.c_str();
}
