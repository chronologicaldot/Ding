// (c) 2019 Nicolaus Anderson

#include "app_window.h"
#include "io_unit.h"
#include "audio_file_writer.h"
#include "../ding/ding_volume_span.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter> // For QtPalette::Base
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#ifndef QT_NO_CURSOR
#include <QApplication> // For the cursor change
#endif

//#include <cstdio>

#ifdef DEBUG_SOUND
#include <cmath>
#endif

class RemovingWhenZeroWavesException {};
class DeletingWhenZeroPanelsException {};

//AppWindow::AppWindow(int argc, char* argv[])
AppWindow::AppWindow()
	: bellMaker()
	, waveRenderPanels()
	, activeWavePanel(nullptr)
	, activeWaveIndex(0)
	, freezeActiveWaveIndex(false)
	, fullBellPanel(nullptr)
	, isAudioPlaying(false)
	, soundBox()
	, unsavedProject(false)
	, freezeWavePhaseSlider(false)
	, currentFile("")
	, wavePanelsParent(nullptr)
	, wavePanelsLayout(nullptr)
	, togglePlaybackButton(nullptr)
	, addWaveButton(nullptr)
	, removeWaveButton(nullptr)
	, waveComboBox(nullptr)
	, waveformComboBox(nullptr)
	, waveNameBox(nullptr)
	, waveFrequencyBox(nullptr)
	, wavePhaseBox(nullptr)
	, addHandleButton(nullptr)
	, removeHandleButton(nullptr)
	, zeroCornerHandlesButton(nullptr)
	, zeroAllHandlesButton(nullptr)
	, maxAllHandlesButton(nullptr)
	, smoothingCheckBox(nullptr)
	, soundAPIComboBox(nullptr)
	, soundAPIavailableLabel(nullptr)
	, pixelsPerSampleSpinBox(nullptr)
	, pixelsPerFullSampleSpinBox(nullptr)
	, wavePhaseSlider(nullptr)
	, durationSpinBox(nullptr)
	//, confirmDurationButton(nullptr)
	, newProjectButton(nullptr)
	, loadProjectButton(nullptr)
	, saveProjectButton(nullptr)
	, exportSoundButton(nullptr)
	//, lastWaveId(1)
{
	resetActiveAudioChannels();

	const int  wavePhaseMinimum = -1;
	const int  wavePhaseMaximum = 1;

	// UI element initializations --------------
	togglePlaybackButton = new QPushButton(tr("Play/Stop"));
	togglePlaybackButton->setCheckable(true);
	addWaveButton = new QPushButton(tr("Add Wave"));
	removeWaveButton = new QPushButton(tr("Remove Wave"));
	//confirmDurationButton = new QPushButton(tr("Set Duration"));
	addHandleButton = new QPushButton(tr("Add Handle"));
	removeHandleButton = new QPushButton(tr("Remove Handle"));
	zeroCornerHandlesButton = new QPushButton(tr("Zero Corners"));
	zeroAllHandlesButton = new QPushButton(tr("Zero All"));
	maxAllHandlesButton = new QPushButton(tr("Maximize All"));
	smoothingCheckBox = new QCheckBox(tr("Smooth Volume"));
	newProjectButton = new QPushButton(tr("New Project"));
	loadProjectButton = new QPushButton(tr("Open Project"));
	saveProjectButton = new QPushButton(tr("Save Project"));
	exportSoundButton = new QPushButton(tr("Export Sound"));

	durationSpinBox = new QDoubleSpinBox;
	durationSpinBox->setSuffix(" sec");
	durationSpinBox->setDecimals(4);
	durationSpinBox->setMinimum(0);
	durationSpinBox->setMaximum(1000);
	durationSpinBox->setSingleStep(0.1);
	durationSpinBox->setValue(1);

	QLabel*  durationLabel = new QLabel(tr("Duration"));

	waveComboBox = new QComboBox;

	//QLabel*  waveLabel = new QLabel(tr("Wave"));
	//waveLabel->setBuddy( waveComboBox );

	waveformComboBox = new QComboBox;
	waveformComboBox->addItem(tr("Sine"));
	waveformComboBox->addItem(tr("Cosine"));
	waveformComboBox->addItem(tr("Regular Saw"));
	waveformComboBox->addItem(tr("Forward Saw"));
	waveformComboBox->addItem(tr("Backward Saw"));
	waveformComboBox->addItem(tr("Square"));

	waveNameBox = new QLineEdit(tr("Base Wave"));
	waveNameBox->setPlaceholderText("wave name");

	waveFrequencyBox = new QDoubleSpinBox;
	waveFrequencyBox->setSuffix(" hz");
	waveFrequencyBox->setDecimals(0);
	waveFrequencyBox->setRange(0, 10000);
	waveFrequencyBox->setSingleStep(1);
	waveFrequencyBox->setValue(1);

	QLabel*  waveFrequencyLabel = new QLabel(tr("Frequency"));

	wavePhaseBox = new QDoubleSpinBox;
	wavePhaseBox->setSuffix(" cycle");
	wavePhaseBox->setDecimals(4);
	wavePhaseBox->setRange(wavePhaseMinimum, wavePhaseMaximum);
	wavePhaseBox->setSingleStep(0.01);
	wavePhaseBox->setValue(0);

	QLabel*  wavePhaseLabel = new QLabel(tr("Phase"));

	soundAPIComboBox = new QComboBox;
	soundAPIComboBox->addItem(tr("[unspecified]"));
	soundAPIComboBox->addItem(tr("ALSA (Linux)"));
	soundAPIComboBox->addItem(tr("Pulse Audio (Linux)"));
	soundAPIComboBox->addItem(tr("OSS (Linux)"));
	soundAPIComboBox->addItem(tr("JACK (Unix)"));
	soundAPIComboBox->addItem(tr("OSX Core (Mac)"));
	soundAPIComboBox->addItem(tr("WASAPI (Windows)"));
	soundAPIComboBox->addItem(tr("ASIO (Windows)"));
	soundAPIComboBox->addItem(tr("DirectSound (Windows)"));
	soundAPIComboBox->addItem(tr("[dummy]"));

	QLabel*  soundAPILabel = new QLabel(tr("API"));

	soundAPIavailableLabel = new QLabel(QString("unset"));

	QLabel*  pixelsPerSampleLabel = new QLabel(tr("Points per Sample"));

	pixelsPerSampleSpinBox = new QSpinBox;
	pixelsPerSampleSpinBox->setSuffix(" pps");
	pixelsPerSampleSpinBox->setSingleStep(1);
	pixelsPerSampleSpinBox->setValue(0);
	pixelsPerSampleSpinBox->setRange(1, 200);

	QLabel*  pixelsPerFullSampleLabel = new QLabel(tr("Points per Full Sample"));

	pixelsPerFullSampleSpinBox = new QSpinBox;
	pixelsPerFullSampleSpinBox->setSuffix(" pps");
	pixelsPerFullSampleSpinBox->setSingleStep(1);
	pixelsPerFullSampleSpinBox->setValue(0);
	pixelsPerFullSampleSpinBox->setRange(1, 200);

	wavePhaseSlider = new QSlider(Qt::Horizontal);
	wavePhaseSlider->setRange(wavePhaseMinimum * PHASE_SLIDER_SCALE, wavePhaseMaximum * PHASE_SLIDER_SCALE);
	wavePhaseSlider->setValue(0);

	QLabel*  wavePhaseSliderLabel = new QLabel(tr("Phase"));

	// UI callback connections -----------------
	connect( togglePlaybackButton, SIGNAL(toggled(bool)), this, SLOT(toggleAudioPlayback(bool)) );
	connect( addWaveButton, SIGNAL(clicked()), this, SLOT(newWaveCreated()) );
	connect( removeWaveButton, SIGNAL(clicked()), this, SLOT(waveRemoved()) );
	connect( waveComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(wavePanelSelected(int)) );
	connect( waveformComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(waveformChanged(int)) );
	connect( waveFrequencyBox, SIGNAL(valueChanged(double)), this, SLOT(waveFrequencyChanged(double)) );
	connect( wavePhaseBox, SIGNAL(valueChanged(double)), this, SLOT(wavePhaseChanged(double)) );
	connect( addHandleButton, SIGNAL(clicked()), this, SLOT(handleAdded()) );
	connect( removeHandleButton, SIGNAL(clicked()), this, SLOT(handleRemoved()) );
	connect( zeroCornerHandlesButton, SIGNAL(clicked()), this, SLOT(cornerHandlesZeroed()) );
	connect( zeroAllHandlesButton, SIGNAL(clicked()), this, SLOT(allHandlesZeroed()) );
	connect( maxAllHandlesButton, SIGNAL(clicked()), this, SLOT(allHandlesMaxed()) );
	connect( smoothingCheckBox, SIGNAL(toggled(bool)), this, SLOT(smoothingToggled(bool)) );
	connect( soundAPIComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(soundAPIChanged(int)) );
	connect( pixelsPerSampleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(pixelsPerSampleChanged(int)) );
	connect( pixelsPerFullSampleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(pixelsPerFullSampleChanged(int)) );
	connect( wavePhaseSlider, SIGNAL(valueChanged(int)), this, SLOT(wavePhaseChangedScaled(int)) );
	connect( durationSpinBox, SIGNAL(valueChanged(double)), this, SLOT(durationActivelyChanged(double)) );
	//connect( confirmDurationButton, SIGNAL(clicked()), this, SLOT(durationChanged()) );
	connect( newProjectButton, SIGNAL(clicked()), this, SLOT(newProjectStart()) );
	connect( loadProjectButton, SIGNAL(clicked()), this, SLOT(loadProjectStart()) );
	connect( saveProjectButton, SIGNAL(clicked()), this, SLOT(saveProjectStart()) );
	connect( exportSoundButton, SIGNAL(clicked()), this, SLOT(exportSoundStart()) );

	// Grouping and Layouts ---------------------
	QGroupBox*  waveGroup = new QGroupBox(tr("Waves"));
	QGroupBox*  handleControlsGroup = new QGroupBox(tr("Volume")); // was "Handles"
	QGroupBox*  soundAPIGroup = new QGroupBox(tr("Audio"));
	QGroupBox*  renderDisplayGroup = new QGroupBox(tr("Display"));
	QGroupBox*  soundControlsGroup = new QGroupBox;
	QGroupBox*  wavePhaseSliderGroup = new QGroupBox;

	// Wave controls ----------------------------
	QVBoxLayout*  waveLayout = new QVBoxLayout;
	waveLayout->setAlignment( Qt::AlignTop );
	//waveLayout->addWidget( waveLabel );
	waveLayout->addWidget( waveComboBox );
	waveLayout->addWidget( waveformComboBox );
	waveLayout->addWidget( removeWaveButton );

	QHBoxLayout*  waveFrequencyLayout = new QHBoxLayout;
	waveFrequencyLayout->addWidget( waveFrequencyLabel );
	waveFrequencyLayout->addWidget( waveFrequencyBox );

	QHBoxLayout*  wavePhaseLayout = new QHBoxLayout;
	wavePhaseLayout->addWidget( wavePhaseLabel );
	wavePhaseLayout->addWidget( wavePhaseBox );

	QHBoxLayout*  createWaveLayout = new QHBoxLayout;
	createWaveLayout->addWidget( waveNameBox );
	createWaveLayout->addWidget( addWaveButton );

	waveLayout->addLayout( waveFrequencyLayout );
	waveLayout->addLayout( wavePhaseLayout );
	waveLayout->addLayout( createWaveLayout );
	waveGroup->setLayout( waveLayout );

	// Volume Modifier controls ------------------
	QVBoxLayout*  handleControlsLayout = new QVBoxLayout;
	handleControlsLayout->setAlignment( Qt::AlignTop );
	handleControlsLayout->addWidget( addHandleButton );
	handleControlsLayout->addWidget( removeHandleButton );
	handleControlsLayout->addWidget( zeroCornerHandlesButton );
	handleControlsLayout->addWidget( zeroAllHandlesButton );
	handleControlsLayout->addWidget( maxAllHandlesButton );
	handleControlsLayout->addWidget( smoothingCheckBox );
	handleControlsGroup->setLayout( handleControlsLayout );

	// Sound API controls ------------------------
	QVBoxLayout*  soundAPILayout = new QVBoxLayout;
	soundAPILayout->setAlignment( Qt::AlignTop );

	QHBoxLayout*  soundAPIChoiceLayout = new QHBoxLayout;
	soundAPIChoiceLayout->addWidget( soundAPILabel );
	soundAPIChoiceLayout->addWidget( soundAPIComboBox );

	soundAPILayout->addLayout( soundAPIChoiceLayout );
	soundAPILayout->addWidget( soundAPIavailableLabel );
	soundAPIGroup->setLayout( soundAPILayout );

	// Render Display controls -------------------
	QVBoxLayout*  renderDisplayLayout = new QVBoxLayout;
	renderDisplayLayout->setAlignment( Qt::AlignTop );

	QHBoxLayout*  renderDisplayChoiceLayout = new QHBoxLayout;
	renderDisplayChoiceLayout->addWidget( pixelsPerSampleLabel );
	renderDisplayChoiceLayout->addWidget( pixelsPerSampleSpinBox );

	QHBoxLayout*  renderDisplayChoiceLayout2 = new QHBoxLayout;
	renderDisplayChoiceLayout2->addWidget( pixelsPerFullSampleLabel );
	renderDisplayChoiceLayout2->addWidget( pixelsPerFullSampleSpinBox );

	renderDisplayLayout->addLayout( renderDisplayChoiceLayout );
	renderDisplayLayout->addLayout( renderDisplayChoiceLayout2 );
	renderDisplayGroup->setLayout( renderDisplayLayout );

	// Display and Output controls ---------------
	QVBoxLayout*  outputControlsLayout = new QVBoxLayout;
	outputControlsLayout->setAlignment( Qt::AlignTop );
	outputControlsLayout->addWidget( soundAPIGroup );
	outputControlsLayout->addWidget( renderDisplayGroup );

	// Controls under the main view --------------
	QHBoxLayout*  soundControlsLayout = new QHBoxLayout;
	soundControlsLayout->addWidget( togglePlaybackButton );
	soundControlsLayout->addWidget( durationLabel );
	soundControlsLayout->addWidget( durationSpinBox );
	//soundControlsLayout->addWidget( confirmDurationButton );
	soundControlsGroup->setLayout( soundControlsLayout );

	// Top bar menu -------------------------------
	QHBoxLayout*  menuLayout = new QHBoxLayout;
	menuLayout->addWidget( newProjectButton );
	menuLayout->addWidget( loadProjectButton );
	menuLayout->addWidget( saveProjectButton );
	menuLayout->addWidget( exportSoundButton );

	// Wave phase section -------------------------
	QHBoxLayout*  wavePhaseSliderLayout = new QHBoxLayout;
	wavePhaseSliderLayout->addWidget( wavePhaseSliderLabel );
	wavePhaseSliderLayout->addWidget( wavePhaseSlider );
	wavePhaseSliderGroup->setLayout( wavePhaseSliderLayout );

	// Grouped controls ---------------------------
	QHBoxLayout*  lowerControlsLayout = new QHBoxLayout;
	lowerControlsLayout->addWidget( waveGroup );
	lowerControlsLayout->addWidget( handleControlsGroup );
	lowerControlsLayout->addLayout( outputControlsLayout );

	// Main waveform view -------------------------
	QWidget*  fullPanelsParent = new QWidget;
	fullPanelsParent->setFixedSize(600,300);
	fullPanelsParent->setBackgroundRole(QPalette::Base);
	fullPanelsParent->setAutoFillBackground(true);

	fullBellPanel = new BellRenderPanel( fullPanelsParent );
	fullBellPanel->setFixedSize(600,300);
	fullBellPanel->setBellMaker( bellMaker );
	fullBellPanel->ConstantRefresh = true;

	wavePanelsLayout = new QStackedLayout;
	wavePanelsParent = new QWidget( fullPanelsParent );
	wavePanelsParent->setFixedSize(600,300);
	wavePanelsParent->setLayout( wavePanelsLayout );

	// Assembled GUI ------------------------------
	QVBoxLayout*  mainUILayout = new QVBoxLayout;
	mainUILayout->addLayout( menuLayout );
	mainUILayout->addWidget( fullPanelsParent );
	mainUILayout->addWidget( wavePhaseSliderGroup );
	mainUILayout->addWidget( soundControlsGroup );
	mainUILayout->addLayout( lowerControlsLayout );

	setLayout( mainUILayout );

	const unsigned  initialSampleRate = 44100;
	const unsigned  initialBufferFrames = 512;

	// At minimum, there must always be one wave.
	newWaveCreated();
	waveNameBox->setText("New Wave");

	setWindowTitle(tr("Bell Maker"));

	// Audio Start ------------------------
	// Done initializing objects (like soundAPIComboBox) modified by OnSoundBoxChannelEmpty.

	soundBox.setListener(this);
	soundBox.setOutputStreamName("ding output");
	soundBox.setOutputParameters( RtAudio::UNSPECIFIED, initialBufferFrames, initialSampleRate );
	bellMaker.setSamplesPerSecond( initialSampleRate );

	pixelsPerFullSampleSpinBox->setValue( fullBellPanel->getPixelsPerSample() );

#ifdef DEBUG_SOUND
	lastValues[0] = 0;
	lastValues[1] = 0;
#endif
}

AppWindow::~AppWindow()
{
	soundBox.stop();
	soundBox.close();
}

void
AppWindow::closeEvent( QCloseEvent*  event ) {
	if ( maybeStay() ) {
		event->ignore();
	} else {
		event->accept();
	}
}

void
AppWindow::OnVolumeHandleChanged( index_t, play_t, volume_t ) {
	changeMade();
}

void
AppWindow::OnSoundBoxChannelEmpty( soundbox::SoundBox::u8  channel ) {
	activeChannels[channel] = false;
	if ( ! isAnAudioChannelActive() ) {
		onAudioFinished();
	}
}

void
AppWindow::resetActiveAudioChannels() {
	size_t  c = 0;
	for (; c < MAX_AUDIO_CHANNELS; ++c)
		activeChannels[c] = false;
}

bool
AppWindow::isAnAudioChannelActive() {
	size_t  c = 0;
	for (; c < MAX_AUDIO_CHANNELS; ++c)
		if ( activeChannels[c] )
			return true;
	return false;
}

void
AppWindow::onAudioStarted() {
	soundAPIComboBox->setEnabled(false);
}

void
AppWindow::onAudioFinished() {
	togglePlaybackButton->setChecked(false);
	soundAPIComboBox->setEnabled(true);
}

soundbox::SoundBox::PlaybackApi
AppWindow::getSoundAPIFromIndex( int  which ) {
	switch( which )
	{
	case 1:
		return RtAudio::LINUX_ALSA;

	case 2:
		return RtAudio::LINUX_PULSE;

	case 3:
		return RtAudio::LINUX_OSS;

	case 4:
		return RtAudio::UNIX_JACK;

	case 5:
		return RtAudio::MACOSX_CORE;

	case 6:
		return RtAudio::WINDOWS_WASAPI;

	case 7:
		return RtAudio::WINDOWS_ASIO;

	case 8:
		return RtAudio::WINDOWS_DS;

	case 9:
		return RtAudio::RTAUDIO_DUMMY;

	default:
		return RtAudio::UNSPECIFIED;
	}
}

void
AppWindow::toggleAudioPlayback(bool  on) {
	if ( on ) {
		//if ( soundBox.isRunning() )
		//	return;

		// Synchronize currently active wave panel with soundBox.
		bellMaker.loadIntoContainer<soundbox::SampleBuffer>( soundBox.getChannelBuffer(0), 0 );
		soundBox.duplicateChannel(0,1);

		//fillBuffer( soundBox.getChannelBuffer(0), 0 );
		//fillBuffer( soundBox.getChannelBuffer(1), 1 );

		soundBox.open();
		if ( soundBox.start() ) {
			activeChannels[0] = true;
			activeChannels[1] = true;
			isAudioPlaying = true;
			onAudioStarted();
			//std::cout << "Soundbox started" << std::endl;
		}
		else {
			//std::cout << "Soundbox failed to start" << std::endl;
			onAudioFinished();
			soundBox.close();
		}
	} else {
		if ( soundBox.isRunning() ) {
			soundBox.stop();
			soundBox.close();
		}
		isAudioPlaying = false;
		soundAPIComboBox->setEnabled(true);
		// No need to call onAudioFinished(). togglePlaybackButton->setChecked() may cause recursion.
		//std::cout << "Soundbox stopped" << std::endl;
	}
}

void
AppWindow::newWaveCreated() {
	activeWaveIndex = bellMaker.getWaveCount();
	ding::Wave&  wave = bellMaker.getWave( activeWaveIndex );
	wave.initNewVolumeMultiplier();
	wave.setFrequency(720);

	const ding::BaseWaveform::Value  startingWaveform = ding::BaseWaveform::sine;
	bellMaker.setBasicWaveSource( activeWaveIndex, startingWaveform );

	wave.Name = waveNameBox->text().toStdString();
	createNewPanel(wave, wave.Name.c_str());
	//QString("%1").arg(lastWaveId++)
}

void
AppWindow::createNewPanel( ding::Wave&  wave, const char*  waveName ) {
	WaveRenderPanel*  newPanel = new WaveRenderPanel( wavePanelsParent );
	newPanel->setFixedSize( wavePanelsParent->size() );
	newPanel->NormalColor = Qt::gray;
	//newPanel->HighlightColor = Qt::green;
	newPanel->HandleNormalColor = Qt::gray;
	newPanel->HandleHighlightColor = Qt::black; // Qt::blue;
	newPanel->HandleSize = 10;

	std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	if ( volumeMultiplier.get() ) {
		newPanel->setHandleCount( volumeMultiplier.get()->getSettingCount() );
	}

	newPanel->setWave( wave );
	newPanel->setListener( this );
	wavePanelsLayout->addWidget( newPanel );

	waveRenderPanels.push_back( WavePanelContainer( newPanel ) );
	activeWaveIndex = waveRenderPanels.size() - 1;

	waveComboBox->addItem( waveName );
	waveComboBox->setCurrentIndex( activeWaveIndex );
	durationSpinBox->setValue( wave.getDuration() );
	initActiveWavePanel();
	changeMade();
}

void
AppWindow::wavePanelSelected(int  index) {
	if ( freezeActiveWaveIndex || index < 0 || (size_t)index >= waveRenderPanels.size() )
		return;
	activeWaveIndex = (size_t)index;
	initActiveWavePanel();
}

void
AppWindow::waveRemoved() {
	// Leave at least one
	if ( bellMaker.getWaveCount() == 1 ) {
		waveComboBox->removeItem(0);
		waveComboBox->addItem(QString("Base Wave"));
		waveformChanged(0);
		activeWavePanel->setWave( bellMaker.getWave(0) ); // Refresh
		activeWavePanel->setHandleCount(0);
		reloadWaveControls();
		return;
	}
	if ( bellMaker.getWaveCount() == 0 ) {
		throw RemovingWhenZeroWavesException{};
	}

	freezeActiveWaveIndex = true; // Prevent wave panel selection from changing by callback
	waveComboBox->removeItem(activeWaveIndex);
	freezeActiveWaveIndex = false;
	bellMaker.removeWave(activeWaveIndex);

	deleteActiveWavePanel();
	resetActiveIndex();
	changeMade();
}

void
AppWindow::deleteActiveWavePanel() {
	if ( waveRenderPanels.size() == 0 ) {
		throw DeletingWhenZeroPanelsException{};
	}
	QLayoutItem*  w = wavePanelsLayout->takeAt(activeWaveIndex);
	if ( w )
		delete w;
	waveRenderPanels[activeWaveIndex].kill();
	waveRenderPanels.erase( waveRenderPanels.begin() + activeWaveIndex );
	// UNSAFE TO CALL resetActiveIndex() HERE
	// This may be the last wave (as performed in loadProject())
}

void
AppWindow::waveformChanged( int  typeIndex ) {
	if ( typeIndex < 0 ) return;
	// Add 1 to skip "flat"
	const ding::BaseWaveform::Value  form = ding::getWaveformFromIndex(typeIndex + 1);
	bellMaker.setBasicWaveSource( activeWaveIndex, form );
	activeWavePanel->setWave( bellMaker.getWave( activeWaveIndex ) ); // Refresh
	changeMade();
}

void
AppWindow::waveFrequencyChanged( double  newValue ) {
	bellMaker.getWave( activeWaveIndex ).setFrequency( newValue );
	activeWavePanel->setFrequency( static_cast<ding::play_t>(newValue) );
	changeMade();
}

void
AppWindow::wavePhaseChanged( double  newValue ) {
	bellMaker.getWave( activeWaveIndex ).setPhaseShift( newValue );
	activeWavePanel->setPhaseShift( static_cast<ding::play_t>(newValue) );
	if ( ! freezeWavePhaseSlider ) {
		wavePhaseSlider->setValue( newValue * PHASE_SLIDER_SCALE );
	}
	changeMade();
}

void
AppWindow::wavePhaseChangedScaled( int  newValue ) {
	const double  trueValue = static_cast<double>(newValue) / PHASE_SLIDER_SCALE;
	freezeWavePhaseSlider = true;
	wavePhaseChanged(trueValue);
	wavePhaseBox->setValue( trueValue );
	freezeWavePhaseSlider = false;
}

void
AppWindow::handleAdded() {
	activeWavePanel->setHandleCount( activeWavePanel->getHandleCount() + 1 );
	changeMade();
}

void
AppWindow::handleRemoved() {
	if ( activeWavePanel->getHandleCount() > 0 )
		activeWavePanel->setHandleCount( activeWavePanel->getHandleCount() - 1 );
	changeMade();
}

void
AppWindow::cornerHandlesZeroed() {
	activeWavePanel->setEndcapHandlesToZero();
	changeMade();
}

void
AppWindow::allHandlesZeroed() {
	activeWavePanel->setHandlesToVolume(0);
	changeMade();
}

void
AppWindow::allHandlesMaxed() {
	activeWavePanel->setHandlesToVolume(1);
	changeMade();
}

void
AppWindow::smoothingToggled( bool  setting ) {
	activeWavePanel->setVolumeSmoothingEnabled(setting);
	changeMade();
}

void
AppWindow::soundAPIChanged( int  which ) {
	soundbox::SoundBox::PlaybackApi  newApi = getSoundAPIFromIndex(which);
	soundBox.setOutputParameters(newApi);
	if ( soundBox.isAPISupported(newApi) ) {
		soundAPIavailableLabel->setText("Supported");
	} else {
		soundAPIavailableLabel->setText("NOT Supported");
	}
}

void
AppWindow::pixelsPerSampleChanged( int  amount ) {
	activeWavePanel->setPixelsPerSample( (unsigned)amount );
}

void
AppWindow::pixelsPerFullSampleChanged( int  amount ) {
	fullBellPanel->setPixelsPerSample( (unsigned) amount );
}

void
AppWindow::durationChanged( bool  changeAll ) {
	const double  dt = durationSpinBox->value();

	if ( ! changeAll ) {
		bellMaker.getWave( activeWaveIndex ).setDuration(dt);
		activeWavePanel->setDuration(dt);
		return;
	}

	size_t  waveCount = bellMaker.getWaveCount();
	size_t  w = 0;
	for (; w < waveCount; ++w ) {
		bellMaker.getWave(w).setDuration(dt);
		waveRenderPanels[w].get().setDuration(dt);
	}
	changeMade();
}

void
AppWindow::durationActivelyChanged( double  amount ) {
	size_t  waveCount = bellMaker.getWaveCount();
	size_t  w = 0;
	for (; w < waveCount; ++w ) {
		bellMaker.getWave(w).setDuration(amount);
		waveRenderPanels[w].get().setDuration(amount);
	}
	changeMade();
}

ding::BaseWaveform::Value
AppWindow::getSelectedWaveform() {
	// Add one to skip "flat"
	return ding::getWaveformFromIndex( waveformComboBox->currentIndex() + 1 );
}

ding::BaseWaveform::Value
AppWindow::getActiveWaveForm() {
	return waveRenderPanels[ activeWaveIndex ].getBaseForm();
}

void
AppWindow::resetActiveIndex( size_t  index ) {
	if ( index >= waveRenderPanels.size() ) {
		index = 0;
	}
	activeWaveIndex = index;
	activeWavePanel = &(waveRenderPanels[ activeWaveIndex ].get());
	showActiveWavePanel();
}

void
AppWindow::initActiveWavePanel() {
	//if ( activeWavePanel ) {
		// Disable currently active wave panel
		//hideActiveWavePanel();
	//}
	activeWavePanel = &(waveRenderPanels[ activeWaveIndex ].get());
	showActiveWavePanel();
	hideInactiveWavePanels();
}

void
AppWindow::showActiveWavePanel() {
	if ( activeWavePanel == nullptr ) {
		reportNoActiveWavePanel();
		return;
	}

	activeWavePanel->setEnabled(true);
	activeWavePanel->setHighlighted(true);
	activeWavePanel->show();
	activeWavePanel->repaint(); // Repaint with highlighting
	activeWavePanel->raise();
	reloadWaveControls();
}
/*
void
AppWindow::hideActiveWavePanel() {
	if ( activeWavePanel == nullptr ) {
		reportNoActiveWavePanel();
		return;
	}

	activeWavePanel->setEnabled(false);
	activeWavePanel->setHighlighted(false);
	activeWavePanel->repaint(); // Repaint without highlighting
	//activeWavePanel->hide();
}
*/
void
AppWindow::hideInactiveWavePanels() {
	WaveRenderPanel*  panel = nullptr;
	for ( WavePanelContainer&  wpc : waveRenderPanels ) {
		panel = &(wpc.get());
		if ( panel != activeWavePanel ) {
			panel->setEnabled(false);
			panel->setHighlighted(false);
			panel->repaint();
		}
	}
}

void
AppWindow::reloadWaveControls() {
	if ( activeWavePanel == nullptr )
		return;

	waveComboBox->setCurrentIndex(activeWaveIndex);
	waveformComboBox->setCurrentIndex( static_cast<int>(getActiveWaveForm()) - 1 );
	waveFrequencyBox->setValue( activeWavePanel->getFrequency() );
	wavePhaseBox->setValue( activeWavePanel->getPhaseShift() );
	smoothingCheckBox->setChecked( activeWavePanel->getVolumeSmoothingEnabled() );
	durationSpinBox->setValue( bellMaker.getMaxDuration() );
	pixelsPerSampleSpinBox->setValue( activeWavePanel->getPixelsPerSample() );
}

void
AppWindow::newProjectStart() {
	if ( maybeStay() )
		return;

	newProject();
}

void
AppWindow::newProject() {
	while ( bellMaker.getWaveCount() > 1 ) {
		waveRemoved();
	}
	waveRemoved(); // Reset the first one too
	resetActiveIndex();
	//reloadWaveControls();
	waveFrequencyBox->setValue( 720 );
	wavePhaseBox->setValue( 0 );
	waveFrequencyChanged( 720 );
	wavePhaseChanged( 0 );
	smoothingCheckBox->setChecked( false );
	durationActivelyChanged( 1 );
	durationSpinBox->setValue( 1 );
	//fullBellPanel->refresh(); // Done in waveRemoved(), but should it be done after all this too?
	unsavedProject = false;
	// FUTURE TODO: Clear the Undo list.
}

void
AppWindow::loadProjectStart() {
	QFileDialog  dialog{this};
	dialog.setWindowTitle(tr("Open File"));
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setNameFilter(tr("Bell Files (*.bell)"));

	if ( dialog.exec() != QDialog::Accepted )
		return;

	loadProject( dialog.selectedFiles().first() );
}

void
AppWindow::saveProjectStart() {
	saveProjectStep2();
}

bool
AppWindow::saveProjectStep2() {
	QFileDialog  dialog{this};
	dialog.setWindowTitle(tr("Save File"));
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setNameFilter(tr("Bell Files (*.bell)"));

	if ( dialog.exec() != QDialog::Accepted )
		return false;

	return saveProject( dialog.selectedFiles().first() );
}

void
AppWindow::exportSoundStart() {
	QFileDialog  dialog{this};
	dialog.setWindowTitle(tr("Save File"));
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setNameFilter(tr("Sound Files (*.wav)"));

	if ( dialog.exec() != QDialog::Accepted )
		return;

	writeAudioFile( dialog.selectedFiles().first() );
}

void
AppWindow::changeMade() {
	fullBellPanel->refresh();
	unsavedProject = true;
	// FUTURE TODO: Push current state to an undo list.
}

bool
AppWindow::maybeStay() {
	if ( ! unsavedProject )
		return false;

	const QMessageBox::StandardButton  result
		= QMessageBox::warning(this, tr("Leave Project?"),
				tr("This project has been modified. Do you wish to save your changes?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );

	switch ( result )
	{
	case QMessageBox::Save:
		return ! saveProjectStep2();

	case QMessageBox::Discard:
		return false;

	default: break;
	}
	return true;
}

void
AppWindow::setCurrentFile( const QString&  fileName ) {
	currentFile = fileName;

	QString  titleName = QFileInfo(currentFile).fileName();
	if ( currentFile.isEmpty() )
		titleName = tr("untitled");
	setWindowFilePath(titleName);
}

bool
AppWindow::loadProject( const QString&  fileName ) {
	QFile  file(fileName);

	if ( !file.open( QFile::ReadOnly | QFile::Text ) ) {
		QMessageBox::warning(this, tr("File Open"),
			tr("Failed to open file %1 for reading:\n%2.").arg(QDir::toNativeSeparators(fileName)).arg(file.errorString() )
		);
		return false;
	}

	newProject();

	//QTextStream  in(&file); // in.readAll()
	IOUnit  iounit;
	bool  success = false;
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
	success = iounit.load( file, bellMaker );
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif

	if ( ! success ) {
		QMessageBox::warning(this, tr("File Read"),
			tr("Failed to properly read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName)).arg(iounit.errorString() )
		);
		newProject();
		return false;
	}

	// It's easier if we wipe out all panels
	activeWaveIndex = 0;
	waveComboBox->removeItem(0);
	deleteActiveWavePanel();
	activeWavePanel = nullptr;
	//lastWaveId = 1;

	const size_t  waveCount = bellMaker.getWaveCount();
	size_t  w = 0;
	ding::Wave*  wave = nullptr;
	for (; w < waveCount; ++w) {
		wave = & bellMaker.getWave(w);
		if ( wave->Name.size() == 0 ) {
			createNewPanel( *wave, QString("%1").arg(w+1).toStdString().c_str() );
		} else {
			createNewPanel( *wave, wave->Name.c_str() );
		}
	}

	// fullBellPanel->refresh() may be called under reloadWaveControls()
	resetActiveIndex();
	reloadWaveControls();
	setCurrentFile(fileName);
	unsavedProject = false;
	return true;
}

bool
AppWindow::saveProject( const QString&  fileName ) {
	QFile  file(fileName);

	if ( !file.open( QFile::WriteOnly | QFile::Text ) ) {
		QMessageBox::warning(this, tr("File Save"),
			tr("Failed to open file %1 for writing:\n%2.").arg( QDir::toNativeSeparators(fileName), file.errorString() )
		);

		return false;
	}

	QTextStream  out(&file);
	IOUnit  iounit;
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
	out << iounit.save( bellMaker );
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif

	setCurrentFile(fileName);
	unsavedProject = false;
	return true;
}

bool
AppWindow::writeAudioFile( const QString&  fileName ) {
	QFile  file(fileName);

	if ( !file.open( QFile::WriteOnly | QFile::Text ) ) {
		QMessageBox::warning(this, tr("Audio File Save"),
			tr("Failed to open file %1:\n%2.").arg( QDir::toNativeSeparators(fileName), file.errorString() )
		);

		return false;
	}

	AudioFileWriter  writer;
	bool  success;
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
	bellMaker.loadIntoContainer< AudioFileWriter::Buffer_t >( writer.getBuffer(), 0 );
	writer.setSampleRate( (quint32) bellMaker.getSamplesPerSecond() );
	success = writer.write( file );
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif

	if ( !success ) {
		QMessageBox::warning(this, tr("Audio File Save"),
			tr("Failed to write audio file %1:\n%2").arg( QDir::toNativeSeparators(fileName), writer.errorString())
		);
	}

	return success;
}

void
AppWindow::reportNoActiveWavePanel() {
	//std::printf("Active wave panel is null when needed.\n");
}

#ifdef DEBUG_SOUND
void
AppWindow::fillBuffer(soundbox::SampleBuffer&  buffer, unsigned char channel) {
	const size_t  size = 44100*2;
	size_t i;
	buffer.clear();
	buffer.reserve(size);
	// Write interleaved audio data.
	for ( i=0; i < size; i++ ) {
		const double  point = sin(lastValues[channel] * 3.141592653588);
		buffer.push_back( point );

		lastValues[channel] += 0.005 * (channel + 1 + (channel*0.1));
		if ( lastValues[channel] >= 1.0 ) lastValues[channel] -= 2.0;
	}
}
#endif
