#include <stdio.h>
#include <windows.h>
#include "debug.h"

FILE *log;

void debug_init(){
	log = fopen("log.txt", "w");
}

int debug_print(char *format, ...){
	char buf[256];
	memset(buf, 0, 256);
	va_list args; va_start(args, format);	
	int i = vsprintf(buf, format, args);
	fprintf(log, buf);	
	va_end(args);
	return i;
}

void debug_close(){
	
	fclose(log);
}
