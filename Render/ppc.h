#pragma once
#include "common.h"

struct ray {
	vec3 ro, rd;
};

/*
 *	Planer Pinhole camera, basic controls.
 */
enum class CameraMovement
{
	forward,
	backward,
	left,
	right,
	up,
	down
};

class ppc
{
public:
	glm::vec3 _position;
	glm::vec3 _front;
	glm::vec3 _up;	// default is (0.0f,1.0f,0.0f)
	float _fov, _near, _far;
	int _width, _height;

	ppc()=default;
	ppc(int w, int h, float fov, float p_near=0.001f, float p_far=100000.0f);
	~ppc();

	bool save(const std::string file);
	bool load(const std::string file);

	CUDA_HOSTDEV
	vec3 GetRight() const {
        vec3 view = GetViewVec();
        return cross(view, _up);
    }
	
    CUDA_HOSTDEV
    vec3 GetUp() const {
        return cross(GetRight(), GetViewVec());
    }

    CUDA_HOSTDEV
	vec3 GetViewVec() const { return _front; }
	
	CUDA_HOSTDEV
	float get_fov() const { return _fov; }

	CUDA_HOSTDEV
	void set_fov(float f) { _fov = f;}
    

	CUDA_HOSTDEV
	void PositionAndOrient(vec3 p, vec3 lookatP, vec3 up){
        _position = p;
        _front = glm::normalize(lookatP - p);
        _up = up;
    }
    
   	CUDA_HOSTDEV
	glm::mat4 GetP() const {
		return glm::perspective(purdue::deg2rad(_fov), (float)_width / (float)_height, _near, _far);
	}

   	CUDA_HOSTDEV
	glm::mat4 GetV() const {
		return glm::lookAt(_position, _position + _front, _up);
	}

	glm::mat3 get_local_mat();
	void Rotate_Axis(glm::vec3 O, glm::vec3 axis, float angled);	// only rotate ppc position
	void Zoom(float delta);
	void Keyboard(CameraMovement cm, float speed); // keyboard reactions
	void scroll(double delta) {
		_fov += float(delta);
	}

	vec3 get_pos() { return _position; }
	void pan(double deg);
	void tilt(double deg);
	void pitch(double deg);
	void mouse_press(int x, int y);
	void mouse_release(int x, int y);
	void mouse_move(int x, int y);
	bool mouse_inside_frame(int x, int y);
	void set_trackball(bool trackball);
	void camera_resize(int w, int h);
	int width() { return _width; }
	int height() { return _height; }

   	CUDA_HOSTDEV
	vec2 project(vec3 p) {
		vec3 a = glm::normalize(GetRight());
		vec3 b = -glm::normalize(GetUp());
		vec3 c = glm::normalize(GetViewVec()) * get_focal() - 0.5f * (float)_width * GetRight() + 0.5f * (float)_height * GetUp();
		mat3 m(a,b,c);
		vec3 pp = glm::inverse(m) * (p-_position);
		return vec2(pp.x/pp.z, _height-pp.y/pp.z);
	}

   	CUDA_HOSTDEV
	ray get_ray(int u, int v) const {
		ray ret;
        float focal = get_focal();
        vec3 right = glm::normalize(GetRight());
        vec3 up = glm::normalize(GetUp());
        vec3 front = glm::normalize(GetViewVec());

        ret.ro = _position;
        int center_u = _width / 2, center_v = _height / 2;

        ret.rd = front * focal + (u - center_u + 0.5f) * right + (v - center_v + 0.5f) * up;
        ret.rd = glm::normalize(ret.rd);
		return ret;
    }

   CUDA_HOSTDEV
    float get_focal() const {
        float rad = 0.5f * _fov /180.0 * 3.1415926;
        return 0.5f * _height/ std::tan(rad);
    }

	std::string to_string();
private:
	float GetFocal();

private:
	int m_last_x, m_last_y;
	vec3 m_last_orientation, m_last_position;
	bool m_pressed;
	bool m_trackball;
};
