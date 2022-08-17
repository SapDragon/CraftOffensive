#pragma once

struct view_setup_t {
	int			m_x;
	int			m_x_unscaled;
	int			m_y;
	int			m_y_unscaled;
	int			m_width;
	int			m_old_width;
	int			m_height;
	int			m_old_height;
	char		pad0[144];
	float		m_fov;
	float		m_view_model_fov;
	vec3_t		m_origin;
	qangle_t	m_angles;
	float m_near;
	float m_far;
	float m_near_viewmodel;
	float m_far_viewmodel;
	float m_aspect_ratio;
};

class i_client_mode {
public:

};