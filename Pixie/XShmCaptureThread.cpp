#include "stdafx.h"
#include "XShmCaptureThread.h"

#include <assert.h>
#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>


CXShmCaptureThread::CXShmCaptureThread( QWidget *parent, bool *stopThread, CSettings *settings, bool *readyToProcess, const int d ) 
	: CCaptureThread( parent, stopThread, settings, readyToProcess ) {
}

CXShmCaptureThread::~CXShmCaptureThread() {
}

void CXShmCaptureThread::init() {
    dpy = XOpenDisplay(NULL);
    screen = DefaultScreen(dpy);
    visual = DefaultVisual(dpy, screen);
    width = DisplayWidth(dpy, screen);
    height = DisplayHeight(dpy, screen);
    rootWindow = RootWindow(dpy, screen);
    depth = DefaultDepth(dpy, screen);

    regions = new SparseRegions( settings->getHorizontalSegmentWidth( width ), settings->getHorizontalHeight( height ),
				 settings->getVerticalWidth( width ), settings->getVerticalSegmentHeight( height ) );
    regions->regionHTop.resize(settings->LEDnumH);
    regions->regionHBottom.resize(settings->LEDnumH);
    regions->regionVLeft.resize(settings->LEDnumV);
    regions->regionVRight.resize(settings->LEDnumV);
    
    printf("%d %d %d %d\n", regions->hWidth, regions->hHeight, regions->vWidth, regions->vHeight);

    // HTop
    images[0] = XShmCreateImage(dpy, visual, depth, ZPixmap, NULL, &shminfos[0], width, regions->hHeight);
    // HBottom
    images[1] = XShmCreateImage(dpy, visual, depth, ZPixmap, NULL, &shminfos[1], width, regions->hHeight);
    // VLeft
    images[2] = XShmCreateImage(dpy, visual, depth, ZPixmap, NULL, &shminfos[2], regions->hWidth, height);
    // VRight
    images[3] = XShmCreateImage(dpy, visual, depth, ZPixmap, NULL, &shminfos[3], regions->hWidth, height);

    for (int i = 0; i < 4; ++i) {
      XImage *&image = images[i];
      XShmSegmentInfo &shminfo = shminfos[i];
      shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);
      assert(shminfo.shmid != -1);
      shminfo.shmaddr = image->data = (char*)shmat(shminfo.shmid, NULL, 0);
      shminfo.readOnly = False;
      int res = XShmAttach(dpy, &shminfo);
      assert(res);
    }

}

void save(XImage &image, const char *fname) {
  FILE* fp = fopen(fname, "w");
  fprintf(fp, "P3\n");
  fprintf(fp, "%d %d\n", image.width, image.height);
  fprintf(fp, "%d\n", 255);
  char *p = image.data;
  for (int y = 0; y < image.height; ++y) {
    for (int x = 0; x < image.width; ++x) {
      fprintf(fp, "%hhu %hhu %hhu ", p[x*4 + 2], p[x*4 + 1], p[x*4 + 0]);
    }
    fprintf(fp, "\n");
    p += image.bytes_per_line;
  }
  fclose(fp);
}

void CXShmCaptureThread::run() {
	emit setIsRunning( true );
	init();

	delayTime = 10;	// initial delay time
	frameCounter = 0;
	frameTimer.start();
	
	while( !*stopThread ) {
		if ( *readyToProcess == true ) {			

		  bool res;
		  // HTop
		  res = XShmGetImage(dpy, rootWindow, images[0], 0, 0, AllPlanes);
		  assert(res);

		  // HBottom
		  res = XShmGetImage(dpy, rootWindow, images[1], 0, height - regions->hHeight, AllPlanes);
		  assert(res);

		  // VLeft
		  res = XShmGetImage(dpy, rootWindow, images[2], 0, 0, AllPlanes);
		  assert(res);

		  // VRight
		  res = XShmGetImage(dpy, rootWindow, images[3], width - regions->vWidth, 0, AllPlanes);
		  assert(res);


#define IMGPTR(img, x, y) (unsigned char*)((img)->data + (y) * (img)->bytes_per_line + (x) * 4)

			// Horizontals
			// Top
			for ( unsigned short x = 0; x < settings->LEDnumH; x++ ) {
				// QImage im = scr->grabWindow( desktopID, x * regions->hWidth, 0, regions->hWidth, regions->hHeight ).toImage();
				// unsigned char * bufH_tmp = new unsigned char[ bufHSize ];
				// memcpy( bufH_tmp, im.bits(), bufHSize );

			  regions->regionHTop[x] = SparseRegion(IMGPTR(images[0], x * regions->hWidth, 0), regions->hWidth, regions->hHeight, images[0]->bytes_per_line);
			}
			
			// Bottom
			for ( unsigned short x = 0; x < settings->LEDnumH; x++ ) {
				// QImage im = scr->grabWindow( desktopID, x * regions->hWidth, ScreenHeight - regions->hHeight, regions->hWidth, regions->hHeight ).toImage();
				// unsigned char * bufH_tmp = new unsigned char[ bufHSize ];
				// memcpy( bufH_tmp, im.bits(), bufHSize );

				regions->regionHBottom[x] = SparseRegion(IMGPTR(images[1], x * regions->hWidth, 0), regions->hWidth, regions->hHeight, images[1]->bytes_per_line);

				// regions->regionHBottom.push_back( bufH_tmp );			
			}

			// Verticals
			// Left
			for ( int x = 0; x < settings->LEDnumV; x++ ) {
				// QImage im = scr->grabWindow( desktopID, 0, x * regions->vHeight, regions->vWidth, regions->vHeight ).toImage();
				// unsigned char * bufV_tmp = new unsigned char[ bufVSize ];
				// memcpy( bufV_tmp, im.bits(), bufVSize );

				regions->regionVLeft[x] = SparseRegion(IMGPTR(images[2], 0, x * regions->vHeight), regions->vWidth, regions->vHeight, images[2]->bytes_per_line);

				// regions->regionVLeft.push_back( bufV_tmp );
			}
			
			// Right
			for ( int x = 0; x < settings->LEDnumV; x++ ) {
				// QImage im = scr->grabWindow( desktopID, ScreenWidth - regions->vWidth, x * regions->vHeight, regions->vWidth, regions->vHeight ).toImage();
				// unsigned char * bufV_tmp = new unsigned char[ bufVSize ];
				// memcpy( bufV_tmp, im.bits(), bufVSize );

				regions->regionVRight[x] = SparseRegion(IMGPTR(images[3], 0, x * regions->vHeight), regions->vWidth, regions->vHeight, images[3]->bytes_per_line);

				// regions->regionVRight.push_back( bufV_tmp );			
			}

			// save(*image, "zzz.pnm");
			
			*readyToProcess = false;
			emit onSparseImageCaptured( regions );
			++frameCounter;
			updateDelayTime();
		}		
		usleep( delayTime );
	}

	cleanup();
	emit setIsRunning( false );
	//*stopThread = true;
}
void CXShmCaptureThread::cleanup() {
}
