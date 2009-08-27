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

#include "display_ui.h"
#include "freeimage.h"
#include "playlist.h"
#include "video.h"
#include "debug.h"

#define MEDIA_TYPE_IMAGE 1
#define MEDIA_TYPE_VIDEO 3

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

// Callback for cURL's progress updates during download
int playlist_media_dl_progress_cb(void *arg, double dltotal, double dlnow, double ultotal, double ulnow){
	playlist_element_t *element = (playlist_element_t *) arg;
	
	// Update the progress in the playlist_element_t structure
	element->progress_total = (int) dltotal;
	element->progress_now   = (int) dlnow;
	return 0;
}

// Does the actual loading. This is called from a thread in order to do simultaneous downloads.
// params:
//		p:	A pointer to a playlist_thread_elem_t structure describing the element to load.
//			Function assumes that the element is an image. This is freed at end of this function.
void playlist_image_doload(void *p){
	playlist_thread_elem_t *th_image = (playlist_thread_elem_t *) p;
	playlist_element_t *element = th_image->elem;
	image_element_t *new_img = (image_element_t *) element->data;

	char download_src[24 + 15];
	
	debug_print("[playlist_image_doload] downloading image... %s\n", new_img->filename);

	sprintf(download_src, "/api/media/download/?id=%d", element->id);
	
	// Download the image if it doesn't exist yet.
	// TODO: Do some sort of check to see if image on the server differs from the local copy.
	if(!file_exists(new_img->filename)){
		if(dui_download(download_src, new_img->filename, true, (void *)playlist_media_dl_progress_cb, (void *)element) != CURLE_OK){
			debug_print("[playlist_image_doload] Image download failed! ID: %d, URL: %s\n", element->id, download_src);
			return;
		}
	}
	
	// Load image from disk.
	new_img->type = FreeImage_GetFileType(new_img->filename, 0);
	if(new_img->type == FIF_UNKNOWN){
		new_img->type = FreeImage_GetFIFFromFilename(new_img->filename);
	}
	new_img->fbmp_image = FreeImage_Load((FREE_IMAGE_FORMAT)new_img->type, new_img->filename, NULL);
	
	long width = FreeImage_GetWidth(new_img->fbmp_image);
	long height = FreeImage_GetHeight(new_img->fbmp_image);
	
	// If width or height exceeds the dimensions of the content area, resize the image
	if(width > CONTENT_WIDTH){
		debug_print("[playlist_image_doload] Image too large! Resizing...\n");

		// Calculate new dimensions.
		height = CONTENT_WIDTH * height / width;
		width = CONTENT_WIDTH;
		
		// Resize the image.
		FIBITMAP *temp = new_img->fbmp_image;
		new_img->fbmp_image = FreeImage_Rescale(new_img->fbmp_image, width, height, FILTER_BICUBIC);
		FreeImage_Unload(temp);
		debug_print("[playlist_image_doload] New dimensions: %dx%d.\n", width, height);
	}
	if(height > CONTENT_HEIGHT){
		debug_print("[playlist_image_doload] Image too large! Resizing...\n");

		// Calculate new dimensions.
		width = CONTENT_HEIGHT * width / height;
		height = CONTENT_HEIGHT;
		
		// Resize the image.
		FIBITMAP *temp = new_img->fbmp_image;
		new_img->fbmp_image = FreeImage_Rescale(new_img->fbmp_image, width, height, FILTER_BICUBIC);
		FreeImage_Unload(temp);
		debug_print("[playlist_image_doload] New dimensions: %dx%d.\n", width, height);
	}
	
	// Pre-multiply PNG images to make alpha transparency work as it should.
	if(new_img->type == FIF_PNG) FreeImage_PreMultiplyWithAlpha(new_img->fbmp_image);

	debug_print("[playlist_image_doload] new_img->fbmp_image = %08lX\n", new_img->fbmp_image);

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
	debug_print("[playlist_image_doload] Image loaded.\n");

	// Tell the program that the image is ready for drawing.
	element->ready = true;
}


// Does the actual loading. This is called from a thread in order to do simultaneous downloads.
// params:
//		p:	A pointer to a playlist_thread_elem_t structure describing the element to load.
//			Function assumes that the element is a video. This is freed at end of this function.
void playlist_video_doload(void *p){
	playlist_thread_elem_t *th_vid = (playlist_thread_elem_t *) p;
	playlist_element_t *element = th_vid->elem;
	video_element_t *new_vid = (video_element_t *) element->data;
	
	char download_src[24 + 15];

	debug_print("[playlist_video_doload] downloading video... %s\n", new_vid->filename);
	
	// Download the video from the server.
	sprintf(download_src, "/api/media/download/?id=%d", element->id);
	if(!file_exists(new_vid->filename)){
		if(dui_download(download_src, new_vid->filename, true, (void *)playlist_media_dl_progress_cb, (void *)element) != CURLE_OK){
			debug_print("[playlist_video_doload] image download failed! %s\n", download_src);
			return;
		}
	}
	
	free(p);

	// Tell the program that the video is ready for rendering.
	element->ready = true;
}

// Dumps all playlist elements to the debug file.
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

// Load the raw playlist file from disk into memory
int playlist_load_raw(){
	FILE *playlist_file;

	if(file_exists("data\\playlist.dat")){
		playlist_file = fopen("data\\playlist.dat", "r");

		fseek(playlist_file, 0, SEEK_END);
		g_playlist_raw_size = ftell(playlist_file);
		fseek(playlist_file, 0, SEEK_SET);
		
		if(g_playlist_raw_size == 0) return -1;
		
		if(g_playlist_raw) free(g_playlist_raw);
		g_playlist_raw = (char *) malloc(g_playlist_raw_size);
		fread(g_playlist_raw, 1, g_playlist_raw_size, playlist_file);
		
		fclose(playlist_file);

		return 0;
	}

	return -1;
}

// Process the raw playlist data. Create playlist structs and load
// media where necessary.
// params:
//		hwnd:	Handle to window in which playlist items will be rendered.
void playlist_process_raw(HWND hwnd){
	
	playlist_f_t *playlist = (playlist_f_t *) g_playlist_raw;
	g_playlist.num_elements = playlist->num_elements;
	debug_print("[playlist_process_raw] number of elements: %d **\n", g_playlist.num_elements);
	
	playlist_element_f_t *current = (playlist_element_f_t *) playlist->data;		
	playlist_element_t *prev_elem = NULL;

	// Loop through all elements
	for(int i = 0; i < playlist->num_elements; i++){
		debug_print("[playlist_process_raw] element type: %d\n", current->type);
		debug_print("[playlist_process_raw]         secs: %d\n", current->secs);
		debug_print("[playlist_process_raw]       length: %x\n", current->len);
		
		// Create a new entry in the linked list
		playlist_element_t *new_elem = (playlist_element_t *) malloc(sizeof(playlist_element_t));
		memset(new_elem, 0, sizeof(playlist_element_t));
		
		// Transfer options from raw entry to linked list entry
		new_elem->type = current->type;
		new_elem->secs = current->secs;
		new_elem->trans = current->trans;
		new_elem->id = current->id;
		
		if(new_elem->type == MEDIA_TYPE_IMAGE){
			image_element_f_t *current_img = (image_element_f_t *) current->data;
			image_element_t *new_img = (image_element_t *) malloc(sizeof(image_element_t));
	
			memset(new_img, 0, sizeof(image_element_t));
			
			// Filenames are [id].[ext]
			sprintf(new_img->filename, "media\\%d.%s", current->id, current_img->ext);
			
			debug_print("[playlist_process_raw] filename: %s\n", new_img->filename);
			
			playlist_thread_elem_t *i = (playlist_thread_elem_t *) malloc(sizeof(playlist_thread_elem_t));
			i->hwnd = hwnd; i->elem = new_elem;
			new_elem->data = new_img;
			
			// Download to disk and load media in a separate thread.
			CloseHandle(CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))playlist_image_doload, (void *)i, 0, NULL));
		}else if(new_elem->type == MEDIA_TYPE_VIDEO){
			video_element_f_t *current_vid = (video_element_f_t *) current->data;
			video_element_t *new_vid = (video_element_t *) malloc(sizeof(video_element_t));
			memset(new_vid, 0, sizeof(video_element_t));
			
			// Need widechar version of filename for DShow
			sprintf(new_vid->filename, "media\\%d.%s", current->id, current_vid->ext);
			mbstowcs(new_vid->filename_w, new_vid->filename, strlen(new_vid->filename));
			
			debug_print("[playlist_process_raw] filename: %s\n", new_vid->filename);
			debug_print("[playlist_process_raw] wfilename: %ws\n", new_vid->filename_w);
			
			playlist_thread_elem_t *i = (playlist_thread_elem_t *) malloc(sizeof(playlist_thread_elem_t));
			i->hwnd = hwnd; i->elem = new_elem;
			new_elem->data = new_vid;
			
			// Download to disk and load media in a separate thread.
			CloseHandle(CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))playlist_video_doload, (void *)i, 0, NULL));
		}
		
		// Insert element into linked list
		if(i == 0) g_playlist.first = new_elem;
		if(prev_elem) prev_elem->next = new_elem;
		prev_elem = new_elem;
		new_elem->next = g_playlist.first;

		// Increment pointer to point to next entry in raw playlist data
		current = (playlist_element_f_t *) ((unsigned long)current + (unsigned long)current->len);
		
		
		//debug_print("%08lX -- %08lX .. %d\n", g_playlist_raw, new_elem, new_elem->type);
	}
	
	free(g_playlist_raw);
}

// Attempts to download a playlist, loads old one from disk if can't connect
void playlist_load(HWND hwnd){
	int result = dui_download("/api/playlists/fetch/", "data\\playlist.dat");

	debug_print("[playlist_load] DL result: %d\n", result);
	if(result == CURLE_OK) {
		debug_print("[playlist_load] download operation succeeded ... loading new playlist\n");
		if(playlist_load_raw() == 0){
			playlist_process_raw(hwnd);			
		}	
	}else if(result == CURLE_COULDNT_RESOLVE_HOST){
		debug_print("[playlist_load] download operation timed out ... loading old playlist\n");
		if(playlist_load_raw() == 0){
			playlist_process_raw(hwnd);				
		}
	}

	//playlist_dumpitems();
}

void playlist_unload(){
	playlist_element_t *current = g_playlist.first;

	for(int i = 0; i < g_playlist.num_elements; i++){
		debug_print("[playlist_unload] type: %d -- address: 0x%08lX\n", current->type, current);
		if(current->type == 1){
			image_element_t *current_img = (image_element_t *) current->data;
			debug_print("[playlist_unload -- image] unloading %s...\n", current_img->filename);
			if(current_img->fbmp_image) FreeImage_Unload(current_img->fbmp_image);
			if(current_img->filename) free(current_img->filename);
			DeleteObject(current_img->hbm);
			DeleteDC(current_img->hdc);
			free(current_img);
		}else if(current->type == 2){
			video_element_t *current_vid = (video_element_t *) current->data;
			debug_print("[playlist_unload -- video] unloading %s...\n", current_vid->filename);
			free(current_vid);
		}

		playlist_element_t *tmp = current;
		current = current->next;
		free(tmp);
	}
}

