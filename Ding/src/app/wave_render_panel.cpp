// (c) 2019 Nicolaus Anderson

#include "wave_render_panel.h"
#include "../ding/ding_volume_span.h"
#include <QPainter>
#include <QPaintEvent>

#include <cstdio>

WaveRenderPanel::WaveRenderPanel(QWidget*  parent)
	: QWidget(parent)
	, HighlightColor(Qt::red)
	, NormalColor(Qt::black)
	, HandleHighlightColor(Qt::blue)
	, HandleNormalColor(Qt::black)
	, HandleSize(10)
	, HideHandlesWhenDisabled(false)
	, enabled(false)
	, highlighted(false)
	, leftMouseButtonDown(false)
	, lastMousePosition(0,0)
	, handles()
	, selectedHandle(0)
	, hasSelectedHandle(false)
	, wave()
	, dirty(true)
	, points()
	, pen()
	, brush()
	, pixelsPerSample(4)
	, percentWidgetHeightFullScale(0.95)
	, listener(nullptr)
{
	setHighlighted(false);
	// We want to see ALL of the waves at the same time, so no background color is set.
	pen.setWidth(2);
}

WaveRenderPanel::~WaveRenderPanel()
{}

void
WaveRenderPanel::setEnabled(bool  setting) {
	enabled = setting;
}

bool
WaveRenderPanel::isEnabled() const {
	return enabled;
}

void
WaveRenderPanel::setHighlighted( bool  setting ) {
	highlighted = setting;
	if ( highlighted ) {
		brush = QBrush(HighlightColor);
		pen.setColor(HighlightColor);
		handlePen.setColor(HandleHighlightColor);
	}
	else {
		brush = QBrush(NormalColor);
		pen.setColor(NormalColor);
		handlePen.setColor(HandleNormalColor);
	}
}

bool
WaveRenderPanel::isHighlighted() const {
	return highlighted;
}

void
WaveRenderPanel::setDuration( play_t  time ) {
	wave.setDuration(time);
	dirty = true;
	refitHandles();
	//refreshPoints(); // Called in paintEvent
	repaint();
}

play_t
WaveRenderPanel::getDuration() const {
	return wave.getDuration();
}

void
WaveRenderPanel::setFrequency( play_t  freq ) {
	wave.setFrequency(freq);
	dirty = true;
	//refreshPoints(); // Called in paintEvent
	repaint();
}

play_t
WaveRenderPanel::getFrequency() const {
	return wave.getFrequency();
}

void
WaveRenderPanel::setPhaseShift( play_t  phase ) {
	wave.setPhaseShift(phase);
	dirty = true;
	//refreshPoints(); // Called in paintEvent
	repaint();
}

play_t
WaveRenderPanel::getPhaseShift() const {
	return wave.getPhaseShift();
}

ding::Wave&
WaveRenderPanel::getWave() {
	return wave;
}

void
WaveRenderPanel::setWave( ding::Wave  w ) {
	wave = w;
	dirty = true;
	refitHandles();
	//refreshPoints(); // Called in paintEvent
	repaint();
}

void
WaveRenderPanel::setHandleCount( size_t  numHandles ) {
	if ( numHandles < handles.size() ) {
		// Zero out volume settings beyond the last index
		zeroVolumeSettingsAfter( numHandles - 1 );
	}
	handles.resize(numHandles);
	reconfigureHandles();
	repaint();
}

size_t
WaveRenderPanel::getHandleCount() const {
	return handles.size();
}

void
WaveRenderPanel::setPixelsPerSample( unsigned  pps ) {
	pixelsPerSample = (pps >= 1 ? pps : 1);
	dirty = true;
	//refreshPoints(); // Called in paintEvent
	repaint();
}

unsigned
WaveRenderPanel::getPixelsPerSample() const {
	return pixelsPerSample;
}

void
WaveRenderPanel::setVolumeSmoothingEnabled( bool  setting ) {
	std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	if ( ! volumeMultiplier.get() )
		return;

	volumeMultiplier.get()->SmoothingEnabled = setting;
}

bool
WaveRenderPanel::getVolumeSmoothingEnabled() const {
	std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	if ( ! volumeMultiplier.get() )
		return false;

	return volumeMultiplier.get()->SmoothingEnabled;
}

play_t
WaveRenderPanel::getTimeFromX( int  x ) const {
	return (wave.getDuration() * x) / QWidget::size().width();
}

volume_t
WaveRenderPanel::getVolumeFromY( int  y ) const {
	const int  height = QWidget::size().height();
	if ( height == 0 )
		return 1;

	return static_cast<volume_t>(height - y) / height; // Same as 1 - y/height
}

int
WaveRenderPanel::getXFromTime( play_t  t ) const {
	if ( wave.getDuration() == 0 )
		return QWidget::size().width();

	return static_cast<int>( t * QWidget::size().width() / wave.getDuration() );
}

int
WaveRenderPanel::getYFromVolume( volume_t  v ) const {
	// Subtract from 1 because the Y-axis is inverted
	return static_cast<int>( (1.0 - v) * QWidget::size().height() );
}

play_t
WaveRenderPanel::getHandleTime( size_t  index ) const {
	if ( index >= handles.size() )
		return wave.getDuration();

	return getTimeFromX( handles[index].x() );
}

volume_t
WaveRenderPanel::getHandleVolume( size_t  index ) const {
	if ( index >= handles.size() )
		return 0;

	return static_cast<volume_t>(1) - volume_t(handles[index].y()) / volume_t(QWidget::size().height());
}

void
WaveRenderPanel::setEndcapHandlesToZero() {
	if ( handles.size() == 0 )
		return;

	const int y = getYFromVolume(0);

	handle_type&  first = handles[0];
	first.setY( y );
	setWaveVolume(0, getTimeFromX(first.x()), 0);

	if ( handles.size() == 1 )
		return;

	const size_t  finalIndex = handles.size()-1;
	handle_type&  last = handles[ finalIndex ];
	last.setY( y );
	setWaveVolume(finalIndex, getTimeFromX(last.x()), 0);

	repaint();
}

void
WaveRenderPanel::setHandlesToVolume( volume_t  v ) {
	handle_type*  handle;
	size_t  h = 0;
	for (; h < handles.size(); ++h) {
		handle = &(handles[h]);
		handle->setY( getYFromVolume(v) );
		setWaveVolume(h, getTimeFromX(handle->x()), getVolumeFromY(handle->y()));
	}

	repaint();
}

void
WaveRenderPanel::mousePressEvent(QMouseEvent*  event) {
	if ( event->button() == Qt::LeftButton ) {
		leftMouseButtonDown = true;
		lastMousePosition = event->pos();
		grabNearbyHandle();
	}
}

void
WaveRenderPanel::mouseReleaseEvent(QMouseEvent*  event) {
	if ( event->button() == Qt::LeftButton ) {
		leftMouseButtonDown = false;
		selectedHandle = 0;
		hasSelectedHandle = false;
	}
}

void
WaveRenderPanel::mouseMoveEvent(QMouseEvent*  event) {
	auto mousePosition = event->pos();
	moveActiveHandle( mousePosition.x(), mousePosition.y() );
	lastMousePosition = mousePosition;
}

void
WaveRenderPanel::resizeEvent(QResizeEvent*  event) {
	QWidget::resizeEvent(event);
	dirty = true;
	refitHandles();
	//refreshPoints(); // Called in paintEvent
}

void
WaveRenderPanel::setListener( Listener*  h ) {
	listener = h;
}

void
WaveRenderPanel::grabNearbyHandle() {
	size_t h = 0;
	selectedHandle = 0;
	hasSelectedHandle = false;
	for (; h < handles.size(); ++h) {
		if ( isPointAtHandle(h, lastMousePosition) ) {
			selectedHandle = h;
			hasSelectedHandle = true;
			break;
		}
	}
}

void
WaveRenderPanel::moveActiveHandle(int dx, int dy) {
	if ( ! hasSelectedHandle )
		return;

	play_t  dt = 0;
	volume_t  dv = 0;
	int left = 0;
	int right = QWidget::size().width();

	if ( selectedHandle == 0 ) {
		shiftHandleY( selectedHandle, dy );
	} else if ( selectedHandle == handles.size() - 1 ) {
		left = handles[ handles.size() - 2 ].x();
		moveHandleBounded( selectedHandle, dx, dy, left, right );
	} else {
		left = handles[selectedHandle-1].x();
		right = handles[selectedHandle+1].x();
		moveHandleBounded( selectedHandle, dx, dy, left, right );
	}
	dt = getHandleTime( selectedHandle );
	dv = getHandleVolume( selectedHandle );
	setWaveVolume( selectedHandle, dt, dv );
	repaint();
}

void
WaveRenderPanel::resetHandles() {
	// TODO: Double-check before using.

	size_t  h = 0;
	if ( ! hasVolumeSource() ) {
		for (; h < handles.size(); ++h )
			setHandle(h, 0, QWidget::size().height()); // Note: Y-axis is flipped
		return;
	}

	std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	volumeMultiplier.get()->clearSettings();

	const QSize  widgetSize = QWidget::size(); // Size of this widget
	const ding::play_t  dt = xzoom * wave.getDuration() / widgetSize.width();
	ding::Volume*  volume;

	// Cannot stretch if only one handle exists
	if ( handles.size() == 1 ) {
		setHandle(0, 0, widgetSize.height());
		volume = &( volumeMultiplier.get()->getSetting(0) );
		volume->time = 0;
		volume->volume = 1;
		return;
	}

	for (; h < handles.size(); ++h ) {
		setHandle(
			h,
			(widgetSize.width() * h) / (handles.size() - 1),
			0 // Not the widget height because our axis are flipped
		);
		volume = &( volumeMultiplier.get()->getSetting(h) );
		volume->time = h * dt;
		volume->volume = 1;
	}
}

void
WaveRenderPanel::reconfigureHandles() {
	// Set positions of both handles and volumes based on the number of handles and the calculated volume.
	// Handles are spaced out.

	size_t  h = 0;
	if ( ! hasVolumeSource() ) {
		for (; h < handles.size(); ++h )
			setHandle(h, 0, 0);
		return;
	}

	std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	ding::VolumeSpan&  volumes = *(volumeMultiplier.get());
	ding::Volume*  vsetting;

	// Cannot stretch if only one handle exists
	if ( handles.size() == 1 ) {
		vsetting = & volumes.getSetting(0);
		setHandle(0, 0, getYFromVolume( vsetting->volume ) );
		vsetting->time = 0;
		return;
	}

	const QSize  widgetSize = QWidget::size(); // Size of this widget
	const ding::play_t  dt = xzoom * wave.getDuration() / widgetSize.width();
	ding::play_t  currTime = 0;

	for (; h < handles.size(); ++h, currTime += dt) {
		const int  x = (widgetSize.width() * h) / (handles.size() - 1);
		const int  y = getYFromVolume( volumes.getSample(currTime) );
		setHandle( h, x, y );
	}

	// Now reset the volumes.
	// NOTE: This creates a curve that is approximated from the previous one.
	for (h = 0; h < handles.size(); ++h ) {
		vsetting = & volumes.getSetting(h);
		vsetting->time = getHandleTime(h);
		vsetting->volume = getHandleVolume(h);
	}

	// NOTE: handles.size() has 1 subtracted so that handles stretch from 0 to width (or duration)
	// because when h == handles.size() - 1, h / (handles.size() - 1) == 1
}

void
WaveRenderPanel::refitHandles() {
	// Set the x position of all handles based on the window size.
	// Used after changing the size of the widget.

	size_t  h = 0;
	if ( ! hasVolumeSource() ) {
		for (; h < handles.size(); ++h )
			setHandle(h, 0, 0);
		return;
	}

	std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	ding::VolumeSpan&  volumes = *(volumeMultiplier.get());
	for (; h < handles.size(); ++h ) {
		setHandle( h, getXFromTime( volumes.getSetting(h).time ), getYFromVolume( volumes.getSetting(h).volume ) );
	}
}

void
WaveRenderPanel::refreshPoints() {
	if ( !dirty || ! hasSampleSource() )
		return;

	points.clear();

	const QSize  widgetSize = QWidget::size(); // Size of this widget
	int  dx = 0;
	for (; dx < widgetSize.width(); dx += pixelsPerSample) {
		const ding::volume_t  waveHeight =
			- getSample( xzoom * getTimeFromX(dx) / 100 ) * yzoom * percentWidgetHeightFullScale * widgetSize.height() / 2;

		points.push_back( QPoint(
			dx,
			waveHeight + widgetSize.height()/2
				+ (pen.width() / 2) // Account also for pet width
		));
	}

	dirty = false;
}

bool
WaveRenderPanel::hasSampleSource() const {
	return wave.hasSampleSource();
}

bool
WaveRenderPanel::hasVolumeSource() const {
	return wave.hasVolumeMultiplier();
}

double
WaveRenderPanel::getSample( play_t  time ) {
	return wave.getReadiedSample(time);

	//if ( hasSampleSource() )
	//	return wave.getSampleSource().get()->getSample(time); // Does not account for frequency and phase
	//return 0;
}

double
WaveRenderPanel::getVolume( play_t  time ) {
	const std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	if ( volumeMultiplier.get() )
		return volumeMultiplier.get()->getSample(time);
	return 0;
}

void
WaveRenderPanel::setWaveVolume( index_t  handleIndex, play_t  time, volume_t  volume ) {
	 if ( ! isEnabled() )
		return;
	std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	if ( ! volumeMultiplier.get() )
		return;

	ding::Volume&  v = volumeMultiplier.get()->getSetting(handleIndex);
	v.time = time;
	v.volume = volume;

	if ( listener ) {
		listener->OnVolumeHandleChanged(handleIndex, time, volume);
	}
}

void
WaveRenderPanel::zeroVolumeSettingsAfter( index_t  i ) {
	if ( ! hasVolumeSource() )
		return;

	std::shared_ptr<ding::VolumeSpan>  volumeMultiplier = wave.getVolumeMultiplier();
	ding::VolumeSpan&  volumes = *(volumeMultiplier.get());

	++i;
	for (; i < volumes.getSettingCount(); ++i)
		volumes.getSetting(i).volume = 0;
}

bool
WaveRenderPanel::isPointAtHandle(size_t  index, const QPoint  point) {
	QPoint  d = handles[index] - point;
	if ( d.x() < 0 ) d.setX( -d.x() );
	if ( d.y() < 0 ) d.setY( -d.y() );
	return static_cast<unsigned>(d.x()) <= HandleSize
		&& static_cast<unsigned>(d.y()) <= HandleSize;
}

void
WaveRenderPanel::setHandle(size_t  index, int  x, int  y) {
	handles[index] = QPoint(x,y);
}

void
WaveRenderPanel::shiftHandleY(size_t  index, int  dy) {
	QPoint&  p = handles[index];
	p.setY( getWidgetBoundY(dy) );
}

void
WaveRenderPanel::moveHandleBounded(size_t  index, int dx, int dy, int  boundLeft, int  boundRight) {
	QPoint&  p = handles[index];
	p.setX( dx < boundLeft ? boundLeft : (dx > boundRight? boundRight : dx) );
	p.setY( getWidgetBoundY(dy) );
}

int
WaveRenderPanel::getWidgetBoundY( int  y ) {
	const int  h = QWidget::size().height();
	return ( y < 0 ? 0 : ( y > h ? h : y ) );
}

void
WaveRenderPanel::paintEvent(QPaintEvent*) {
	refreshPoints();

	if ( points.size() == 0 ) return;

	QPainter  painter(this);

	// Draw the music line --------------------------------
	QPen  musicLinePen;
	musicLinePen.setColor(Qt::gray);
	painter.setPen(musicLinePen);

	const int  widgetWidth = QWidget::size().width();
	const int  halfWidgetHeight = QWidget::size().height() / 2;
	const int  musicLineBorder = static_cast<int>( percentWidgetHeightFullScale * halfWidgetHeight );
	// Top
	painter.drawLine(
		0, halfWidgetHeight - musicLineBorder,
		widgetWidth, halfWidgetHeight - musicLineBorder
	);
	// Zero
	painter.drawLine(
		0, halfWidgetHeight,
		widgetWidth, halfWidgetHeight
	);
	// Bottom
	painter.drawLine(
		0, halfWidgetHeight + musicLineBorder,
		widgetWidth, halfWidgetHeight + musicLineBorder
	);

	// Draw the actual wave --------------------------------
	painter.setPen(pen);
	painter.setBrush(brush);
	painter.setRenderHint(QPainter::Antialiasing, true);

	// Draw actual points
	painter.drawPolyline( points.data(), points.size() );

	// Draw handles if permitted
	if ( !isEnabled() && HideHandlesWhenDisabled ) return;

	painter.setPen(handlePen);

	painter.setRenderHint(QPainter::Antialiasing, false);
	for ( const QPoint&  h : handles ) {
		// Paint the handle
		const QRect  area(h.x() - (int)HandleSize/2, h.y() - (int)HandleSize/2, HandleSize, HandleSize);
		painter.drawRect(area);
	}

	// Draw lines between handles
	painter.drawPolyline( const_cast<const QPoint*>(handles.data()), handles.size() );
}
