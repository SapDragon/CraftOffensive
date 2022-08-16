#pragma once
#include "../../globals.h"
#include <array>
enum e_material_type {
	MATERIAL_TYPE_REGULAR,
	MATERIAL_TYPE_FLAT
};
struct chams_material_settings_t
{
	bool m_enable;
	int m_material = MATERIAL_TYPE_REGULAR;
	col_t m_color = col_t( 255, 0, 0 );
};

struct chams_settings_t
{
	bool m_enable = false;
	std::array < chams_material_settings_t, 6 > m_materials;
};

struct chams_entity_settings_t
{
	chams_settings_t m_visible;
	chams_settings_t m_invisible;
};

class c_chams : public c_singleton<c_chams> {
private:

	void override_material(int type, const col_t& clr, bool ignorez);

	void draw_material_on_entity( chams_entity_settings_t options, i_model_render* ecx, void* context, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bones );

	i_material* create_material(std::string_view material_name, std::string_view shader_type, std::string_view material_data);

	i_material* m_regular = nullptr;
	i_material* m_regular_z = nullptr;

	i_material* m_flat = nullptr;
	i_material* m_flat_z = nullptr;



	std::map < int, chams_entity_settings_t > m_shared_players;
public:
	c_chams() {
		m_regular = create_material(_("black_ink_regular.vmt"), _("VertexLitGeneric"), _(R"#("VertexLitGeneric" {
	"$basetexture" "vgui/white_additive"
	"$ignorez"      "0"
	"$model"		"1"
	"$flat"			"0"
	"$nocull"		"1"
	"$halflambert"	"1"
	"$nofog"		"1"
	"$wireframe"	"0"
})#"));

		m_regular_z = create_material(_("black_ink_regular_z.vmt"), _("VertexLitGeneric"), _(R"#("VertexLitGeneric" {
	"$basetexture" "vgui/white_additive"
	"$ignorez"      "1"
	"$model"		"1"
	"$flat"			"0"
	"$nocull"		"1"
	"$halflambert"	"1"
	"$nofog"		"1"
	"$wireframe"	"0"
})#"));

		m_flat = create_material(_("black_ink_flat.vmt"), _("UnlitGeneric"), _(R"#("UnlitGeneric" {
	"$basetexture" "vgui/white_additive"
	"$ignorez"      "0"
	"$model"		"1"
	"$flat"			"1"
	"$nocull"		"1"
	"$selfillum"	"1"
	"$halflambert"	"1"
	"$nofog"		"1"
	"$wireframe"	"0"
})#"));

		m_flat_z = create_material(_("black_ink_flat_z.vmt"), _("UnlitGeneric"), _(R"#("UnlitGeneric" {
	"$basetexture" "vgui/white_additive"
	"$ignorez"      "1"
	"$model"		"1"
	"$flat"			"1"
	"$nocull"		"1"
	"$selfillum"	"1"
	"$halflambert"	"1"
	"$nofog"		"1"
	"$wireframe"	"0"
})#"));
	}

	chams_entity_settings_t get_shared_player( int player_index ) { return m_shared_players[ player_index ]; }
	chams_entity_settings_t set_shared_chams( int player_index, chams_entity_settings_t settings ) { return m_shared_players[ player_index ] = settings; }

	bool on_draw_model(i_model_render* ecx, void* context, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bones);
};
#define chams c_chams::instance()