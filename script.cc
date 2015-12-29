#include "script.h"
#include "precompiled_headers.h"
#include "init.h"

#ifdef COMPILE_IN_CHAI_STDLIB

#	include <chaiscript/chaiscript_stdlib.hpp>
	chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());

#else

	chaiscript::ChaiScript chai;

#endif

using namespace std;
namespace fs = boost::filesystem;
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
using namespace boost::log::trivial;

typedef vector<fs::path> filevec;

filevec project_files;


static string cpp_bridge_get_output_from_mark() {
	// the terminaloutput allocates an array of char *, need to convert it to string.
	// hmm, looks that it would be nice to rewrite the whole dvtm to c++/stl
	TerminalOutput r = bridge_get_output_from_mark();
	string ret = string(r.buf, r.bufsize);

	// free the temporary buffer
	free(r.buf);
	return ret;
}

static void cpp_bridge_insert_input_string(const string &s) {
	 bridge_insert_input_string(s.c_str(), s.length());
}

static void bridge_delay(int ms) {
	for (int i=0; i<ms; i++) {
		process_input(1);
	}
}

static void bridge_log(string _severity, string s) {
	src::severity_logger< severity_level > slg;
	if (_severity == "trace") {
		BOOST_LOG_SEV(slg, severity_level::trace) << s;
	} else if(_severity == "debug") {
		BOOST_LOG_SEV(slg, debug) << s;
	} else if(_severity == "info") {
		BOOST_LOG_SEV(slg, info) << s;
	} else if(_severity == "warning") {
		BOOST_LOG_SEV(slg, warning) << s;
	} else if(_severity == "error") {
		BOOST_LOG_SEV(slg, error) << s;
	} else if(_severity == "fatal") {
		BOOST_LOG_SEV(slg, fatal) << s;
	}
}

void initialize_scripting() {
	// logging: TODO: make this customizable ...
	boost::log::add_common_attributes();
	logging::add_file_log(
//			keywords::file_name = "/tmp/dvtm_%Y-%m-%d_%H-%M-%S.%N.log",
			keywords::file_name = "/tmp/dvtm_%N.log",
	        keywords::rotation_size = 10 * 1024 * 1024,
	        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
	        //keywords::format = "[%TimeStamp%] (%LineID%) <%Severity%>: %Message%"
			keywords::format =
			        (
			            expr::stream
							// core dumps on finish so commented out for now ...
							// << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
							// << ": "
			                << expr::attr< unsigned int >("LineID")
			                << ": <" << logging::trivial::severity
			                << "> " << expr::smessage
			        ),
			keywords::auto_flush = true
	);

	chai.add(chaiscript::fun(&bridge_create_window), "create_window");
	chai.add(chaiscript::fun(&bridge_mark), "dvtm_mark");
	chai.add(chaiscript::fun(&cpp_bridge_get_output_from_mark), "dvtm_get_output_from_mark");
	chai.add(chaiscript::fun(&cpp_bridge_insert_input_string), "dvtm_insert_input_string");
	chai.add(chaiscript::fun(&bridge_delay), "dvtm_delay");
	chai.add(chaiscript::fun(&bridge_log), "log");
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
