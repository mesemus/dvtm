#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#ifdef __cplusplus
extern "C" {
#endif

int  bridge_create_window();
void bridge_close_all_windows();
void bridge_draw_all();
void bridge_mark();

typedef struct {
	char *buf;
	int bufsize;
} TerminalOutput;

TerminalOutput bridge_get_output_from_mark();
void bridge_insert_input_string(const char *s, int length);



void bridge_on_create(const char **args);
void bridge_select_project(const char **args);

// processing loop, to be used for example in waits ...
bool
process_input(int timeoutmillis);

#ifdef __cplusplus
}
#endif

#endif
