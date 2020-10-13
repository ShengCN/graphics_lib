#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include "signed_distance_field.h"`

signed_distance_field::signed_distance_field(int w, int h, int z):m_w(w), m_h(h), m_z(z){
	field_buffer.resize(w * h * z);
}

void signed_distance_field::construct(std::shared_ptr<mesh> m) {

}

float signed_distance_field::query(glm::vec3 p) {

}
