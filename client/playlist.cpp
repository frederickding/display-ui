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

#include "display_ui.h"
#include "playlist.h"
#include "debug.h"

typedef struct _playlist_f_t{
	byte num_elements;
	byte data[1];
}playlist_f_t;

typedef struct _playlist_element_f_t{
	byte type;
	byte secs;
	short trans;
	// int id;
	int len; // 12 + 8 + filename len * 2
	byte data[1];
}playlist_element_f_t;

typedef struct _image_element_f_t{
	short width, height;
	int filename_len;
	char filename[1];
}image_element_f_t;


playlist_t g_playlist;


void playlist_load(){

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
				new_img->filename = (wchar_t *) malloc((current_img->filename_len + 1) * 2);
				memset(new_img->filename, 0, (current_img->filename_len + 1) * 2);

				new_img->width = current_img->width;
				new_img->height = current_img->height;
				memcpy(new_img->filename, current_img->filename, current_img->filename_len * 2);
				debug_print("width: %d, height: %d, filename: %ws\n", new_img->width, new_img->height, new_img->filename);
				
				new_elem->data = new_img;
				
			}
			
			if(i == 0) g_playlist.first = new_elem;
			if(prev_elem) prev_elem->next = new_elem;
			prev_elem = new_elem;
			new_elem->next = g_playlist.first;
			current = (playlist_element_f_t *) ((unsigned long)current + (unsigned long)current->len);

			debug_print("%08lX -- %08lX\n", raw_playlist, current);
		}

		free(raw_playlist);
		fclose(fp);
	}	

}
