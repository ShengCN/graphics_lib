#pragma once
#include <sstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/iterator_range.hpp>

#include "graphics.h"
#include "Logger.h"
#include <cstdarg>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
namespace fs = boost::filesystem;

namespace purdue {
	inline bool file_exists(const std::string file) {
		return fs::exists(file);
	}

	inline std::string get_file_ext(const std::string file) {
		return fs::extension(file).substr(1);
	}

	inline bool check_file_extension(const std::string file, const std::string ext) {
		return ext == get_file_ext(file);
	}

	inline std::vector<std::string> get_files_from_dir(const std::string dir) {
		using namespace boost::filesystem;
		std::vector<std::string> ret;
		for (auto& entry : boost::make_iterator_range(directory_iterator(dir), {}))
			//std::cout << entry << "\n";
			ret.push_back(entry.path().string());

		return ret;
	}

	// with ext
	inline std::string get_file_basename(const std::string file) {
		fs::path p(file);
		return p.filename().string();
	}

	// without ext
	inline std::string get_file_name(const std::string file) {
		fs::path p(file);
		return p.stem().string();
	}


	inline std::string get_file_abs_path(const std::string file) {
		fs::path p(file);
		if (p.is_absolute()) return p.string();

		return fs::canonical(file).string();
	}

	inline std::string get_file_dir(const std::string file) {
		fs::path p(file);
		return p.parent_path().string();
	}

	inline void safe_create_folder(const std::string folder) {
		fs::path p(folder);
		if (!fs::exists(folder))
			fs::create_directory(folder);
	}

	inline std::string get_prefix(int iter, const std::string fmt) {
		char buff[100];
		snprintf(buff, sizeof(buff), fmt.c_str(), iter);
		std::string buffAsStdStr = buff;
		return buffAsStdStr;
	}

	bool save_image(const std::string fname, unsigned int *pixels, int w, int h, int c = 4);

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
			if (!_is_ticed) {
				std::cerr << "timer has not been ticed. \n";
			}
			std::stringstream oss;
			oss << get_elapse() * 1e-9;
			return oss.str();
		}

	private:
		bool _is_ticed = false;
		std::chrono::time_point<std::chrono::system_clock> _tic;
		std::chrono::time_point<std::chrono::system_clock> _toc;
	};

	inline vec3 bary_centric_interpolate(vec3 a, vec3 b, vec3 c, vec3 bary_coord) {
		return a * bary_coord.x + b * bary_coord.y + c * bary_coord.z;
	}

	inline bool float_equal(float a, float b, float eps = 1e-7) {
		return std::abs(a - b) < eps;
	}

	inline std::ostream& operator<<(std::ostream& out, vec3 v) {
		out << v.x << "," << v.y << "," << v.z;
		return out;
	}
}

inline std::vector<std::string> split_string(const std::string s, const std::string delimiter = ",") {
	std::vector<std::string> ret;
	std::string tmp = s;
	size_t pos = 0;
	std::string token;
	while ((pos = tmp.find(delimiter)) != std::string::npos) {
		token = tmp.substr(0, pos);
		ret.push_back(token);
		tmp.erase(0, pos + delimiter.length());
	}

	return ret;
}