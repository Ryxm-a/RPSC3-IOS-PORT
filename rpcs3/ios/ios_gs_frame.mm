#import <UIKit/UIKit.h>
#include "ios_gs_frame.h"

static UIView* as_view(void* v) { return (__bridge UIView*)v; }

int ios_gs_frame::client_width()
{
	UIView* v = as_view(m_view);
	return static_cast<int>(v.bounds.size.width * v.contentScaleFactor);
}

int ios_gs_frame::client_height()
{
	UIView* v = as_view(m_view);
	return static_cast<int>(v.bounds.size.height * v.contentScaleFactor);
}

f64 ios_gs_frame::client_display_rate()
{
	return static_cast<f64>(UIScreen.mainScreen.maximumFramesPerSecond);
}
