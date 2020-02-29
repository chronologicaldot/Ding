// (c) 2019 Nicolaus Anderson

#ifndef WAVE_RENDER_PANEL_H
#define WAVE_RENDER_PANEL_H

#include <QWidget>
#include <QPen>
#include <QMouseEvent>
#include <QResizeEvent>
#include "../ding/ding_sample_source.h"
#include "../ding/ding_wave.h"
#include <memory>

using ding::index_t;
using ding::play_t;
using ding::volume_t;

//! Wave Render Panel
/*
	The purpose of this panel is merely to draw a Wave and its volume modifiers.
	It can modify the VolumeSpan by means of handles.
*/
class WaveRenderPanel
	: public QWidget
{
	Q_OBJECT

	typedef decltype(Qt::green) Color;
	typedef QPoint  handle_type;

public:

	struct Listener {
		//! Called when volume is changed by the given handle and set to the given time and volume
		virtual void  OnVolumeHandleChanged( index_t, play_t, volume_t )=0;
	};

	WaveRenderPanel(QWidget*  parent);
	~WaveRenderPanel();

	Color  HighlightColor;
	Color  NormalColor;
	Color  HandleHighlightColor;
	Color  HandleNormalColor;
	unsigned  HandleSize;
	bool  HideHandlesWhenDisabled;

	void  setEnabled(bool);
	bool  isEnabled() const;
	void  setHighlighted(bool);
	bool  isHighlighted() const;
	void  setDuration( play_t );
	play_t  getDuration() const;
	void  setFrequency( play_t );
	play_t  getFrequency() const;
	void  setPhaseShift( play_t );
	play_t  getPhaseShift() const;
	ding::Wave&  getWave();
	void  setWave( ding::Wave ); // Also acts as a reset for internal cache and handles
	void  setHandleCount( size_t );
	size_t  getHandleCount() const;
	void  setPixelsPerSample( unsigned );
	unsigned  getPixelsPerSample() const;
	void  setVolumeSmoothingEnabled( bool );
	bool  getVolumeSmoothingEnabled() const;
	play_t  getHandleTime( size_t ) const;
	volume_t  getHandleVolume( size_t ) const;
	void  setEndcapHandlesToZero();
	void  setHandlesToVolume( volume_t );
	void  mousePressEvent(QMouseEvent*);
	void  mouseReleaseEvent(QMouseEvent*);
	void  mouseMoveEvent(QMouseEvent*);
	void  resizeEvent(QResizeEvent*);
	void  setListener( Listener* );

protected:
	play_t  getTimeFromX( int ) const;
	volume_t  getVolumeFromY( int ) const;
	int  getXFromTime( play_t ) const;
	int  getYFromVolume( volume_t ) const;
	void  grabNearbyHandle();
	void  moveActiveHandle(int, int);
	void  resetHandles();
	void  reconfigureHandles();
	void  refitHandles();
	void  refreshPoints();
	bool  hasSampleSource() const;
	bool  hasVolumeSource() const;
	double  getSample( play_t );
	double  getVolume( play_t );
	void  setWaveVolume( index_t, play_t, volume_t );
	void  zeroVolumeSettingsAfter( index_t );
	bool  isPointAtHandle(size_t, const QPoint);
	void  setHandle(size_t, int, int);
	void  shiftHandleY(size_t, int);
	void  moveHandleBounded(size_t, int, int, int, int);
	int  getWidgetBoundY(int);
    void  paintEvent(QPaintEvent*) override;

	// Members --------------------
	bool  enabled;
	bool  highlighted;
	bool  leftMouseButtonDown;
	QPoint  lastMousePosition;
	std::vector<handle_type>  handles;
	size_t  selectedHandle;
	bool hasSelectedHandle;
	mutable ding::Wave  wave; // Mutable so it can be used in getVolumeSmoothingEnabled()
	bool dirty;
	std::vector<QPoint>  points;
	QPen  pen;
	QBrush  brush;
	QPen  handlePen;
	unsigned  pixelsPerSample;
	double  percentWidgetHeightFullScale;
	Listener*  listener;

	// For the time being, these are constant.
	static constexpr play_t  xzoom = 1; // For now
	static constexpr play_t  yzoom = 1; // For now
};

#endif // WAVE_RENDER_PANEL_H
