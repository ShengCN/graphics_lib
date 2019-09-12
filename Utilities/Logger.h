#pragma once
#include <iostream>
#include <sstream>
#include "global_variable.h"

class logger
{
public:
	logger() = default;
	~logger() {};

	template<typename T, typename TT, typename TTT>
	static void log_fail(const std::string info, T file, TT line, TTT func) {
		std::stringstream os;
		os << info << " failed\n";
		os << "File: " << file << "\n"
			<< "Line: "<< line << "\n"
			<< "Func: "<< func << std::endl;

		if(global_variable::instance()->verbose)
			std::cerr << os.str();

		//#todo_log_files
	}

	template<typename T>
	static void log(const T info) {
		std::stringstream oss;
		oss << info << std::endl;

		if (global_variable::instance()->verbose)
			std::cerr << oss.str();
	}

private:
};

#ifndef LOG_FAIL
#define LOG_FAIL(s) logger::log_fail(s, __FILE__, __LINE__, __FUNCTION__);
#endif


#ifndef LOG
#define LOG(s) logger::log(s, __FILE__, __LINE__, __FUNCTION__);
#endif