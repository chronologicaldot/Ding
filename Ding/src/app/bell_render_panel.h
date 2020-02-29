// (C) 2019 Nicolaus Anderson

#include "../ding/ding_bell_maker.h"
#include <vector>
#include <QPen>
#include <QWidget>

using ding::play_t;
using ding::volume_t;

class BellRenderPanel
	: public QWidget
{
	Q_OBJECT

	typedef  std::vector<QPoint>  point_list_t;
	typedef decltype(Qt::green) Color;

public:

	BellRenderPanel( QWidget*  parent = 0 );

	Color  RenderColor;
	bool  ConstantRefresh;

	void  setBellMaker( ding::BellMaker& );
	void  setPixelsPerSample( unsigned );
	unsigned  getPixelsPerSample() const;
	void  setVisibleDuration( play_t );
	play_t  getVisibleDuration() const;
	void  refresh();
	void  paintEvent(QPaintEvent*) override;

protected:
	void  refreshPoints();
	volume_t  getSample( play_t );
	play_t  getTimeFromX( int ) const;

private:
	// Members ----------------------------
	ding::BellMaker*  bellMaker;
	unsigned  pixelsPerSample;
	play_t  visibleDuration;
	point_list_t  points;
	bool  dirty;
	QPen  pen;
	double  percentWidgetHeightFullScale;
};
