// (c) 2019 Nicolaus Anderson

#ifndef QT_APP_WINDOW_H
#define QT_APP_WINDOW_H

#define  APP_VERSION 1.0

#include "../ding/ding_waveform_types.h"
#include "../ding/ding_basic_wavesource.h"
#include "../ding/ding_bell_maker.h"
#include "../soundbox/SoundBox.h"
#include "wave_render_panel.h"
#include "bell_render_panel.h"
#include <QString>
#include <QWidget>
#include <QCloseEvent>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QLabel>
#include <QStackedLayout>
#include <memory>

//#define DEBUG_SOUND 1

using ding::play_t;
using ding::volume_t;

//! Wave Panel list item wrapper
struct WavePanelContainer {
	WavePanelContainer( WaveRenderPanel* p )
		: panel(p)
	{}

	// Copy constructor is defaulted to copying the pointer, no reference-counting.
	// Deconstructor should not destroy panel because it is called after a copy.

	WaveRenderPanel&  get() {
		return *panel;
	}

	void  kill() {
		if ( panel )
			delete panel;
		panel = nullptr;
	}

	ding::BaseWaveform::Value  getBaseForm() {
		ding::BasicWaveSource*  sampleSource;
		if ( panel ) {
			sampleSource = (ding::BasicWaveSource*) panel->getWave().getSampleSource().get();
			if ( sampleSource ) {
				return sampleSource->getWaveformType();
			}
		}
		return ding::BaseWaveform::flat;
	}

private:
	WaveRenderPanel*  panel;
};

//! Main Application
class AppWindow
	: public QWidget
	, public soundbox::SoundBox::Listener
	, public WaveRenderPanel::Listener
{
	Q_OBJECT

public:

	//AppWindow(int argc, char* argv[]);
	AppWindow();
	~AppWindow();

	virtual void  OnVolumeHandleChanged( index_t, play_t, volume_t ) override;
	virtual void  OnSoundBoxChannelEmpty( soundbox::SoundBox::u8 ) override;
	virtual void  closeEvent( QCloseEvent* ) override;

private slots:
	void  toggleAudioPlayback(bool);
	void  newWaveCreated();
	void  wavePanelSelected(int);
	void  waveRemoved();
	void  waveformChanged(int);
	void  waveFrequencyChanged(double);
	void  wavePhaseChanged(double);
	void  wavePhaseChangedScaled(int);
	void  handleAdded();
	void  handleRemoved();
	void  cornerHandlesZeroed();
	void  allHandlesZeroed();
	void  allHandlesMaxed();
	void  smoothingToggled(bool);
	void  soundAPIChanged(int);
	void  pixelsPerSampleChanged(int);
	void  pixelsPerFullSampleChanged(int);
	void  durationChanged( bool changeAll = true);
	void  durationActivelyChanged(double);
	void  newProjectStart();
	void  loadProjectStart();
	void  saveProjectStart();
	void  exportSoundStart();

private:
	void  createNewPanel( ding::Wave&, const char* );
	void  deleteActiveWavePanel();
	ding::BaseWaveform::Value  getSelectedWaveform();
	ding::BaseWaveform::Value  getActiveWaveForm();
	void  resetActiveIndex( size_t  index = 0 );
	void  initActiveWavePanel();
	void  showActiveWavePanel(); // Called by initActiveWavePanel().
	//void  hideActiveWavePanel(); // Called by initActiveWavePanel().
	void  hideInactiveWavePanels();
	void  reloadWaveControls();
	void  resetActiveAudioChannels();
	bool  isAnAudioChannelActive();
	void  onAudioStarted();
	void  onAudioFinished();
	soundbox::SoundBox::PlaybackApi  getSoundAPIFromIndex(int);

	void  changeMade();
	void  setCurrentFile( const QString& );
	void  newProject();
	bool  loadProject( const QString& );
	bool  maybeStay();
	bool  saveProjectStep2();
	bool  saveProject( const QString& );
	bool  writeAudioFile( const QString& );

private:
	void  reportNoActiveWavePanel();

	// Members -------------------------
	ding::BellMaker  bellMaker;
	std::vector<WavePanelContainer>  waveRenderPanels;
	WaveRenderPanel*  activeWavePanel;
	size_t  activeWaveIndex;
	bool  freezeActiveWaveIndex;
	BellRenderPanel*  fullBellPanel;

	// Audio ---------------------------
	bool  isAudioPlaying;			// Application/GUI side
	soundbox::SoundBox  soundBox;
	static constexpr size_t  MAX_AUDIO_CHANNELS = 2;
	bool  activeChannels[MAX_AUDIO_CHANNELS];

	// Debug
#ifdef DEBUG_SOUND
	void  fillBuffer(soundbox::SampleBuffer&, unsigned char);
	double  lastValues[2];
#endif

	// IO ------------------------------
	bool  unsavedProject;

	// GUI -----------------------------
	static constexpr double  PHASE_SLIDER_SCALE = 1000;
	bool  freezeWavePhaseSlider;
	QString  currentFile;

	QWidget*  wavePanelsParent;
	QStackedLayout*  wavePanelsLayout;

	QPushButton*  togglePlaybackButton;
	QPushButton*  addWaveButton;
	QPushButton*  removeWaveButton;
	QComboBox*  waveComboBox;
	QComboBox*  waveformComboBox;
	QLineEdit*  waveNameBox;
	QDoubleSpinBox*  waveFrequencyBox;
	QDoubleSpinBox*  wavePhaseBox;
	QPushButton*  addHandleButton;
	QPushButton*  removeHandleButton;
	QPushButton*  zeroCornerHandlesButton;
	QPushButton*  zeroAllHandlesButton;
	QPushButton*  maxAllHandlesButton;
	QCheckBox*  smoothingCheckBox;
	QComboBox*  soundAPIComboBox;
	QLabel*  soundAPIavailableLabel;
	QSpinBox*  pixelsPerSampleSpinBox;
	QSpinBox*  pixelsPerFullSampleSpinBox;
	QSlider*  wavePhaseSlider;
	QDoubleSpinBox*  durationSpinBox;
	QPushButton*  confirmDurationButton;
	QPushButton*  newProjectButton;
	QPushButton*  loadProjectButton;
	QPushButton*  saveProjectButton;
	QPushButton*  exportSoundButton;

	// Debug ----------------------------
	//int lastWaveId;
};

#endif // QT_APP_WINDOW_H
