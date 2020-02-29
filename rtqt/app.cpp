#include "app.h"
#ifdef USE_QT
#include <QVBoxLayout>
#endif

// Platform-dependent sleep routines.
#if defined( __WINDOWS_ASIO__ ) || defined( __WINDOWS_DS__ ) || defined( __WINDOWS_WASAPI__ )
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

App::App()
	: audio()
#ifdef USE_QT
	, streamButton(nullptr)
	, playbackButton(nullptr)
	, streamLabel(nullptr)
	, playbackLabel(nullptr)
#endif
	, on(false)
	, playing(false)
	, emptyChannels(0)
{
	audio.setOutputStreamName("test app out");
	//audio.setOutputParameters( RtAudio::LINUX_ALSA, FRAMES, 44100 );
	audio.setOutputParameters( RtAudio::LINUX_ALSA, 512, 44100 );
	audio.setListener(this);

#ifdef USE_QT
	streamButton = new QPushButton("Stream Toggle");
	playbackButton = new QPushButton("Play/Stop");
	streamLabel = new QLabel("Stream stopped");
	playbackLabel = new QLabel("Play stopped");

	connect( streamButton, SIGNAL(clicked()), this, SLOT(toggleStream()) );
	connect( playbackButton, SIGNAL(clicked()), this, SLOT(togglePlayback()) );

	QVBoxLayout*  layout = new QVBoxLayout;
	layout->addWidget(streamButton);
	layout->addWidget(streamLabel);
	layout->addWidget(playbackButton);
	layout->addWidget(playbackLabel);
	setLayout(layout);
#endif

	lastValues[0] = 0;
	lastValues[1] = 0;
}

App::~App() {
	audio.close();
}

void
App::run() {
	toggleStream();
	togglePlayback();
}

void
App::toggleStream() {
	if ( on ) {
		audio.close();
		on = !on;
#ifdef USE_QT
		streamLabel->setText("Stream stopped");
#endif
	} else {
		on = audio.open();
#ifdef USE_QT
		if ( on )
			streamLabel->setText("Stream started");
		else
			streamLabel->setText("Stream not started");
#endif
	}
}

void
App::togglePlayback() {
	if ( playing ) {
		audio.stop();
		playing = !playing;
#ifdef USE_QT
		playbackLabel->setText("Play stopped");
#endif
	} else {
		fillBuffer();
		audio.loadChannelBuffer(0, buffer[0], FRAMES);
		audio.loadChannelBuffer(1, buffer[1], FRAMES);
		playing = audio.start();
#ifdef USE_QT
		if ( playing )
			playbackLabel->setText("Play started");
		else
			playbackLabel->setText("Play not started");
#else
		//while ( audio.isRunning() )
			//SLEEP( 100 );

		SLEEP(5000);
		audio.stop();
#endif

#ifdef USE_QT
		playbackLabel->setText("Play done");
#endif
	}
}

void
App::OnSoundBoxChannelEmpty( soundbox::SoundBox::u8 ) {
	if ( ++emptyChannels == 1 ) {
	//if ( ++emptyChannels == 2 ) {
		playing = false;
		emptyChannels = 0;
	}
}

void
App::fillBuffer() {
	unsigned int i, j;
	// Write interleaved audio data.
	for ( i=0; i<FRAMES; i++ ) {
		for ( j=0; j<2; j++ ) {
			buffer[j][i] = lastValues[j];
			lastValues[j] += 0.005 * (j+1+(j*0.1));
			if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
		}
	}
}
