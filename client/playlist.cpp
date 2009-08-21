/**
 * playlist.cpp, playlist stuff
 *
 * Copyright 2009 Kirill Peretoltchine
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *      http://www.apache.org/licenses/LICENSE-2.0
 * or the full licensing terms for this project at
 *      http://code.google.com/p/display-ui/wiki/License
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * $Id: weather.cpp 138 2009-07-30 01:56:07Z frederickjding $
 */

#include <windows.h>
#include <urlmon.h>
#include <stdio.h>
#include <time.h>

#include <Qedit.h>
#include <dshow.h>

#include "display_ui.h"
#include "freeimage.h"
#include "playlist.h"
#include "video.h"
#include "debug.h"

#pragma pack(1)

typedef struct _playlist_f_t{
	short num_elements;
	byte data[1];
}playlist_f_t;

typedef struct _playlist_element_f_t{
	byte type;
	byte secs;
	byte trans;
	int id;
	int len; // 11 + 4 + filename
	byte data[1];
}playlist_element_f_t;

typedef struct _image_element_f_t{
	short filename_len;
	char filename[1];
}image_element_f_t;

typedef struct _video_element_f_t{
	short filename_len;
	char filename[1];
}video_element_f_t;

#pragma pack()



playlist_t g_playlist;


void playlist_load(HWND hwnd){

	FILE *fp = fopen("data\\playlist.dat", "r");
	
	if(fp){
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char *raw_playlist = (char *) malloc(size);

		fread(raw_playlist, 1, size, fp);
		
		playlist_f_t *playlist = (playlist_f_t *) raw_playlist;
		g_playlist.num_elements = playlist->num_elements;
		debug_print("number of elements: %d **\n", g_playlist.num_elements);
		
		playlist_element_f_t *current = (playlist_element_f_t *) playlist->data;		
		
		playlist_element_t *prev_elem = NULL;
		for(int i = 0; i < playlist->num_elements; i++){
			debug_print("element type: %d\nsecs: %d\nlen: %x\n", current->type, current->secs, current->len);
			playlist_element_t *new_elem = (playlist_element_t *) malloc(sizeof(playlist_element_t));
			new_elem->type = current->type;
			new_elem->secs = current->secs;
			new_elem->trans = current->trans;
			
			if(new_elem->type == 1){
				// image
				image_element_f_t *current_img = (image_element_f_t *) current->data;
				image_element_t *new_img = (image_element_t *) malloc(sizeof(image_element_t));
				memset(new_img, 0, sizeof(image_element_t));
				new_img->filename = (char *) malloc(current_img->filename_len + 1);
				memset(new_img->filename, 0, current_img->filename_len + 1);

				
				memcpy(new_img->filename, current_img->filename, current_img->filename_len);
				debug_print("filename: %s\n", new_img->filename);
				
				new_img->type = FreeImage_GetFileType(new_img->filename, 0);
				if(new_img->type == FIF_UNKNOWN){
					new_img->type = FreeImage_GetFIFFromFilename(new_img->filename);
				}
				new_img->fbmp_image = FreeImage_Load((FREE_IMAGE_FORMAT)new_img->type, new_img->filename, NULL);
				if(new_img->type == FIF_PNG) FreeImage_PreMultiplyWithAlpha(new_img->fbmp_image);
				

				HDC hdcWin = GetDC(hwnd);
				new_img->hdc = CreateCompatibleDC(hdcWin);
				new_img->hbm = CreateCompatibleBitmap(hdcWin, FreeImage_GetWidth(new_img->fbmp_image), FreeImage_GetHeight(new_img->fbmp_image));
				SelectObject(new_img->hdc, new_img->hbm);
				
				// Draw the image once to the DC to avoid having to redraw every time a frame is rendered
				StretchDIBits(new_img->hdc, 0, 0, FreeImage_GetWidth(new_img->fbmp_image), FreeImage_GetHeight(new_img->fbmp_image), 
					0, 0, FreeImage_GetWidth(new_img->fbmp_image), FreeImage_GetHeight(new_img->fbmp_image), 
					FreeImage_GetBits(new_img->fbmp_image), FreeImage_GetInfo(new_img->fbmp_image), DIB_RGB_COLORS, SRCCOPY);
				ReleaseDC(hwnd, hdcWin);

				new_img->bf.BlendOp = AC_SRC_OVER;
				new_img->bf.BlendFlags = 0;
				new_img->bf.SourceConstantAlpha = 255;
				new_img->bf.AlphaFormat = (new_img->type == FIF_PNG ? AC_SRC_ALPHA : AC_SRC_OVER);

				new_elem->data = new_img;

			}else if(new_elem->type == 2){
				video_element_f_t *current_vid = (video_element_f_t *) current->data;
				video_element_t *new_vid = (video_element_t *) malloc(sizeof(video_element_t));
				memset(new_vid, 0, sizeof(video_element_t));
				new_vid->filename = (wchar_t *) malloc((current_vid->filename_len + 1) * 2);
				memset(new_vid->filename, 0, (current_vid->filename_len + 1) * 2);
				
				mbstowcs(new_vid->filename, current_vid->filename, current_vid->filename_len);
				
				debug_print("filename: %ws\n", new_vid->filename);
				
				long streams;
				HRESULT hr;
				
				if(video_getpointer_VMRwc() == NULL){
					debug_print("initializing video..\n");
					video_init();
				}
				
				new_elem->data = new_vid;

				debug_print(" -- %d asdf\n", __LINE__);
				
				//video_load(hwnd, new_vid);


				/*
				hr = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC, IID_IMediaDet, (void**)&new_vid->iMedDet);
				if(FAILED(hr)){
					debug_print("CoCreateInstance failed: hr = %08lX\n", hr);
					continue;
				}
				BSTR bstrFileName = L"C:\\downloaded\\[Shamisen] Melancholy of Haruhi-chan - 13 (XviD 704x400 4E9016C4).avi";
					//L"C:\\Program Files\\Microsoft Platform SDK\\Samples\\Multimedia\\DirectShow\\Media\\Video\\skiing.avi";
				
				new_vid->iMedDet->put_Filename(bstrFileName);
				new_vid->iMedDet->get_OutputStreams(&streams);
				bool bFound = false;


				for (int i = 0; i < streams; i++){
					GUID major_type;
					hr = new_vid->iMedDet->put_CurrentStream(i);
					if (SUCCEEDED(hr))
						hr = new_vid->iMedDet->get_StreamType(&major_type);
					if (FAILED(hr))
						break;
					if (major_type == MEDIATYPE_Video)
					{
						bFound = true;
						break;
					}
				}
				if (!bFound){
					debug_print("faaaaaaaiill");
					return;
				}

				AM_MEDIA_TYPE mt;
				new_vid->iMedDet->get_StreamMediaType(&mt);
				if ((mt.formattype == FORMAT_VideoInfo) &&
					(mt.cbFormat >= sizeof(VIDEOINFOHEADER))){
					VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)(mt.pbFormat);
					new_vid->width = pVih->bmiHeader.biWidth;
					new_vid->height = pVih->bmiHeader.biHeight;
					
					// We want the absolute m_Height, don't care about orientation.
					if (new_vid->width < 0) new_vid->height *= -1;
				}
				else{
					hr = VFW_E_INVALIDMEDIATYPE; // Should not happen, in theory.
				}
				//FreeMediaType(mt);
				
				double stream_length;
				new_vid->iMedDet->get_FrameRate(&new_vid->fps);
				new_vid->iMedDet->get_StreamLength(&stream_length);
				//new_vid->framebuf = malloc(4 * new_vid->width * new_vid->height);
				//new_vid->frame_datasize = 4 * new_vid->width * new_vid->height;
				new_vid->num_frames = new_vid->fps * stream_length;
				
 				new_vid->iMedDet->EnterBitmapGrabMode(0);
 				new_vid->iMedDet->GetSampleGrabber(&new_vid->iSplGrab);

			//	new_vid->iSplGrab->SetBufferSamples(true);
			//	new_vid->iSplGrab->SetOneShot(true);

				debug_print("frame rate: %f , length: %f\n", new_vid->fps, stream_length);
*/

			}
			
			if(i == 0) g_playlist.first = new_elem;
			if(prev_elem) prev_elem->next = new_elem;
			prev_elem = new_elem;
			new_elem->next = g_playlist.first;
			current = (playlist_element_f_t *) ((unsigned long)current + (unsigned long)current->len);
			

			debug_print("%08lX -- %08lX .. %d\n", raw_playlist, new_elem, new_elem->type);
		}

		free(raw_playlist);
		fclose(fp);
	}	
}

void playlist_unload(){
	playlist_element_t *current = g_playlist.first;

	for(int i = 0; i < g_playlist.num_elements; i++){
		debug_print("unload: %d -- %08lX\n", current->type, current);
		if(current->type == 1){
			image_element_t *current_img = (image_element_t *) current->data;
			debug_print("unload: %s\n", current_img->filename);
			if(current_img->fbmp_image) FreeImage_Unload(current_img->fbmp_image);
			if(current_img->filename) free(current_img->filename);
			DeleteObject(current_img->hbm);
			DeleteDC(current_img->hdc);
			free(current_img);
		}else if(current->type == 2){
			video_element_t *current_vid = (video_element_t *) current->data;
			debug_print("unload: %s\n", current_vid->filename);
			//if(current_vid->iMedDet) current_vid->iMedDet->Release();
			//if(current_vid->iSplGrab) current_vid->iSplGrab->Release();
			//if(current_vid->framebuf) free(current_vid->framebuf);
			
/*			if(current_vid->iVMRwc) current_vid->iVMRwc->Release();
			if(current_vid->iGraphBldr) current_vid->iGraphBldr->Release();
			if(current_vid->iMedCtrl) current_vid->iMedCtrl->Release();
			if(current_vid->iMedEvt) current_vid->iMedEvt->Release();
			if(current_vid->iMedSeek) current_vid->iMedSeek->Release(); */
			//video_unload(current_vid);
			//DeleteObject(current_vid->hbm);
			//DeleteDC(current_vid->hdc);

			free(current_vid);
		}
		playlist_element_t *tmp = current;
		current = current->next;
		free(tmp);
	}
}