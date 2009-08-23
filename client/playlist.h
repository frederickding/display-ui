#ifndef playlist_header
#define playlist_header

#include "freeimage.h"

typedef struct _playlist_element_t{
	byte type;
	byte secs;
	byte trans;
	int id;
	
	bool ready;
	double progress_total;
	double progress_now;
	
	void *data;
	_playlist_element_t *next;
}playlist_element_t;

typedef struct _playlist_t{
	short num_elements;
	_playlist_element_t *first;
}playlist_t;

typedef struct _image_element_t{
	byte type;
	long width, height;

	FIBITMAP *fbmp_image;
	HDC hdc;
	HBITMAP hbm;
	BLENDFUNCTION bf;
	char filename[22];
}image_element_t;

typedef struct _video_element_t{

// 	BITMAPINFOHEADER *bmi;
// 	void *framebuf;

// 	IMediaDet *iMedDet;
//  	ISampleGrabber *iSplGrab;
	
	//HDC hdc;
	//HBITMAP hbm;

// 	IVMRWindowlessControl *iVMRwc;
// 	IGraphBuilder *iGraphBldr;
// 	IMediaControl *iMedCtrl;
// 	IMediaEvent *iMedEvt;
// 	IMediaSeeking *iMedSeek;
	
	char filename[22];
	wchar_t filename_w[22];
}video_element_t;

//void playlist_dumpitems();

void playlist_load(HWND hwnd);
void playlist_unload();

#endif
