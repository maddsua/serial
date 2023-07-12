#include <dir.h>
#include <dirent.h>

#include <regex>
#include <fstream>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "app.hpp"

std::string preparePath(std::string tree) {

	tree = std::regex_replace(tree, std::regex("[\\\\\\/]+"), "\\");

	std::string userdir = std::string(std::getenv("userprofile"));
	if (!userdir.size()) userdir = std::getenv("%HOMEPATH%");

	if (!userdir.size()) {
		return {};
	}

	auto createIfDontexist = [](std::string path) {
		auto dir = opendir(path.c_str());
		if (dir) {
			closedir(dir);
			return true;
		}
		if (mkdir(path.c_str())) return false;
		return true;
	};

	auto hierrarchy = tree.find_first_of('\\');
	while(hierrarchy != std::string::npos) {
		if (!createIfDontexist(userdir + tree.substr(0, hierrarchy))) return {};
		hierrarchy = tree.find_first_of('\\', hierrarchy + 1);
	}

	return userdir + tree;
}

bool saveConfiguration(appData* data) {

	auto filepath = preparePath(CONFIG_SAVE_TREE);
	if (!filepath.size()) return false;

	std::ofstream configFile(filepath.c_str(), std::ios::out | std::ios::binary);
	if (!configFile.is_open()) return false;

	JSON appconfig = {
		{"showTimestamps", data->showTimestamps},
		{"echoInputs", data->echoInputs},
		{"hexMode", data->hexMode},
		{"hexStyleFull", data->hexStyleFull},
		{"specialCharsSupport", data->specialCharsSupport},
		{"sel_speed", data->sel_speed},
		{"sel_port", data->sel_port},
		{"sel_endline", data->sel_endline}
	};

	configFile << appconfig.dump();

	configFile.close();

	return true;
}

bool loadConfiguration(appData* data) {

	bool actionResult = true;
	auto filepath = preparePath(CONFIG_SAVE_TREE);
	if (!filepath.size()) return false;

	std::ifstream configFile(filepath.c_str(), std::ios::in | std::ios::binary);
	if (!configFile.is_open()) return false;

	try {
		
		auto appconfig = JSON::parse(configFile);

		data->showTimestamps = appconfig["showTimestamps"].get<bool>();
		data->echoInputs = appconfig["echoInputs"].get<bool>();
		data->hexMode = appconfig["hexMode"].get<bool>();
		data->hexStyleFull = appconfig["hexStyleFull"].get<bool>();
		data->specialCharsSupport = appconfig["specialCharsSupport"].get<bool>();

		size_t temp;
		temp = appconfig["sel_speed"].get<size_t>();
			if (temp < data->speeds.size()) data->sel_speed = temp;
		temp = appconfig["sel_port"].get<size_t>();
			if (temp < data->ports.size()) data->sel_port = temp;
		temp = appconfig["sel_endline"].get<size_t>();
			if (temp < data->endlines.size()) data->sel_endline = temp;
		
	} catch(...) {
		actionResult = false;
	}

	configFile.close();

	return actionResult;
}