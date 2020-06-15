#include "PPC.h"
#include <sstream>
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include "graphics_lib/Utilities/Utils.h"

using namespace glm;
using namespace purdue;
float ppc::GetFocal() {
	return static_cast<float>(_width / 2) / tan(_fov / 2.0f);
}

ppc::ppc(int w, int h, float fov, float p_near, float p_far) :
	_width(w),
	_height(h),
	_fov(fov),
	_near(p_near),
	_far(p_far),
	_position(0.0f, 0.35f, 1.3f), 
	_worldUp(0.0f, 1.0f, 0.0f) {
}

ppc::~ppc()
{
}

vec3 ppc::GetRight()
{
	vec3 view = GetViewVec();

	return cross(view, _worldUp);
}

vec3 ppc::GetUp()
{
	return cross(GetRight(), GetViewVec());
}

void ppc::PositionAndOrient(vec3 p, vec3 lookatP, vec3 up)
{
	_position = p;
	_front = glm::normalize(lookatP - p);
	_worldUp = up;
}

glm::mat4 ppc::GetP() {
	return glm::perspective(deg2rad(_fov), (float)_width / (float)_height, _near, _far);
}

glm::mat4 ppc::GetV()
{
	return glm::lookAt(_position, _position + _front, _worldUp);
	// return glm::lookAt(_position, target, _worldUp);
}

void ppc::Rotate_Axis(glm::vec3 O, glm::vec3 axis, float angled)
{
	glm::vec3 CO = _position - O;
	auto rot_mat = glm::rotate(deg2rad(angled), normalize(axis));
	CO = vec3(rot_mat * vec4(CO, 0.0f));
	_position = CO + O;

}

void ppc::Zoom(float delta)
{
	_fov += delta;
	_fov = glm::clamp(_fov, 10.0f, 150.0f);
}

void ppc::Keyboard(CameraMovement cm, float speed)
{
	switch (cm)
	{
	case CameraMovement::forward:
		_position += speed * _front;
		break;
	case CameraMovement::backward:

		_position -= speed * _front;
		break;
	case CameraMovement::left:

		_position -= speed * GetRight();
		break;
	case CameraMovement::right:

		_position += speed * GetRight();
		break;
	case CameraMovement::up:
		_position += speed * GetUp();
		break;
	case CameraMovement::down:
		_position -= speed * GetUp();
		break;
	default:
		break;
	}
}

void ppc::pan(double deg)
{
	deg = deg / _width * 10.0f;
	//glm::mat4 rot_y = glm::rotate(deg2rad(deg), glm::vec3(0.0f, 1.0f, 0.0f));
	//_front = glm::vec3(rot_y * glm::vec4(_front, 0.0f));
	vec3 right = GetRight();
	_front += right * float(deg);
	_front = glm::normalize(_front);
}

void ppc::tilt(double deg)
{
	// #TODO
}

void ppc::pitch(double deg)
{
	deg = deg /_height * 10.0f;
	//glm::mat4 rot_x = glm::rotate(deg2rad(deg), glm::vec3(1.0f, 0.0f, 0.0f));
	//_front = glm::vec3(rot_x * glm::vec4(_front, 0.0f));
	vec3 up = GetUp();
	_front += up * float(deg);
	_front = glm::normalize(_front);
}

void ppc::mouse_press(int x, int y) {
	m_last_x = x; m_last_y = y;
	m_last_orientation = _front;
}

void ppc::mouse_release(int x, int y) {
}

void ppc::mouse_move(int x, int y) {
	auto to_sphere=[](int x,int y, int w, int h){
		float x_fract = 1.0 - (float)x / w * 2.0, y_fract = (float)y / h * 2.0 - 1.0;
		float alpha = x_fract * 180.0f - 90.0f, beta = y_fract * 90.0f;
		return vec3(cos(deg2rad(beta)) * cos(deg2rad(alpha)), sin(deg2rad(beta)), cos(deg2rad(beta)) * sin(deg2rad(alpha)));
	};
	vec3 last = to_sphere(m_last_x, m_last_y, _width, _height), cur = to_sphere(x, y, _width, _height);
	vec3 rot_axis = glm::cross(last, cur); rad rot_ang = std::acos(glm::dot(last, cur));
	printf("axis: %s rot angle: %f \n", pd::to_string(rot_axis).c_str(), rot_ang);
	_front = glm::rotate(rot_ang * 0.2f, rot_axis) * m_last_orientation;

	//INFO(pd::to_string(relative));
	//_front = glm::normalize(glm::normalize(m_last_orientation) + relative);
}

std::string ppc::to_string() {
	std::ostringstream oss;
	oss << "w: " << _width << " h: " << _height<< std::endl;
	oss << "look: " << purdue::to_string(_position) << std::endl;
	oss << "at: " << purdue::to_string(_position + _front) << std::endl;
	oss << "up: " << purdue::to_string(GetUp()) << std::endl;
	oss << "fov: " << std::to_string(_fov) << std::endl;

	return oss.str();
}
