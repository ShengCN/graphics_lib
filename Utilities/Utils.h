#pragma once
#include <common.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/iterator_range.hpp>

#include "Logger.h"
#include <cstdarg>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
namespace fs = boost::filesystem;

#ifdef __CUDACC__
#define CUDA_HOSTDEV __host__ __device__
#else
#define CUDA_HOSTDEV
#endif

namespace purdue {
	typedef float deg;
	typedef float rad;
	constexpr float pi = 3.14159265f;

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

	inline std::vector<std::string> get_files_from_dir(std::string const dir, std::string const ext) {
		using namespace boost::filesystem;
		std::vector<std::string> ret;
        if (fs::exists(dir) && fs::is_directory(dir)) {
            for (auto const & entry : fs::recursive_directory_iterator(dir)) {
                if (fs::is_regular_file(entry) && entry.path().extension() == ext)
                    ret.push_back(entry.path().string());
            }
        }
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
        ret.push_back(tmp);
        return ret;
    }

	bool read_image(const std::string fname, int &w, int &h, int &c, std::vector<unsigned char> &buf);
	bool save_image(const std::string fname, unsigned int *pixels, int w, int h, int c = 4);

    CUDA_HOSTDEV
	inline rad deg2rad(deg d) {
		return d / 180.0f * pi;
	}

    CUDA_HOSTDEV
	inline deg rad2deg(rad r) {
		return r / pi * 180.0f;
	}

	inline float deg2quat(deg d) {
		return std::cos(deg2rad(d / 2));
	}

	template<typename T>
	inline T clamp(T v, T m, T mm) {
		v = std::max(v, m);
		v = std::min(v, mm);
		return v;
	}
	
	inline std::string to_string(glm::vec3 v) {
		std::stringstream out;
		out << v.x << "," << v.y << "," << v.z;
		return out.str();
	}

	inline std::string to_string(glm::vec4 v) {
		std::stringstream out;
		out << v.x << "," << v.y << "," << v.z << "," << v.w;
		return out.str();
	}

	inline std::string to_string(glm::mat4 m) {
		std::stringstream out;
		out << to_string(m[0]) << ",";
		out << to_string(m[1]) << ",";
		out << to_string(m[2]) << ",";
		out << to_string(m[3]);
		return out.str();
	}

	inline glm::vec3 string_vec3(std::string s) {
        glm::vec3 v(0.0f);
        std::vector<std::string> strings = split_string(s, ",");
        FAIL(strings.size() != 3, "{}(size: {}) cannot be converted to vec3", s, strings.size());
        v[0] = std::stof(strings[0]);
        v[1] = std::stof(strings[1]);
        v[2] = std::stof(strings[2]);
        return v;
	}

	inline glm::vec4 string_vec4(std::string s) {
        glm::vec4 v(0.0f);
        std::vector<std::string> strings = split_string(s, ",");
        FAIL(strings.size() != 4, "{}(size: {}) cannot be converted to vec4", s, strings.size());
        v[0] = std::stof(strings[0]);
        v[1] = std::stof(strings[1]);
        v[2] = std::stof(strings[2]);
        v[3] = std::stof(strings[3]);
        return v;
	}

	inline glm::mat4 string_mat4(std::string s) {
        glm::mat4 m(1.0f);
        std::vector<std::string> strings = split_string(s, ",");
        FAIL(strings.size() != 16, "{}/(size: {}) cannot be converted to mat4", s, strings.size());
        for (int i =0; i < 4; ++i) for(int j = 0; j < 4; ++j) {
            m[i][j] = std::stof(strings[i * 4 + j]);
        }
        return m;
	}

	inline bool same_point(const glm::vec3 &a, const glm::vec3 &b) {
		return glm::distance(a, b) < 1e-3;
	}


    static std::mt19937 engine(19920208);
	inline float random_float(float fmin = 0.0f, float fmax = 1.0f) {
		// engine
		//std::random_device rd;
		//std::mt19937 engine(rd());
		std::uniform_real_distribution<float> u(fmin, fmax);
		return u(engine);
	}

	inline int random_int(int mi = 0, int ma = 10) {
		// engine
		//std::random_device rd;
		//std::mt19937 engine(rd());
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

		void print_elapsed(const std::string log=""); 

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

	inline glm::vec3 bary_centric_interpolate(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 bary_coord) {
		return a * bary_coord.x + b * bary_coord.y + c * bary_coord.z;
	}

	inline bool float_equal(float a, float b, float eps = 1e-7) {
		return std::abs(a - b) < eps;
	}

	inline std::ostream& operator<<(std::ostream& out, glm::vec3 v) {
		out << v.x << "," << v.y << "," << v.z;
		return out;
	}

}

#define TIME(x, log) do { purdue::timer clc; clc.tic(); x; clc.toc(); clc.print_elapsed(log); } while(0)

namespace pd = purdue;
