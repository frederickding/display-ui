#ifndef playlist_header
#define playlist_header

#include "freeimage.h"

typedef struct _playlist_element_t{
	byte type;
	int secs;
	int trans;
	
	void *data;
	_playlist_element_t *next;
	_playlist_element_t *prev;
}playlist_element_t;

typedef struct _playlist_t{
	short num_elements;
	_playlist_element_t *first;
}playlist_t;

typedef struct _image_element_t{
	byte type;
	FIBITMAP *fbmp_image;
	HDC hdc;
	HBITMAP hbm;
	BLENDFUNCTION bf;
	char *filename;
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
	
	
	wchar_t *filename;
}video_element_t;


void playlist_load(HWND hwnd);
void playlist_unload();

#endif
