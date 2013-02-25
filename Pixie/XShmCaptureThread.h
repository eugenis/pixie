#pragma once
#include "CaptureThread.h"
#include <qdesktopwidget.h>
#include "Regions.h"

#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>


class CXShmCaptureThread : public CCaptureThread {
public:
	CXShmCaptureThread( QWidget *parent, bool *stopThread, CSettings *settings, bool *readyToProcess, const int d );
	~CXShmCaptureThread();
private:
	virtual void init();
	virtual void run();
	virtual void cleanup();

	Display *dpy;
	int screen;
	Visual *visual;
	int width;
	int height;
	Window rootWindow;
	int depth;
	XShmSegmentInfo shminfo;
	XImage *image;

	SparseRegions *regions;
};

