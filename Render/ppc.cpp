#include "PPC.h"
#include <sstream>
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include "Utils.h"
#include "global_variable.h"

using namespace glm;


float ppc::GetFocal()
{
	auto gv = global_variable::instance();
	return static_cast<float>(gv->width / 2) / tan(_fov / 2.0f);
}

ppc::ppc(float _fov, float _aspect, float p_near, float p_far) :
	_fov(_fov), 
	_aspect(_aspect),
	_near(p_near),
	_far(p_far),
	_position(0.0f, 0.35f, 1.3f), 
	_worldUp(0.0f, 1.0f, 0.0f)
{
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

glm::mat4 ppc::GetP()
{
	return glm::perspective(deg2rad(_fov), _aspect, _near, _far);
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
	_fov = clamp(_fov, 10.0f, 150.0f);
}

void ppc::Keyboard(CameraMovement cm, float deltaTime)
{
	auto gv = global_variable::instance();
	float moving_speed = gv->is_speed_up ? 40.0f : 1.0f;

	switch (cm)
	{
	case CameraMovement::forward:
		_position += moving_speed * deltaTime * _front;
		break;
	case CameraMovement::backward:

		_position -= moving_speed * deltaTime * _front;
		break;
	case CameraMovement::left:

		_position -= moving_speed * deltaTime * GetRight();
		break;
	case CameraMovement::right:

		_position += moving_speed * deltaTime * GetRight();
		break;
	case CameraMovement::up:
		_position += moving_speed * deltaTime * GetUp();
		break;
	case CameraMovement::down:
		_position -= moving_speed * deltaTime * GetUp();
		break;
	default:
		break;
	}
}

void ppc::pan(double deg)
{
	auto gv = global_variable::instance();
	deg = deg / gv->width * 10.0f;
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
	auto gv = global_variable::instance();
	deg = deg / gv->height * 10.0f;
	//glm::mat4 rot_x = glm::rotate(deg2rad(deg), glm::vec3(1.0f, 0.0f, 0.0f));
	//_front = glm::vec3(rot_x * glm::vec4(_front, 0.0f));
	vec3 up = GetUp();
	_front += up * float(deg);
	_front = glm::normalize(_front);
}

void ppc::write_xml_element(QXmlStreamWriter& xml) {
	xml.writeStartElement("sensor");
	xml.writeAttribute("type", "perspective");
	
	mitsuba_xml::write_map_element(xml,
							   "string",
							   { {"fovAxis",  "smaller"} });
	
	mitsuba_xml::write_map_element(xml,
							   "float",
							   { {"nearClip", std::to_string(_near)} });

	mitsuba_xml::write_map_element(xml,
								   "float",
								   { {"farClip", std::to_string(_far)} });

	//#todo_camera_focus
	//mitsuba_xml::write_map_element(xml,
	//							   "float",
	//							   { {"focusDistance", std::to_string(_near)} });

	mitsuba_xml::write_map_element(xml,
								   "float",
								   { {"fov", std::to_string(_fov)} });

	// transform
	auto vec3_string = [](glm::vec3 v){
		std::ostringstream oss;
		oss << v.x << "," << v.y << "," << v.z;
		return oss.str();
	};

	xml.writeStartElement("transform");
	xml.writeAttribute("name", "toWorld");
	xml.writeStartElement("lookAt");
	xml.writeAttribute("origin", QString::fromStdString(vec3_string(_position)));
	xml.writeAttribute("target", QString::fromStdString(vec3_string(_position + _front)));
	xml.writeAttribute("up", QString::fromStdString(vec3_string(GetUp())));
	xml.writeEndElement();
	xml.writeEndElement();

	// sampler
	xml.writeStartElement("sampler");
	xml.writeAttribute("type", "ldsampler");
	mitsuba_xml::write_map_element(xml,
								   "integer",
								   { {"sampleCount", std::to_string(global_variable::instance()->sample_count)} });
	xml.writeEndElement();
	
	// film
	xml.writeStartElement("film");
	xml.writeAttribute("type", "hdrfilm");
	mitsuba_xml::write_map_element(xml,
								   "integer",
								   { {"width", std::to_string(global_variable::instance()->mts_width)} });
	mitsuba_xml::write_map_element(xml,
								   "integer",
								   { {"height", std::to_string(global_variable::instance()->mts_height)} });
	xml.writeStartElement("rfilter");
	xml.writeAttribute("type", "gaussian");
	xml.writeEndElement();	// rfilter

	xml.writeEndElement();	// film

	xml.writeEndElement();
}
