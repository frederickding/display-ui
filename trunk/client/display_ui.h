#ifndef display_ui_header
#define display_ui_header

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define CONTENT_WIDTH SCREEN_WIDTH - 300
#define CONTENT_HEIGHT SCREEN_HEIGHT - 112 - 56

#define CONTENT_TOP 112

void generate_sig();

// use the first functions if sig needs to be appended
int dui_download(char *url, char *dest_file);
int dui_download(char *url, char *dest_file, bool show_progress, void *callback, void *arg);

int download_curl(char *url, char *dest_file);
int download_curl(char *url, char *dest_file, bool show_progress, void *callback, void *arg);

#endif
