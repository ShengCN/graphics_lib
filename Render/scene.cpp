#include <memory>
#include <QFileInfo>

#include "scene.h"
#include "Utilities/Logger.h"
#include "global_variable.h"
#include "Utilities/model_loader.h"

scene::scene() {
}


scene::~scene() {
}

void scene::load_scene(QString scene_file) {
	auto gv = global_variable::instance();

	//#todo_parse_ppc
	vec3 camera_pos;
	vec3 meshes_center = scene_center();
	if(m_meshes.empty()){
		camera_pos = vec3(0.0f, 5.0f, 10.0f);
	} else {
		float mesh_length = scene_aabb().diag_length();
		camera_pos = meshes_center + vec3(0.0f, 0.3f * mesh_length, mesh_length);
	}

	float aspect = (float)gv->width / gv->height;
	m_camera = std::make_shared<ppc>(80.0f, aspect);
	m_camera->PositionAndOrient(camera_pos, meshes_center, vec3(0.0f, 1.0f, 0.0f));

	m_last_ppc = m_camera;
	m_new_ppc = m_camera;
}

bool scene::reload_shaders() {
	bool success = true;
	for (auto& m : m_meshes) {
		success &= m->reload_shaders();
	}

	return success;
}

void scene::draw_scene(int iter) {
	if (m_camera == nullptr) {
		LOG_FAIL("Camera initialized");
		// assert(false);
		return;
	}

	auto gv = global_variable::instance();
	auto scale_compute = [](float x) {
		return (40.0f * x) + 1.0f;
	};

	for(auto m:m_meshes){
		m->draw(m_camera, iter);
	}
}

void scene::clean_up() {
}

vec3 scene::scene_center() {
	vec3 center(0.0f,0.0f,0.0f);
	float weight = (float)m_meshes.size();
	for (auto&m : m_meshes) {
		center += m->compute_world_center() * weight;
	}
	return center;
}

//void scene::load_stl(QString file_path, vec3 color, bool is_container/*=false*/) {
//	auto gv = global_variable::instance();
//
//	std::shared_ptr<mesh> mesh = std::make_shared<triangle_mesh>(gv->template_vs, gv->template_fs);
//	mesh->load(file_path);
//	mesh->set_color(color);
//	mesh->create_ogl_buffers();
//
//	m_meshes.push_back(mesh);
//}

void scene::load_mesh(QString file_path) {
	QFileInfo fi(file_path);
	qDebug() << "File name: " << fi.fileName();
	qDebug() << "Base name: " << fi.baseName();
	qDebug() << "Extension name: " << fi.completeSuffix();
	qDebug() << "File path name: " << fi.path();

	std::shared_ptr<model_loader> loader_func;
	if(fi.completeSuffix() == "obj"){
		loader_func = model_loader::create(model_type::obj);
	} else {
		loader_func = model_loader::create(model_type::fbx);
	}

	std::shared_ptr<mesh> mesh;
	if(loader_func->load_model(file_path, mesh)){
		assert(mesh->triangle_num() > 0);
		mesh->normalize_to_origin();
		m_meshes.push_back(mesh);
	}
}

AABB scene::scene_aabb() {
	AABB scene_aabb(vec3(0.0f));

	if(!m_meshes.empty()){
		scene_aabb = m_meshes[0]->compute_world_aabb();
	}

	for(auto m:m_meshes){
		AABB aabb = m->compute_world_aabb();
		scene_aabb.add_point(aabb.p0);
		scene_aabb.add_point(aabb.p1);
	}
	return scene_aabb;
}

bool scene::save_scene(const QString filename) {
	// todo 
	return false;
}

void scene::mesh_selected(int id) {
	for(auto m:m_meshes){
		if(m->get_id() == id){
			m->set_select(true);
		} else {
			m->set_select(false);
		}
	}
}

void scene::set_mesh_transform(int mesh_id, glm::mat4 new_transform) {
	for(auto m:m_meshes){
		if(m->get_id() == mesh_id){
			m->set_world_mat(new_transform);
		}
	}
}

void scene::mouse_pressed(int x, int y) {
	m_is_pressed = true;
	m_last_x = x; m_last_y = y;
	m_new_ppc = std::make_shared<ppc>(0.0f,0.0f);
	
	// deep copy
	*m_new_ppc = *m_last_ppc;
}

void scene::mouse_released(int x, int y) {
	m_is_pressed = false;

	// update m_last_ppc
	m_camera = m_last_ppc = m_new_ppc;
}

void scene::mouse_movement(int x, int y) {
	if (!m_is_pressed)
		return;

	auto gv = global_variable::instance();
	int offset_x = x - m_last_x, offset_y = y - m_last_y;
	float x_fract = (float)offset_x / gv->width, y_fract = (float)offset_y / gv->height;

	float sensitiy = 2.0f;
	m_new_ppc->_front = m_last_ppc->_front +
		m_last_ppc->GetRight() * sensitiy * x_fract -
		m_last_ppc->GetUp() * sensitiy * y_fract;

	m_new_ppc->_front = glm::normalize(m_new_ppc->_front);
	qDebug() << x_fract << " " << y_fract;

	m_camera = m_new_ppc;
}

void scene::key_pressed(int k, bool is_shift) {
	float delta = is_shift? 2.0f: 1.0f;
	/*qDebug() << "Pressed key: " << k;*/

	switch (k) {
	case Qt::Key_W:{
			qDebug() << "Pressed key: w";
			m_camera->Keyboard(CameraMovement::forward, delta);
			break;
		}
	case Qt::Key_S: {
			qDebug() << "Pressed key: s";
			m_camera->Keyboard(CameraMovement::backward, delta);
			break;
		}
	case Qt::Key_A: {
			qDebug() << "Pressed key: a";
			m_camera->Keyboard(CameraMovement::left, delta);
			break;
		}
	case Qt::Key_D: {
			qDebug() << "Pressed key: d";
			m_camera->Keyboard(CameraMovement::right, delta);
			break;
		}
	case Qt::Key_Q:{
			qDebug() << "Pressed key: q";
			m_camera->Keyboard(CameraMovement::up, delta);
			break;
		}
	case Qt::Key_E:
		{
			qDebug() << "Pressed key: e";
			m_camera->Keyboard(CameraMovement::down, delta);
			break;
		}
	default:
		break;
	}
}
