#pragma once
#include <random>

#include <sstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/iterator_range.hpp>

#include "graphics_lib/common.h"
#include "Logger.h"

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

	bool save_image(const std::string fname, unsigned int *pixels, int w, int h, int c = 4);

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

	inline float deg2quat(pd::deg d) {
		return std::cos(deg2rad(d / 2));
	}

	inline float random_float(float fmin = 0.0f, float fmax = 1.0f) {
		// engine
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_real_distribution<float> u(fmin, fmax);
		return u(engine);
	}

	inline int random_int(int mi = 0, int ma = 10) {
		// engine
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_int_distribution<> dis(mi, ma);

		return dis(engine);
	}

	float normal_random(float mean, float sig);

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
			if (!_is_ticed) {
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

	inline bool float_equal(float a, float b, float eps = 1e-7) {
		return std::abs(a - b) < eps;
	}

	inline std::ostream& operator<<(std::ostream& out, vec3 v) {
		out << v.x << "," << v.y << "," << v.z;
		return out;
	}

	inline std::string to_string(vec3 v) {
		std::stringstream out;
		out << v.x << "," << v.y << "," << v.z;
		return out.str();
	}

	inline std::string to_string(vec4 v) {
		std::stringstream out;
		out << v.x << "," << v.y << "," << v.z << "," << v.w;
		return out.str();
	}

	inline std::string to_string(mat4 m) {
		std::stringstream out;
		out << to_string(m[0]) << std::endl;
		out << to_string(m[1]) << std::endl;
		out << to_string(m[2]) << std::endl;
		out << to_string(m[3]);
		return out.str();
	}

	inline bool same_point(const vec3 &a, const vec3 &b) {
		return glm::distance(a, b) < 1e-3;
	}

}
