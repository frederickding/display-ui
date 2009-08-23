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
//#define CURL_STATICLIB

#include <windows.h>
#include <urlmon.h>
#include <stdio.h>
#include <time.h>

#include <Qedit.h>
#include <dshow.h>

#include <curl\curl.h>

// because dshow.h does REALLY stupid stuff
// (see line 7673 of strsafe.h. WTF, microsoft)
#undef sprintf
#undef swprintf
#undef strcat
//#define sprintf sprintf
//#define swprintf swprintf
//#define strcat strcat

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
	char ext[1];
}image_element_f_t;

typedef struct _video_element_f_t{
	char ext[1];
}video_element_f_t;

#pragma pack()

typedef struct _playlist_thread_elem_t{
	playlist_element_t *elem;
	HWND hwnd;
}playlist_thread_elem_t;


playlist_t g_playlist;
char *g_playlist_raw;
int g_playlist_raw_size;

bool file_exists(const char * filename){
    if (FILE * file = fopen(filename, "r")){
        fclose(file);
        return true;
    }
    return false;
}


int playlist_media_dl_progress_cb(void *arg, double dltotal, double dlnow, double ultotal, double ulnow){
	playlist_element_t *element = (playlist_element_t *) arg;
	
	element->progress_total = (int) dltotal;
	element->progress_now   = (int) dlnow;
	return 0;
}

void playlist_image_doload(void *p){
	playlist_thread_elem_t *th_image = (playlist_thread_elem_t *) p;
	playlist_element_t *element = th_image->elem;
	image_element_t *new_img = (image_element_t *) element->data;

	char download_src[69 + 15];
	memset(download_src, 0, 69 + 15);
	
	debug_print("downloading image... %s\n", new_img->filename);

	sprintf(download_src, "http://du.geekie.org/server/api/media/download/?sys_name=1&sig=%%s&id=%d", element->id);
	
	//debug_print("downloading ... [%s] -> [%s]\n", download_src, download_dest);
	if(!file_exists(new_img->filename)){
		if(dui_download(download_src, new_img->filename, true, (void *)playlist_media_dl_progress_cb, (void *)element) != CURLE_OK){
			debug_print("image download failed! %d %s\n", element->id, download_src);
			return;
		}
	}
	
	new_img->type = FreeImage_GetFileType(new_img->filename, 0);
	if(new_img->type == FIF_UNKNOWN){
		new_img->type = FreeImage_GetFIFFromFilename(new_img->filename);
	}
	new_img->fbmp_image = FreeImage_Load((FREE_IMAGE_FORMAT)new_img->type, new_img->filename, NULL);
	
	long width = FreeImage_GetWidth(new_img->fbmp_image);
	long height = FreeImage_GetHeight(new_img->fbmp_image);
	
	if(width > CONTENT_WIDTH){
		debug_print("image too large! resizing\n");
		height = CONTENT_WIDTH * height / width;
		width = CONTENT_WIDTH;
		FIBITMAP *temp = new_img->fbmp_image;
		new_img->fbmp_image = FreeImage_Rescale(new_img->fbmp_image, width, height, FILTER_BICUBIC);
		FreeImage_Unload(temp);
		debug_print("new dimensions: %d x %d ... \n", width, height);
	}else if(height > CONTENT_HEIGHT){
		width = CONTENT_HEIGHT * width / height;
		height = CONTENT_HEIGHT;
		FIBITMAP *temp = new_img->fbmp_image;
		new_img->fbmp_image = FreeImage_Rescale(new_img->fbmp_image, width, height, FILTER_BICUBIC);
		FreeImage_Unload(temp);
	}
	
	if(new_img->type == FIF_PNG) FreeImage_PreMultiplyWithAlpha(new_img->fbmp_image);
	debug_print("new_img->fbmp_image = %08lX\n", new_img->fbmp_image);

	HDC hdcWin = GetDC(th_image->hwnd);
	new_img->hdc = CreateCompatibleDC(hdcWin);
	
	
	
	new_img->hbm = CreateCompatibleBitmap(hdcWin, width, height);
	SelectObject(new_img->hdc, new_img->hbm);
	
	// Draw the image once to the DC to avoid having to redraw every time a frame is rendered
	StretchDIBits(new_img->hdc, 0, 0, width, height, 0, 0, width, height, 
		FreeImage_GetBits(new_img->fbmp_image), FreeImage_GetInfo(new_img->fbmp_image), DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(th_image->hwnd, hdcWin);
	
	new_img->width = width;
	new_img->height = height;
	new_img->bf.BlendOp = AC_SRC_OVER;
	new_img->bf.BlendFlags = 0;
	new_img->bf.SourceConstantAlpha = 255;
	new_img->bf.AlphaFormat = (new_img->type == FIF_PNG ? AC_SRC_ALPHA : AC_SRC_OVER);
	
	free(p);
	debug_print("image loaded.\n");
	element->ready = true;
}


void playlist_video_doload(void *p){
	playlist_thread_elem_t *th_vid = (playlist_thread_elem_t *) p;
	playlist_element_t *element = th_vid->elem;
	video_element_t *new_vid = (video_element_t *) element->data;
	
	char download_src[69 + 15];
	memset(download_src, 0, 69 + 15);

	debug_print("downloading video... %s\n", new_vid->filename);
	
	sprintf(download_src, "http://du.geekie.org/server/api/media/download/?sys_name=1&sig=%%s&id=%d", element->id);
	//debug_print("downloading ... [%s] -> [%s]\n", download_src, download_dest);
	free(p);
	if(!file_exists(new_vid->filename)){
		if(dui_download(download_src, new_vid->filename, true, (void *)playlist_media_dl_progress_cb, (void *)element) != CURLE_OK){
			debug_print("image download failed! %s\n", download_src);
			return;
		}
	}

	element->ready = true;

	
}

void playlist_dumpitems(){
	playlist_element_t *current = g_playlist.first;

	do{
		debug_print("[playlist_dumpitems] current: %08lX\n", current);
		debug_print("[playlist_dumpitems] type: %d\n", current->type);
		debug_print("[playlist_dumpitems] ID: %d\n", current->id);
		if(current->type == 1){
			image_element_t *img = (image_element_t *)current->data;
			debug_print("[playlist_dumpitems] img filename: %s\n", img->filename);
		}else if(current->type == 3){
			video_element_t *vid = (video_element_t *)current->data;
			debug_print("[playlist_dumpitems] vid filename: %s\n", vid->filename);
		}
		current = current->next;
	}while(current != g_playlist.first);
}


int playlist_load_raw(){
	FILE *playlist_file;

	if(file_exists("data\\playlist.dat")){
		playlist_file = fopen("data\\playlist.dat", "r");

		fseek(playlist_file, 0, SEEK_END);
		g_playlist_raw_size = ftell(playlist_file);
		fseek(playlist_file, 0, SEEK_SET);
		
		if(g_playlist_raw_size == 0) return -1;

		g_playlist_raw = (char *) malloc(g_playlist_raw_size);
		fread(g_playlist_raw, 1, g_playlist_raw_size, playlist_file);
		
		fclose(playlist_file);

		return 0;
	}

	return -1;
}

void playlist_process_raw(HWND hwnd){
	
	playlist_f_t *playlist = (playlist_f_t *) g_playlist_raw;
	g_playlist.num_elements = playlist->num_elements;
	debug_print("number of elements: %d **\n", g_playlist.num_elements);
	
	playlist_element_f_t *current = (playlist_element_f_t *) playlist->data;		
	
	playlist_element_t *prev_elem = NULL;
	for(int i = 0; i < playlist->num_elements; i++){
		debug_print("element type: %d\nsecs: %d\nlen: %x\n", current->type, current->secs, current->len);
		playlist_element_t *new_elem = (playlist_element_t *) malloc(sizeof(playlist_element_t));
		memset(new_elem, 0, sizeof(playlist_element_t));

		new_elem->type = current->type;
		new_elem->secs = current->secs;
		new_elem->trans = current->trans;
		new_elem->id = current->id;
		
		if(new_elem->type == 1){
			// image
			image_element_f_t *current_img = (image_element_f_t *) current->data;
			image_element_t *new_img = (image_element_t *) malloc(sizeof(image_element_t));
			
			memset(new_img, 0, sizeof(image_element_t));
			sprintf(new_img->filename, "media\\%d.%s", current->id, current_img->ext);
			
			debug_print("filename: %s\n", new_img->filename);
			
			playlist_thread_elem_t *i = (playlist_thread_elem_t *) malloc(sizeof(playlist_thread_elem_t));
			i->hwnd = hwnd; i->elem = new_elem;
			
			new_elem->data = new_img;
			
			//playlist_image_doload((void *) i);
			CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))playlist_image_doload, (void *)i, 0, NULL);
			
			
			
		}else if(new_elem->type == 3){
			video_element_f_t *current_vid = (video_element_f_t *) current->data;
			video_element_t *new_vid = (video_element_t *) malloc(sizeof(video_element_t));
			memset(new_vid, 0, sizeof(video_element_t));
			
			
			sprintf(new_vid->filename, "media\\%d.%s", current->id, current_vid->ext);
			mbstowcs(new_vid->filename_w, new_vid->filename, strlen(new_vid->filename));
			//swprintf(new_vid->filename_w, L"media\\%d.%s", current->id, current_vid->ext);
			
			debug_print("filename: %s\nwfilename: %ws\n", new_vid->filename, new_vid->filename_w);
			
			
			
			playlist_thread_elem_t *i = (playlist_thread_elem_t *) malloc(sizeof(playlist_thread_elem_t));
			i->hwnd = hwnd; i->elem = new_elem;
			
			new_elem->data = new_vid;
			
			//playlist_video_doload((void *)i);
			CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))playlist_video_doload, (void *)i, 0, NULL);
			
			debug_print(" -- %d asdf\n", __LINE__);
			
			//video_load(hwnd, new_vid);
			
		}
		
		if(i == 0) g_playlist.first = new_elem;
		if(prev_elem) prev_elem->next = new_elem;
		prev_elem = new_elem;
		new_elem->next = g_playlist.first;
		current = (playlist_element_f_t *) ((unsigned long)current + (unsigned long)current->len);
		
		
		debug_print("%08lX -- %08lX .. %d\n", g_playlist_raw, new_elem, new_elem->type);
	}
	
	free(g_playlist_raw);
}

void playlist_load(HWND hwnd){
	int loaded_old = playlist_load_raw();
	debug_print("playlist_load\n");
	int result = dui_download("http://du.geekie.org/server/api/playlists/fetch/?sys_name=1&sig=%s", "data\\playlist.dat");
	debug_print("playlist_load .. %d\n", result);
	if(result == CURLE_OK) {
		free(g_playlist_raw);

		if(playlist_load_raw() == 0){
			playlist_process_raw(hwnd);			

		}	
	}else if(result == CURLE_COULDNT_RESOLVE_HOST){
		debug_print("[playlist_load] download operation timed out\n");
		if(loaded_old) playlist_process_raw(hwnd);

		FILE *fp = fopen("data\\playlist.dat", "wb");
		fwrite(g_playlist_raw, 1, g_playlist_raw_size, fp);
		fclose(fp);
	}

	//playlist_dumpitems();
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

