#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#ifdef __cplusplus
extern "C" {
#endif

int  bridge_create_window();
void bridge_close_all_windows();
void bridge_draw_all();

void bridge_on_create(const char **args);
void bridge_select_project(const char **args);

#ifdef __cplusplus
}
#endif

#endif
