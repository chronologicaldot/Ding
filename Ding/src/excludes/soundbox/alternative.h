using list_t<T> = std::vector<T>;

struct s8SoundByte {
	typedef  signed char  type;
	static const double  range = 127;
};

struct s32SoundByte {
	typedef  signed long  type;
	static const double  range = 2147483647;
};

struct f64SoundByte {
	typedef  double  type;
	static const double  range = 1;
};

struct InterleavingSampler {
	template<class FromFormat, double FromRange, class ToFormat, double ToRange>
	void getOutputSamples(
		list_t<FromFormat>&  source, int channel, int numChannels, void*  output,
		unsigned  frames, double  streamTime
	) {
		ToFormat*  out = static_cast<ToFormat*>(output);
		unsigned i;
		for(i=channel; i < frames; i += numChannels)
			out = static_cast<ToFormat>( ToRange * source[i] / FromRange );
	}
};

struct NonInterleavingSampler {
	template<class FromFormat, double FromRange, class ToFormat, double ToRange>
	void getOutputSamples(
		list_t<FromFormat>&  source, int channel, int numChannels, void*  output,
		unsigned  frames, double  streamTime
	) {
		ToFormat*  out = static_cast<ToFormat*>(output);
		unsigned i;
		for(i=channel; i < frames; i += frames / numChannels)
			out = static_cast<ToFormat>( ToRange * source[i] / FromRange );
	}
}


template<class OutputFormat, class InternalFormat, class OutputSampler>
struct Audio {

	typedef  OutputFormat  OutputType;
	typedef  InternalFormat  InternalType;

	int
	handleAudioInOut( void* output, void* input, unsigned frames, double streamTime, RtAudioStreamStatus status ) {
		if ( status )
			return 1;

		OutputSampler::getOutputSamples<
					InternalFormat::type, InternalFormat::range, OutputFormat::type, OutputFormat::range
					>
					( leftBuffer, 0, 2, output, frames, streamTime );

		OutputSampler::getOutputSamples<
					InternalFormat::type, InternalFormat::range, OutputFormat::type, OutputFormat::range
					>
					( rightBuffer, 1, 2, output, frames, streamTime );

		return 0;
	}

private:
	list_t<InternalType>  leftBuffer;
	list_t<InternalType>  rightBuffer;

};

// Usage:
// Audio<s32SoundByte, f64SoundByte, InterleavingSampler>  audio;
