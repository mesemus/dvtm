#include "script.h"
#include "precompiled_headers.h"
#include "init.cc"

#ifdef COMPILE_IN_CHAI_STDLIB

#	include <chaiscript/chaiscript_stdlib.hpp>
	chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());

#else

	chaiscript::ChaiScript chai;

#endif

using namespace std;
namespace fs = boost::filesystem;

typedef vector<fs::path> filevec;

filevec project_files;

void initialize_scripting() {
	chai.add(chaiscript::fun(&bridge_create_window), "create_window");
	chai.eval(dvtm_script_initialization_string);

}

void load_project_files() {

	const char *homedir;

	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}

	fs::path dir(homedir);
	fs::path config_dir = homedir / fs::path(".config") / fs::path("dvtm");

	if (!fs::exists(config_dir))
		return;

	fs::directory_iterator end_itr; // default construction yields past-the-end
	for (fs::directory_iterator itr(config_dir); itr != end_itr; ++itr) {
		fs::path fn = itr->path();
		// ignore unknown files
		if (fn.extension() != ".chai") {
			continue;
		}
		// initialization file ...
		if (fn.stem() == "init") {
			// source the file
			chai.eval_file(fn.string());
			continue;
		}
		project_files.push_back(itr->path());
	}
	sort(project_files.begin(), project_files.end());
}

extern "C" void bridge_on_create(const char **args) {
	initialize_scripting();
	load_project_files();
}

static void _init_dialog_library() {
	dialog_state.screen_initialized = true;
	dialog_state.output = stdout;
	dialog_state.pipe_input = stdin;
	dialog_state.screen_output = stdout;
	dialog_state.use_colors = false; /* use colors by default? */
	dialog_state.use_shadow = true; /* shadow dialog boxes by default? */
	if (dialog_state.use_colors) {
		dlg_color_setup();
	}
}

extern "C" void bridge_select_project(const char **args) {
	_init_dialog_library();

	const char *menu_items[2*project_files.size()];
	vector<fs::path> strs;		// push paths so that we do not need to allocate/deallocate c strings
	vector<string>   labels;

	for (unsigned int i=0; i<project_files.size(); i++) {
		char c[] = {(char)('a' + i), 0};
		labels.push_back(string(c));
		menu_items[2*i] = labels[i].c_str();
		strs.push_back(project_files[i].stem());
		menu_items[2*i+1] = strs[i].c_str();
	}

	int ret = dialog_menu(
	"Choose a project",
	"",
	0, 0,
	0,
	project_files.size(),
	(char **) menu_items
	);

	if (ret >= 0) {
		bridge_close_all_windows();
	}

	bridge_draw_all();
}
