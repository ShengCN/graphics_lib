#pragma once
#include <QFileInfo>
#include <random>
#include <chrono>
#include <sstream>
#include <iostream>
#include "graphics_lib/common.h"
#include "Logger.h"

typedef std::chrono::high_resolution_clock Clock;

inline bool file_exists(QString file) {
	QFileInfo check_file(file);
	// check if file exists and if yes: Is it really a file and no directory?
	if(!check_file.exists()) {
		LOG_FAIL("Cannot find file: " + check_file.absoluteFilePath().toStdString());
		return false;
	}

	if(!check_file.isFile()) {
		LOG_FAIL("The required path" + file.toStdString() + "is not a file");
		return false;
	}

	return true;
}

inline bool check_file_extension(QString file, QString ext) {
	QFileInfo qfile(file);
	return qfile.suffix() == ext;
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

inline float deg2quat(pd::deg d){
	return std::cos(deg2rad(d / 2));
}

inline float random_float(float fmin=0.0f, float fmax=1.0f){
	// engine
	std::random_device rd;
	std::mt19937 engine(rd());
	std::uniform_real_distribution<float> u(fmin, fmax);
	return u(engine);
}

inline int random_int(int min = 0, int max = 10) {
	// engine
	std::random_device rd;
	std::mt19937 engine(rd());
	std::uniform_int_distribution<> dis(min, max);

	return dis(engine);
}

template<typename T>
T lerp(T a, T b, float fract) {
	return (1.0f - fract) * a + fract * b;
}

class timer {
public:
	timer() = default;
	~timer() {};

	void tic() {
		_is_ticed = true;
		_tic = Clock::now();
	}
	void toc() {
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

	std::string to_string() {
		if(!_is_ticed) {
			std::cerr << "timer has not been ticed. \n";
		}
		std::stringstream oss;
		oss << get_elapse() * 1e-9;
		return oss.str();
	}

private:
	bool _is_ticed = false;
	std::chrono::time_point<std::chrono::steady_clock> _tic;
	std::chrono::time_point<std::chrono::steady_clock> _toc;
};

inline vec3 bary_centric_interpolate(vec3 a, vec3 b, vec3 c, vec3 bary_coord) {
	return a * bary_coord.x + b * bary_coord.y + c * bary_coord.z;
}

inline bool float_equal(float a, float b, float eps=1e-7) {
	return std::abs(a - b) < eps;
}

inline std::ostream& operator<<(std::ostream& out, vec3 v) {
	out << "(" << v.x << "," << v.y << "," << v.z << ")";
	return out;
}

inline std::string vec3_2_string(vec3 v) {
	std::stringstream out;
	out << "(" << v.x << "," << v.y << "," << v.z << ")";
	return out.str();
}

inline bool same_point(const vec3 &a, const vec3 &b) {
	return glm::distance(a, b) < 1e-3;
}
