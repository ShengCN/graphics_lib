#include <glad/glad.h>
#include <common.h>
#include <memory>
#include <stdexcept>
#include "fmt/core.h"
#include "render_engine.h"
#include "Utilities/Utils.h"

render_engine::render_engine() {
	m_draw_render = true;
	m_draw_visualize = false;
}

void render_engine::test_scene(int w, int h) {
	const std::string test_mesh_fpath = "Meshes/bunny.obj";
	mesh_id id = add_mesh(test_mesh_fpath);
	INFO("finish loading mesh buny");

	set_mesh_color(id, vec3(0.8f));
	auto cur_mesh = get_mesh(id);
	if(!cur_mesh) {
		throw std::invalid_argument(fmt::format("Testing Scene Cannnot find {}", test_mesh_fpath)); 
	} else {
		cur_mesh->normalize_position_orientation();
		cur_mesh->add_rotate(90.0f, vec3(-1.0f,0.0f,0.0f));
	}

	m_manager.cur_camera = std::make_shared<ppc>(w, h, 80.0f);
	look_at(id);
}

void render_engine::init_camera(int w, int h, float fov) {
	m_manager.cur_camera = std::make_shared<ppc>(w, h, fov);
}

void render_engine::recompute_normal(int mesh_id) {
	get_mesh(mesh_id)->recompute_normal();
}

void render_engine::stand_on_plane(int mesh_id, vec3 p, vec3 n) {
	auto mesh_ptr = get_mesh(mesh_id);
	if(mesh_ptr == nullptr) return;

	AABB world_aabb = mesh_ptr->compute_world_aabb();
	vec3 normalized_n = glm::normalize(n);
	glm::vec3 trans_vec = -glm::dot(world_aabb.p0 - p, normalized_n) * normalized_n;
	
	mesh_ptr->add_world_transate(trans_vec); 
}

mesh_id render_engine::add_visualize_line(vec3 h, vec3 t) {
	std::shared_ptr<mesh> vis_mesh = std::make_shared<mesh>();
	if (vis_mesh == nullptr) {
		throw std::invalid_argument(fmt::format("New mesh failed"));
		return -1;
	}

	vis_mesh->add_vertex(h, vec3(0.0f), vec3(1.0f,0.0f,0.0f)); 
	vis_mesh->add_vertex(t, vec3(0.0f), vec3(1.0f,0.0f,0.0f)); 
	
	m_manager.rendering_mappings[vis_mesh] = m_manager.shaders.at(default_shader_name);
	m_manager.visualize_scene->add_mesh(vis_mesh);	
	return vis_mesh->get_id();
}

bool render_engine::remove_visualize_line(mesh_id id) {
	return m_manager.render_scene->remove_mesh(id);	
}

void render_engine::render(int frame) {	
	rendering_params params = { m_manager.cur_camera, m_manager.lights, frame, draw_type::triangle};

	if (m_draw_render) {
		render_scene(m_manager.render_scene, params);
	}
	if (m_draw_visualize) {
		params = { m_manager.cur_camera, m_manager.lights, frame, draw_type::line_segments };
		if (m_vis_frame_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		render_scene(m_manager.visualize_scene, params);
		if (m_vis_frame_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void render_engine::init() {
	//------- initialize rendering states --------//
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	//------- initialize Shaders --------//
	const std::string template_vs = "Shaders/template_vs.glsl";
	const std::string template_fs = "Shaders/template_fs.glsl";
	m_manager.shaders["template"] = std::make_shared<shader>(template_vs.c_str(), template_fs.c_str());

	const std::string quad_vs = "Shaders/quad_vs.glsl";
	const std::string quad_fs = "Shaders/quad_fs.glsl";
	m_manager.shaders["quad"] = std::make_shared<quad_shader>(quad_vs.c_str(), quad_fs.c_str());
	INFO("{} shaders finished", (int)m_manager.shaders.size());

	// const std::string weighted_OIT_vs = "Shaders/transparent_vs.glsl";
	// const std::string weighted_OIT_fs = "Shaders/transparent_fs.glsl";
	// m_manager.shaders["weight_OIT"] = std::make_shared<shader>(weighted_OIT_vs.c_str(), weighted_OIT_fs.c_str());

	//------- initialize scene --------//
	m_manager.render_scene = std::make_shared<scene>();
	m_manager.visualize_scene = std::make_shared<scene>();
}

void render_engine::render_scene(std::shared_ptr<scene> cur_scene, rendering_params params) {
	// shader render mesh
	auto meshes = cur_scene->get_meshes();

	for(auto &m:meshes) {
		m_manager.rendering_mappings.at(m.second)->draw_mesh(m.second, params);
	}
}

void render_engine::render_weighted_OIT(std::shared_ptr<scene> cur_scene, rendering_params params) {
	static unsigned int framebuffer = -1;
	static unsigned int accum_texture = -1;
	static unsigned int reveal_texture = -1;
	static unsigned int rbo = -1;
	
	// initialize
	if (framebuffer == -1) {
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glGenTextures(1, &accum_texture);
		glBindTexture(GL_TEXTURE_2D, accum_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_manager.cur_camera->_width, m_manager.cur_camera->_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &reveal_texture);
		glBindTexture(GL_TEXTURE_2D, reveal_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, m_manager.cur_camera->_width, m_manager.cur_camera->_height, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accum_texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, reveal_texture, 0);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			INFO("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// ------------------------ pass 1, render opaque ------------------------ //
	// todo

	// ------------------------ pass 2, render transparent ------------------------ //
	// clear initial values
	// render transparent
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glDepthMask(GL_FALSE);

	glDrawBuffer(GL_COLOR_ATTACHMENT0); 
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, DrawBuffers);
	glEnable(GL_BLEND);
	glBlendFunci(0, GL_ONE, GL_ONE);
	//glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

	render_scene(cur_scene, params);
	glDepthMask(GL_TRUE);
	
	// ------------------------ pass 3, merge results ------------------------ //
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	m_manager.shaders.at("quad")->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, accum_texture);
	glUniform1i(glGetUniformLocation(m_manager.shaders.at("quad")->get_shader_program(), "accum_tex"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, reveal_texture);
	glUniform1i(glGetUniformLocation(m_manager.shaders.at("quad")->get_shader_program(), "weight_tex"), 1);

	glViewport(0, 0, m_manager.cur_camera->_width, m_manager.cur_camera->_height);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

	// draw_quad();
}

std::shared_ptr<mesh> render_engine::get_mesh(mesh_id id) {
	return m_manager.render_scene->get_mesh(id);
}

int render_engine::add_mesh(const std::string model_fname, vec3 c) {
	auto cur_mesh = m_manager.render_scene->add_mesh(model_fname, c);

	if (cur_mesh) {
		m_manager.rendering_mappings[cur_mesh] = m_manager.shaders[default_shader_name];
		return cur_mesh->get_id();
	}

	return -1;
}

bool render_engine::save_mesh(mesh_id id, const std::string model_fname) {
	return false;
}

bool render_engine::load_render_scene(const std::string scene_file) {
	return false;
}

bool render_engine::save_framebuffer(const std::string ofname) {
	unsigned int *pixels;
	int w = m_manager.cur_camera->width(), h = m_manager.cur_camera->height();
	pixels = new unsigned int[w * h * 4];
	for (int i = 0; i < (w * h * 4); i++) {
		pixels[i] = 0;
	}

	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// flip pixels
	for (int j = 0; j < h / 2; ++j) for (int i = 0; i < w; ++i) {
		std::swap(pixels[w * j + i], pixels[w * (h-1-j) + i]);
	}

	bool ret = purdue::save_image(ofname, pixels, w, h, 4);
	delete[] pixels;
	return ret;
}

bool render_engine::reload_shaders() {
	bool success = true;
	for(auto &s:m_manager.shaders) {
		success = success && s.second->reload_shader();
	}
	return success;
}

void render_engine::camera_press(int x, int y) {
	m_manager.cur_camera->mouse_press(x, y);
}

void render_engine::camera_release(int x, int y) {
	m_manager.cur_camera->mouse_release(x, y);
}

void render_engine::camera_move(int x, int y) {
	m_manager.cur_camera->mouse_move(x, y);
}

void render_engine::camera_scroll(int offset) {
	m_manager.cur_camera->scroll((double)offset);
}

void render_engine::camera_keyboard(char m, bool shift) {
	float speed = 1.0f * 0.1f;
	if(shift) 
		speed *= 10.0f;
	
	switch (m)
	{
	case 'w': 
		m_manager.cur_camera->Keyboard(CameraMovement::forward, speed);
		break;
	case 'a': 
		m_manager.cur_camera->Keyboard(CameraMovement::left, speed);
		break;
	case 's': 
		m_manager.cur_camera->Keyboard(CameraMovement::backward, speed);
		break;
	case 'd': 
		m_manager.cur_camera->Keyboard(CameraMovement::right, speed);
		break;
	case 'q': 
		m_manager.cur_camera->Keyboard(CameraMovement::up, speed);
		break;
	case 'e': 
		m_manager.cur_camera->Keyboard(CameraMovement::down, speed);
		break;
	default:
		break;
	}
}

void render_engine::look_at(int mesh_id, vec3 relative) {
	auto at_mesh = get_mesh(mesh_id);
	if(at_mesh) {
		m_manager.render_scene->focus_at(m_manager.cur_camera, at_mesh, relative);
	} else {
		WARN("Cannot find mesh " + std::to_string(mesh_id));
	}
}

void render_engine::norm_render_scene() {
	//#todo_normalize_render_scene
}

void render_engine::draw_line(glm::vec3 t, glm::vec3 h, vec3 tc, vec3 hc) {
	std::shared_ptr<mesh> line_mesh = std::make_shared<mesh>();
	line_mesh->add_vertex(t, vec3(0.0f), tc);
	line_mesh->add_vertex(h, vec3(0.0f), hc);
	
	rendering_params params = { m_manager.cur_camera, m_manager.lights, 0, draw_type::line_segments};

	glDisable(GL_DEPTH_TEST);
	m_manager.shaders.at("template") ->draw_mesh(line_mesh, params);
	glEnable(GL_DEPTH_TEST);

	mesh::id--;
}

void render_engine::set_mesh_color(mesh_id id, vec3 c) {
	auto mesh_ptr = m_manager.render_scene->get_mesh(id);
	if(mesh_ptr == nullptr) {
		WARN("Cannot find mesh ID[{}]", id);
		return;
	}
	mesh_ptr->set_color(c);
}

void render_engine::mesh_add_transform(mesh_id id, glm::mat4 mat) {
	auto m = m_manager.render_scene->get_mesh(id);
	m->m_world = mat * m->m_world;
}

void render_engine::mesh_set_transform(mesh_id id, glm::mat4 mat) {
	auto m = m_manager.render_scene->get_mesh(id);
	m->set_world_mat(mat);
}

glm::mat4 render_engine::get_mesh_world(mesh_id id) {
	auto m = m_manager.render_scene->get_mesh(id);
	return m->get_world_mat();
}

void render_engine::set_render_camera(int w, int h) {
	if (m_manager.cur_camera == nullptr) {
		m_manager.cur_camera = std::make_shared<ppc>(w, h, 120.0f);
		return;
	} 

	m_manager.cur_camera->_width = w;
	m_manager.cur_camera->_height = h;
	glViewport(0, 0, m_manager.cur_camera->_width, m_manager.cur_camera->_height);
}

void render_engine::set_render_camera(int w, int h, float fov) {
	if (m_manager.cur_camera == nullptr) {
		m_manager.cur_camera = std::make_shared<ppc>(w, h, fov);
		return;
	} 
	m_manager.cur_camera->_width = w;
	m_manager.cur_camera->_height = h;
	m_manager.cur_camera->_fov = fov;
	glViewport(0, 0, m_manager.cur_camera->_width, m_manager.cur_camera->_height);
}

void render_engine::set_shader(mesh_id id, const std::string shader_name) {
	auto m = m_manager.render_scene->get_mesh(id);
	m_manager.rendering_mappings[m] = m_manager.shaders.at(shader_name);
}

void render_engine::remove_mesh(int mesh_id) {
	m_manager.render_scene->remove_mesh(mesh_id);
}

void render_engine::draw_visualize_voxels(std::vector<AABB> voxels) {
	// auto vis_mesh = vis_new_mesh();
	// if (vis_mesh) {
	// 	for(auto& cur_bb : voxels) {
	// 		auto tri_meshes = cur_bb.to_tri_mesh();
	// 		vis_mesh->add_vertices(tri_meshes);
	// 	}
	// }
	// vis_mesh->set_color(vec3(0.0f, 0.0f, 0.8f));
}

void render_engine::voxel_vis(mesh_id id) {
	auto mesh_ptr = m_manager.render_scene->get_mesh(id);
	if (mesh_ptr) {
		std::vector<AABB> voxels; 
		voxelizater::voxelize(mesh_ptr, 10, voxels);
		draw_visualize_voxels(voxels);
	}
}

std::shared_ptr<ppc> render_engine::get_render_ppc() {
	return m_manager.cur_camera;
}

void render_engine::draw_visualize_line(glm::vec3 t, glm::vec3 h) {
}

void render_engine::draw_image(std::shared_ptr<Image> img) {
	Mesh_Descriptor image_descriptor = {nullptr, {img}};
	auto param = rendering_params();
	m_manager.shaders.at("quad")->draw_mesh(image_descriptor, param);
}

void render_engine::draw_sihouette(mesh_id id, vec3 light_pos) {
	auto mesh_ptr = m_manager.render_scene->get_mesh(id);
	if (!mesh_ptr) {
		INFO("Cannot find the mesh" + std::to_string(id));
		return;
	}

	// compute sihouette
	std::vector<std::shared_ptr<geo_edge>> sihouettes = compute_sihouette(mesh_ptr, light_pos);

	// visualize sihouette
	for(auto sptr:sihouettes) {
		add_visualize_line(sptr->h->p, sptr->t->p);
	}
}

void render_engine::draw_shadow_volume(mesh_id id, vec3 light_pos) {
	// compute sihouette edges
	auto mesh_ptr = m_manager.render_scene->get_mesh(id);
	if (!mesh_ptr) {
		INFO("Cannot find the mesh" + std::to_string(id));
		return;
	}

	// compute extruded triangles
	std::shared_ptr<mesh> shadow_volume = std::make_shared<mesh>();
	auto shadow_verts = compute_shadow_volume(mesh_ptr, light_pos);
	shadow_volume->add_vertices(shadow_verts);
	shadow_volume->set_color(vec3(1.0f));
	
	m_manager.rendering_mappings[shadow_volume] = m_manager.shaders["template"];
	clear_visualize();
	m_manager.visualize_scene->add_mesh(shadow_volume);
}

void render_engine::clear_visualize() {
	m_manager.visualize_scene->clean_up();
}
