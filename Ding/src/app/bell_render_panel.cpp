// (c) 2019 Nicolaus Anderson

#include "bell_render_panel.h"
#include <QPainter>

BellRenderPanel::BellRenderPanel( QWidget*  parent )
	: QWidget(parent)
	, RenderColor(Qt::blue)
	, ConstantRefresh(false)
	, bellMaker(nullptr)
	, pixelsPerSample(4)
	, visibleDuration(1)
	, points()
	, dirty(true)
	, pen()
	, percentWidgetHeightFullScale(0.95)
{}

void
BellRenderPanel::setBellMaker( ding::BellMaker&  b ) {
	bellMaker = &b;
	refresh();
}

void
BellRenderPanel::setPixelsPerSample( unsigned  pps ) {
	pixelsPerSample = pps;
	refresh();
}

unsigned
BellRenderPanel::getPixelsPerSample() const {
	return pixelsPerSample;
}

void
BellRenderPanel::setVisibleDuration( play_t  time ) {
	visibleDuration = time;
	refresh();
}

play_t
BellRenderPanel::getVisibleDuration() const {
	return visibleDuration;
}

void
BellRenderPanel::refresh() {
	dirty = true;
	//refreshPoints();
	repaint();
}

void
BellRenderPanel::paintEvent(QPaintEvent*) {
	refreshPoints();

	QPainter  painter(this);

	// Clip Border --------------------------
	QPen  capLinePen;
	capLinePen.setColor(Qt::gray);
	painter.setPen(capLinePen);

	const int  widgetWidth = QWidget::size().width();
	const int  halfWidgetHeight = QWidget::size().height() / 2;
	const int  musicLineBorder = static_cast<int>( percentWidgetHeightFullScale * halfWidgetHeight );
	// Top
	painter.drawLine(
		0, halfWidgetHeight - musicLineBorder,
		widgetWidth, halfWidgetHeight - musicLineBorder
	);
	// Bottom
	painter.drawLine(
		0, halfWidgetHeight + musicLineBorder,
		widgetWidth, halfWidgetHeight + musicLineBorder
	);

	// Data Points -------------------------
	pen.setColor(RenderColor);
	painter.setPen(pen);

	painter.drawPolyline( points.data(), points.size() );
}

void
BellRenderPanel::refreshPoints() {
	if ( ConstantRefresh ) {
		if ( bellMaker )
			visibleDuration = bellMaker->getMaxDuration();
	} else {
		if ( !dirty )
			return;
	}

	points.clear();

	const QSize  widgetSize = QWidget::size(); // Size of this widget
	int  dx = 0;
	for (; dx < widgetSize.width(); dx += pixelsPerSample) {
		const ding::volume_t  waveHeight =
			- getSample( getTimeFromX(dx) ) * percentWidgetHeightFullScale * widgetSize.height() / 2;

		points.push_back( QPoint(
			dx,
			waveHeight + widgetSize.height()/2
				+ (pen.width() / 2) // Account also for pet width
		));
	}
	
	dirty = false;
}

volume_t
BellRenderPanel::getSample( play_t  time ) {
	if ( !bellMaker )
		return 0;

	return bellMaker->fetchCompiledSample(time);
}

play_t
BellRenderPanel::getTimeFromX( int  x ) const {
	return (visibleDuration * x) / QWidget::size().width();
}
