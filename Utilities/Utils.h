#pragma once
#include <QFileInfo>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

#include "global_variable.h"

inline bool file_exists(QString file) {
	QFileInfo check_file(file);
	// check if file exists and if yes: Is it really a file and no directory?
	return (check_file.exists() && check_file.isFile());
}

inline bool check_file_extension(QString file, QString ext) {
	QFileInfo qfile(file);
	return qfile.suffix() == ext;
}

// check if the file has ext, if not, add it
inline QString add_suffix(QString file, QString ext){
	QString ret;
	if(!check_file_extension(file, ext)){
		ret = file + "." + ext;
	}
	else
		ret = file;

	return ret;
}

/*!
 * 
 * \brief Math lab
 *
 * \author YichenSheng
 * \date August 2019
 */
inline pd::rad deg2rad(pd::deg d) {
	return d / 180.0f * pd::pi;
}

inline pd::deg rad2deg(pd::rad r) {
	return r / pd::pi * 180.0f;
}

class timer{
public:
	timer() = default;
	~timer() {};

	void tic() {
		_is_ticed = true;
		_tic = Clock::now();
	}
	void tok() {
		_toc = Clock::now();
	}

	long long get_elapse() {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(_toc - _tic).count();
	}

	void print_elapsed() {
		if (!_is_ticed) {
			std::cerr << "timer has not been ticed \n";
			return;
		}

		auto elapsed = get_elapse();
		std::cerr << "Time: " << elapsed * 1e-9 << " seconds \n";
	}
private:
	bool _is_ticed = false;
	std::chrono::time_point<std::chrono::steady_clock> _tic;
	std::chrono::time_point<std::chrono::steady_clock> _toc;
};