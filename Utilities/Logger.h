#pragma once
#include <iostream>
#include <sstream>

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#ifndef VERBOSE
#define VERBOSE 1 
#endif

class logger {
public:
	~logger() {};

	static logger* instance() {
		if (!m_instance)
			m_instance = new logger();

		return m_instance;
	}

	template<typename T, typename TT, typename TTT>
	void dbg(const std::string s, T file, TT line, TTT func) {
		std::string log_str = fmt::format("[DBG] {:50} \t [{}-{}-{}]\n", s, func, line, file);
		m_log_str += log_str;
        if (VERBOSE)
            std::cout << log_str;
	}

	template<typename T, typename TT, typename TTT>
	void info(const std::string s, T file, TT line, TTT func) {
		std::string log_str = fmt::format("[INFO] {:50} \t [{}-{}-{}]\n", s, func, line, file);
		m_log_str += log_str;
        std::cout << log_str;
	}

	template<typename T, typename TT, typename TTT>
	void warn(const std::string s, T file, TT line, TTT func) {
		std::string log_str = fmt::format("[WARN] {:50} \t [{}-{}-{}]\n", s, func, line, file);
		m_log_str += log_str;
		std::cout << log_str;
	}

	template<typename T, typename TT>
	void error(const std::string s, T file, TT line, T func) {
		std::string log_str = fmt::format("[ERROR] {:50} \t [{}-{}-{}]\n", s, func, line, file);
		m_log_str += log_str;
		std::cerr << log_str;
	}

	std::string get_log() { return m_log_str; }

private:
	std::string m_log_str;
	static logger *m_instance;
};

#ifndef DBG 
#define DBG(fmt_str, ...) logger::instance()->dbg(fmt::format(fmt_str, ##__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__)
#endif // !INFO

#ifndef INFO
#define INFO(fmt_str, ...) logger::instance()->info(fmt::format(fmt_str, ##__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__)
#endif // !INFO


#ifndef WARN
#define WARN(fmt_str, ...) logger::instance()->warn(fmt::format(fmt_str, ##__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__)
#endif // !WARN

#ifndef ERROR 
#define ERROR(fmt_str, ...) logger::instance()->error(fmt::format(fmt_str, ##__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__)
#endif // !ERROR

#ifndef FAIL
#define FAIL(condition, fmt_str, ...) do { if ((condition)) {ERROR("{} {} {}", __FILE__, __LINE__, __FUNCTION__); throw std::invalid_argument(fmt::format(fmt_str, ##__VA_ARGS__)); }} while(0)
#endif
