#include "../utils.h"
#include "../../features/visuals/chams.h"
#include "../../features/visuals/preview/preview.hpp"

namespace cfg_internal {
	enum State { ESCAPED, UNESCAPED };

	std::string unescapeJSON( const std::string& input )
	{
		State s = UNESCAPED;
		std::string output;
		output.reserve( input.length( ) );

		for ( std::string::size_type i = 0; i < input.length( ); ++i )
		{
			switch ( s )
			{
			case ESCAPED:
			{
				switch ( input[ i ] )
				{
				case '"':
					output += '\"';
					break;
				case '/':
					output += '/';
					break;
				case 'b':
					output += '\b';
					break;
				case 'f':
					output += '\f';
					break;
				case 'n':
					output += '\n';
					break;
				case 'r':
					output += '\r';
					break;
				case 't':
					output += '\t';
					break;
				case '\\':
					output += '\\';
					break;
				default:
					output += input[ i ];
					break;
				}

				s = UNESCAPED;
				break;
			}
			case UNESCAPED:
			{
				switch ( input[ i ] )
				{
				case '\\':
					s = ESCAPED;
					break;
				default:
					output += input[ i ];
					break;
				}
			}
			}
		}
		return output;
	}

	template <class T>
	void jsonify( nlohmann::json& j, bool load, uint32_t key )
	{
		if ( j.is_null( ) && load )
			return;

		if ( !load )
			j = cfg::get<T>( key );
		else
			cfg::set<T>( key, j.get<T>( ) );
	}

	void jsonify_color( nlohmann::json& j, bool load, uint32_t key )
	{
		if ( j.is_null( ) && load )
			return;

		if ( !load ) {
			j[ 0 ] = cfg::get<col_t>( key ).r( );
			j[ 1 ] = cfg::get<col_t>( key ).g( );
			j[ 2 ] = cfg::get<col_t>( key ).b( );
			j[ 3 ] = cfg::get<col_t>( key ).a( );
		}
		else {
			cfg::set<col_t>( key, col_t( j[ 0 ].get<int>( ), j[ 1 ].get<int>( ), j[ 2 ].get<int>( ), j[ 3 ].get<int>( ) ) );
		}
	}

	void jsonify_basic( nlohmann::json& j, bool load, bool& key )
	{
		if ( j.is_null( ) && load )
			return;

		if ( !load )
			j = key;
		else
			key = j.get<bool>( );
	}
	void jsonify_basic( nlohmann::json& j, bool load, int& key )
	{
		if ( j.is_null( ) && load )
			return;

		if ( !load )
			j = key;
		else
			key = j.get<int>( );
	}
	void jsonify_basic( nlohmann::json& j, bool load, float& key )
	{
		if ( j.is_null( ) && load )
			return;

		if ( !load )
			j = key;
		else
			key = j.get<float>( );
	}
	void jsonify_basic( nlohmann::json& j, bool load, col_t& key )
	{
		if ( j.is_null( ) && load )
			return;

		if ( !load ) {
			j[ 0 ] = key.r( );
			j[ 1 ] = key.g( );
			j[ 2 ] = key.b( );
			j[ 3 ] = key.a( );
		}
		else
			key = col_t( j[ 0 ].get<int>( ), j[ 1 ].get<int>( ), j[ 2 ].get<int>( ), j[ 3 ].get<int>( ) );
	}
	void jsonify_basic( nlohmann::json& j, bool load, std::string& key )
	{
		if ( j.is_null( ) && load )
			return;

		if ( !load )
			j = key;
		else
			key = j.get<std::string>( );
	}

	void jsonify_chams( nlohmann::json& j, bool load, std::vector < chams_material_settings_t>& hash, std::string prefix )
	{
		if ( !load ) {
			for ( auto a = 0; a < hash.size( ); a++ ) {
				j[ "chams" ][ prefix ][ a ][ "m_enable" ] = hash[ a ].m_enable;

				j[ "chams" ][ prefix ][ a ][ "m_color" ][ 0 ] = hash[ a ].m_color.r( );
				j[ "chams" ][ prefix ][ a ][ "m_color" ][ 1 ] = hash[ a ].m_color.g( );
				j[ "chams" ][ prefix ][ a ][ "m_color" ][ 2 ] = hash[ a ].m_color.b( );
				j[ "chams" ][ prefix ][ a ][ "m_color" ][ 3 ] = hash[ a ].m_color.a( );

				j[ "chams" ][ prefix ][ a ][ "label" ] = hash[ a ].label;
				j[ "chams" ][ prefix ][ a ][ "m_material" ] = hash[ a ].m_material;
			}
		}
		else {
			if ( j[ "chams" ][ prefix ].is_null( ) )
				return;

			hash.clear( );

			auto size = j[ "chams" ][ prefix ].size( );

			for ( auto a = 0; a < size; a++ ) {
				auto data = chams_material_settings_t{
					j[ "chams" ][ prefix ][ a ][ "m_enable" ].get<bool>( ),
					j[ "chams" ][ prefix ][ a ][ "m_material" ].get<int>( ),
					j[ "chams" ][ prefix ][ a ][ "label" ].get<std::string>( ),
					col_t(
						j[ "chams" ][ prefix ][ a ][ "m_color" ][ 0 ].get<int>( ),
						j[ "chams" ][ prefix ][ a ][ "m_color" ][ 1 ].get<int>( ),
						j[ "chams" ][ prefix ][ a ][ "m_color" ][ 2 ].get<int>( ),
						j[ "chams" ][ prefix ][ a ][ "m_color" ][ 3 ].get<int>( )
					)
				};

				hash.push_back( data );
			}
		}
	}
	void jsonify_materials( nlohmann::json& j, bool load, std::vector < chams_layer >& hash )
	{
		if ( !load ) {
			for ( auto a = 0; a < hash.size( ); a++ ) {
				if ( hash[ a ].buildin )
					continue;

				j[ "custom_materials" ][ a ][ "file_name" ] = hash[ a ].file_name;
				j[ "custom_materials" ][ a ][ "label" ] = hash[ a ].label;
				j[ "custom_materials" ][ a ][ "material_data" ] = hash[ a ].material_data;
				j[ "custom_materials" ][ a ][ "shader_type" ] = hash[ a ].shader_type;
			}
		}
		else {
			if ( j[ "custom_materials" ].is_null( ) )
				return;

			for ( auto a = 0; a < hash.size( ); a++ ) {
				if ( hash[ a ].buildin ) continue;

				hash.erase( hash.begin( ) + a );
			}

			auto size = j[ "custom_materials" ].size( );

			for ( auto a = 0; a < size; a++ )
			{
				if ( j[ "custom_materials" ][ a ].is_null( ) )
					continue;

				chams_layer data;

				data.material_data = unescapeJSON( j[ "custom_materials" ][ a ][ "material_data" ].get<std::string>( ) );
				data.shader_type = unescapeJSON( j[ "custom_materials" ][ a ][ "shader_type" ].get<std::string>( ) );
				data.label = unescapeJSON( j[ "custom_materials" ][ a ][ "label" ].get<std::string>( ) );
				data.file_name = unescapeJSON( j[ "custom_materials" ][ a ][ "file_name" ].get<std::string>( ) );

				data.buildin = false;

				chams->create_material( data );
			}

			menu->selected_material = 0;
		}
	}
	void jsonify_legitbot( nlohmann::json& j, bool load, aimbot_group_settings& hash )
	{
		if ( !load ) {
			j[ "enable" ] = hash.enable;
			j[ "silent" ] = hash.silent;

			j[ "fov" ] = hash.fov;
			j[ "smooth" ] = hash.smooth;

			for ( auto a = 0; a < 18; a++ )
				j[ "hitboxes" ][ a ] = hash.hitboxes[ a ];

			j[ "flash_check" ] = hash.flash_check;
			j[ "jump_check" ] = hash.jump_check;
			j[ "smoke_check" ] = hash.smoke_check;

			j[ "rcs.enable" ] = hash.rcs.enable;
			j[ "rcs.after" ] = hash.rcs.after;
			j[ "rcs.fov" ] = hash.rcs.fov;
			j[ "rcs.smooth" ] = hash.rcs.smooth;
			j[ "rcs.pitch" ] = hash.rcs.pitch;
			j[ "rcs.yaw" ] = hash.rcs.yaw;
		}
		else {
			if ( j.is_null( ) )
				return;

			hash.enable = j[ "enable" ].get<bool>( );
			hash.silent = j[ "silent" ].get<bool>( );

			hash.fov = j[ "fov" ].get<float>( );
			hash.smooth = j[ "smooth" ].get<float>( );

			for ( auto a = 0; a < 18; a++ )
				hash.hitboxes[ a ] = j[ "hitboxes" ][ a ].get<bool>( );

			hash.flash_check = j[ "flash_check" ].get<bool>( );
			hash.jump_check = j[ "jump_check" ].get<bool>( );
			hash.smoke_check = j[ "smoke_check" ].get<bool>( );

			hash.rcs.enable = j[ "rcs.enable" ].get<bool>( );
			hash.rcs.after = j[ "rcs.after" ].get<int>( );
			hash.rcs.fov = j[ "rcs.fov" ].get < float >( );
			hash.rcs.smooth = j[ "rcs.smooth" ].get<float>( );
			hash.rcs.pitch = j[ "rcs.pitch" ].get<float>( );
			hash.rcs.yaw = j[ "rcs.yaw" ].get<float>( );
		}
	}
}

namespace cfg {
	void init( ) {
		set<bool>( FNV1A( "fakelags.enable" ), false );
		set<int>( FNV1A( "fakelags.amount" ), 1 );

		set<bool>( FNV1A( "auto_jump" ), false );
		set<bool>( FNV1A( "autotrafe" ), false );
		set<bool>( FNV1A( "aimbot.show_fov" ), false );

		set < bool >( FNV1A( "misc.view_model.enable" ), false );
		set < float >( FNV1A( "misc.view_model.x" ), 1.f );
		set < float >( FNV1A( "misc.view_model.y" ), 1.f );
		set < float >( FNV1A( "misc.view_model.z" ), 1.f );
		set < int >( FNV1A( "misc.view_model.fov" ), 90.f );
		set < bool >( FNV1A( "misc.view_model.override_while_scoped" ), true );
		set < bool >( FNV1A( "misc.sharerd.name" ), false );

		set<col_t>( FNV1A( "esp.team.health.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.team.health.border.inside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.team.health.border.outside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.team.armor.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.team.armor.border.inside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.team.armor.border.outside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.team.nickname.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.team.weapon.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.team.box.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.team.box.border.inside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.team.box.border.outside.color" ), col_t( 55, 55, 55 ) );

		set<col_t>( FNV1A( "esp.enemies.health.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.enemies.health.border.inside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.enemies.health.border.outside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.enemies.armor.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.enemies.armor.border.inside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.enemies.armor.border.outside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.enemies.nickname.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.enemies.weapon.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.enemies.box.color" ), col_t( 255, 255, 255 ) );
		set<col_t>( FNV1A( "esp.enemies.box.border.inside.color" ), col_t( 55, 55, 55 ) );
		set<col_t>( FNV1A( "esp.enemies.box.border.outside.color" ), col_t( 55, 55, 55 ) );
	}

	std::string json_action( bool load )
	{
		try {
			cfg_internal::jsonify<bool>( jsonk[ "misc.view_model.enable" ], load, FNV1A( "misc.view_model.enable" ) );
			cfg_internal::jsonify<bool>( jsonk[ "misc.view_model.override_while_scoped" ], load, FNV1A( "misc.view_model.override_while_scoped" ) );
			cfg_internal::jsonify<bool>( jsonk[ "auto_jump" ], load, FNV1A( "auto_jump" ) );
			cfg_internal::jsonify<bool>( jsonk[ "autostrafe" ], load, FNV1A( "autotrafe" ) );
			cfg_internal::jsonify<bool>( jsonk[ "aimbot.show_fov" ], load, FNV1A( "aimbot.show_fov" ) );

			cfg_internal::jsonify<int>( jsonk[ "misc.view_model.fov" ], load, FNV1A( "misc.view_model.fov" ) );
			cfg_internal::jsonify<float>( jsonk[ "misc.view_model.x" ], load, FNV1A( "misc.view_model.x" ) );
			cfg_internal::jsonify<float>( jsonk[ "misc.view_model.y" ], load, FNV1A( "misc.view_model.y" ) );
			cfg_internal::jsonify<float>( jsonk[ "misc.view_model.z" ], load, FNV1A( "misc.view_model.z" ) );

			cfg_internal::jsonify<bool>( jsonk[ "fakelags.enable" ], load, FNV1A( "fakelags.enable" ) );
			cfg_internal::jsonify<int>( jsonk[ "fakelags.amount" ], load, FNV1A( "fakelags.amount" ) );

			cfg_internal::jsonify_color( jsonk[ "esp.team.health.color" ], load, FNV1A( "esp.team.health.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.health.border.inside.color" ], load, FNV1A( "esp.team.health.border.inside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.health.border.outside.color" ], load, FNV1A( "esp.team.health.border.outside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.armor.color" ], load, FNV1A( "esp.team.armor.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.armor.border.inside.color" ], load, FNV1A( "esp.team.armor.border.inside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.armor.border.outside.color" ], load, FNV1A( "esp.team.armor.border.outside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.nickname.color" ], load, FNV1A( "esp.team.nickname.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.weapon.color" ], load, FNV1A( "esp.team.weapon.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.box.color" ], load, FNV1A( "esp.team.box.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.box.border.inside.color" ], load, FNV1A( "esp.team.box.border.inside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.team.box.border.outside.color" ], load, FNV1A( "esp.team.box.border.outside.color" ) );

			cfg_internal::jsonify_color( jsonk[ "esp.enemies.health.color" ], load, FNV1A( "esp.enemies.health.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.health.border.inside.color" ], load, FNV1A( "esp.enemies.health.border.inside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.health.border.outside.color" ], load, FNV1A( "esp.enemies.health.border.outside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.armor.color" ], load, FNV1A( "esp.enemies.armor.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.armor.border.inside.color" ], load, FNV1A( "esp.enemies.armor.border.inside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.armor.border.outside.color" ], load, FNV1A( "esp.enemies.armor.border.outside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.nickname.color" ], load, FNV1A( "esp.enemies.nickname.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.weapon.color" ], load, FNV1A( "esp.enemies.weapon.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.box.color" ], load, FNV1A( "esp.enemies.box.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.box.border.inside.color" ], load, FNV1A( "esp.enemies.box.border.inside.color" ) );
			cfg_internal::jsonify_color( jsonk[ "esp.enemies.box.border.outside.color" ], load, FNV1A( "esp.enemies.box.border.outside.color" ) );

			cfg_internal::jsonify_legitbot( jsonk[ "aimbot" ][ "pistol" ], load, aimbot_pistol );
			cfg_internal::jsonify_legitbot( jsonk[ "aimbot" ][ "smg" ], load, aimbot_smg );
			cfg_internal::jsonify_legitbot( jsonk[ "aimbot" ][ "rifle" ], load, aimbot_rifle );
			cfg_internal::jsonify_legitbot( jsonk[ "aimbot" ][ "shotgun" ], load, aimbot_shotgun );
			cfg_internal::jsonify_legitbot( jsonk[ "aimbot" ][ "heavy" ], load, aimbot_heavy );
			cfg_internal::jsonify_legitbot( jsonk[ "aimbot" ][ "sniper" ], load, aimbot_sniper );

			cfg_internal::jsonify_chams( jsonk, load, local_player_visible, "local_player_visible" );
			cfg_internal::jsonify_chams( jsonk, load, enemy_visible, "enemy_visible" );
			cfg_internal::jsonify_chams( jsonk, load, teammates_visible, "teammates_visible" );
			cfg_internal::jsonify_chams( jsonk, load, local_player_invisible, "local_player_invisible" );
			cfg_internal::jsonify_chams( jsonk, load, enemy_invisible, "enemy_invisible" );
			cfg_internal::jsonify_chams( jsonk, load, teammates_invisible, "teammates_invisible" );

			cfg_internal::jsonify_materials( jsonk, load, chams->materials );
		}
		catch ( const std::runtime_error& error ) {
			MessageBoxA( nullptr, error.what( ), "craftoffensive | fatal error!", MB_OK | MB_ICONERROR );
		}

		return jsonk.dump( );
	}
}