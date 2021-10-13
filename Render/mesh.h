#pragma once
#include <common.h>

using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;
namespace pd = purdue;

/*
* Class Axis aligned bounding box
**/
struct AABB
{
    vec3 p0, p1;
	AABB()=default;
	AABB(vec3 p) :p0(p), p1(p) {}
	AABB(vec3 sm, vec3 lg) :p0(sm), p1(lg) {} // small, large

	void add_point(vec3 p); 
	void add_aabb(const AABB &new_aabb); 
	float diag_length();	
	vec3 diagonal(); 
	vec3 center(); 
	bool inside(vec3 p); 
	bool collide(AABB b); 
	AABB transform(mat4 m); 
	std::string to_string(); 
	std::vector<glm::vec3> to_tri_mesh();
	std::vector<glm::vec3> to_line_mesh();
};

class mesh : public ISerialize {
public:
	mesh();
	~mesh();

    /* Interface */
    virtual std::string to_json() override;  
    virtual int from_json(const std::string jsonstr) override;  

public:
	//------- shared functions --------//
	std::vector<vec3> compute_world_space_coords();
	std::vector<vec3> compute_world_space_normals();
	vec3 compute_center();	// center in model space
	vec3 compute_world_center();  // center in world space
	
	void add_world_transate(vec3 v);
	void add_scale(vec3 s);	// scale in model space
	void add_rotate(pd::deg angle, vec3 axis);

	void add_face(vec3 va, vec3 vb, vec3 vc);	// estimate normal
	void add_vertex(vec3 v, vec3 n, vec3 c);
	void add_vertex(vec3 v, vec3 n, vec3 c, vec2 uv);
	void add_vertices(std::vector<vec3>& verts);

	AABB compute_aabb() const;
	AABB compute_world_aabb();
	void set_color(vec3 col);
	void set_color(unsigned triangle_id, vec3 col);
	void normalize_position_orientation(vec3 scale=vec3(1.0f), 
										glm::quat rot_quant = glm::quat(0.0f,0.0f,0.0f,0.0f));
	
	void get_demose_matrix(vec3& scale, quat& rot, vec3& translate);
	void set_matrix(const vec3 scale, const quat rot, const vec3 translate);
	void clear_vertices() { m_world = glm::identity<mat4>(); m_verts.clear(); m_norms.clear(); m_colors.clear(); m_uvs.clear(); }
	void recompute_normal();
	void remove_duplicate_vertices();
	std::string to_string() {
		return std::to_string(get_id());
	}

	//------- getter & setter --------//
public:
	mat4 get_world_mat() { return m_world; }
	void set_world_mat(const mat4 m) { m_world = m; }
	int get_id() { return cur_id; }
	bool get_is_selected() { return m_is_selected; }
	void set_is_selected(bool is_selected) { m_is_selected = is_selected; }
	void set_light(bool trigger = true) { m_is_emitter = true; }
	bool is_light() { return m_is_emitter; }
	void set_verts(std::vector<vec3> &verts) {
		m_verts = verts;
	}
    void set_caster(bool is_caster) { m_is_caster = is_caster;} 

	//------- member variables --------//
public:
	mat4 m_world = glm::mat4(1.0f); // model space -> world space
	std::vector<vec3> m_verts;
	std::vector<vec3> m_norms;
	std::vector<vec3> m_colors;
	std::vector<vec2> m_uvs;
	std::string file_path;
	
	std::string m_vs, m_fs;
	int cur_id = -1;
	static int id;
	bool m_is_selected = false;
	bool m_is_emitter = false;
    bool m_is_caster = true;
};
