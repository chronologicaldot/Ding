#define USE_QT 1

#ifndef APP_H
#define APP_H

//#include <../Ding/src/soundbox/SoundBox.h>
#include <SoundBox.h>

#ifdef USE_QT
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#endif

class App
#ifdef USE_QT
	: public QWidget
	, public soundbox::SoundBox::Listener
#else
	: public soundbox::SoundBox::Listener
#endif
{
#ifdef USE_QT
	Q_OBJECT
#endif

public:
	App();
	~App();
	void  run();
	virtual void  OnSoundBoxChannelEmpty( soundbox::SoundBox::u8 ) override;

#ifdef USE_QT
private slots:
#endif
	void  toggleStream();
	void  togglePlayback();

private:
	void  fillBuffer();

	soundbox::SoundBox  audio;
#ifdef USE_QT
	QPushButton*  streamButton;
	QPushButton*  playbackButton;
	QLabel*  streamLabel;
	QLabel*  playbackLabel;
#endif
	bool  on;
	bool  playing;
	unsigned  emptyChannels;
	static constexpr unsigned  FRAMES = 44100*5; //2048; //512;
	double  buffer[2][FRAMES];
	double  lastValues[2];
};

#endif
