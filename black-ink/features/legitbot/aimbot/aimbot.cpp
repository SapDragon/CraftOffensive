#include "aimbot.hpp"

void c_aimbot::recoil_control_system( qangle_t& view_angle )
{
	
}

void c_aimbot::on_create_move( )
{
	m_best_fov = 10000000;
	m_best_distance = 10000000;
	m_best_position = vec3_t( 0, 0, 0 );
	if ( !globals::m_local || !globals::m_local->is_alive( ) )
		return;


	if ( !cfg::get( FNV1A( "legitbot.aimbot.enable" ) ) )
		return;

	if ( cfg::get( FNV1A( "legitbot.aimbot.flash_check" ) ) && globals::m_local->get_flash_alpha( ) > 0.f )
		return;

	if ( cfg::get( FNV1A( "legitbot.aimbot.jump_check" ) ) && !globals::m_local->get_flags().has( FL_ONGROUND )  )
		return;

	auto local_weapon = globals::m_local->get_active_weapon( );

	if ( !local_weapon || !local_weapon->is_gun( ) )
		return;

	auto local_eye_pos = globals::m_local->get_eye_pos( );

	if ( !local_eye_pos.is_valid( ) )
		return;

	qangle_t view_angle;

	interfaces::m_engine->get_view_angles( view_angle );

	float aimbot_fov_settings =  cfg::get < float >( FNV1A( "legitbot.aimbot.fov" ) )  * 5.f;

	float rcs_fov_settings = cfg::get < float >( FNV1A( "legitbot.aimbot.rcs.fov" ) ) * 5.f;


	for ( int i = 0; i < interfaces::m_global_vars->m_max_clients; i++ )
	{ 
		auto player = reinterpret_cast< c_cs_player* > ( interfaces::m_entity_list->get_client_entity( i ) );

		if ( !player || player == (c_cs_player*) globals::m_local ||  !player->is_player( ) || !player->is_alive( ) || player->is_dormant( ) || !player->is_enemy( globals::m_local ) )
			continue;

		auto hdr = interfaces::m_model_info->get_studio_model( player->get_model( ) );

		if ( !hdr )
			continue;

		mstudiohitboxset_t* hitbox_set = ( mstudiohitboxset_t* ) hdr->get_hitbox_set( player->get_hitbox_set( ) );

		if ( !hitbox_set )
			continue;

		auto bone_cache = player->get_bone_cache( );

		if ( !bone_cache )
			continue;

		if ( !bone_cache->m_cached_bones )
			continue;

		for ( int hitbox_id = 0; hitbox_id < HITBOX_LEFT_FOREARM; hitbox_id++ )
		{
			if ( !cfg::get <std::array<bool, 18> >( FNV1A( "legitbot.aimbot.hitboxes" ) ).at( hitbox_id ) )
				continue;

			auto hitbox = hitbox_set->get_hitbox( hitbox_id );

			if ( !hitbox )
				continue;

			auto hitbox_position = hitbox->get_position( bone_cache->m_cached_bones );

			if ( !hitbox_position.is_valid( ) )
				continue;

			vec3_t hitbox_on_screen = vec3_t( );

			if ( !render::world_to_screen(hitbox_position, hitbox_on_screen ) )
					continue;

			float distance = local_eye_pos.dist_to( hitbox_position );
	
			float fov = vec3_t( ImGui::GetIO( ).DisplaySize.x / 2, ImGui::GetIO( ).DisplaySize.y / 2, 0 ).dist_to( hitbox_on_screen );

			if ( fov > aimbot_fov_settings && fov > rcs_fov_settings )
				continue;

			if ( fov < m_best_fov )
			{
				m_best_position = hitbox_position;
				m_best_distance = distance;
				m_best_fov = fov;
			}

		}
	}



	if ( m_best_position.empty( ) )
		return;

	auto aim_angle = math::calculate_angle( local_eye_pos, m_best_position );

	aim_angle.normalize( );

	auto punch_angle = qangle_t( 0, 0, 0 );

	if ( m_best_fov <= rcs_fov_settings  && globals::m_local->get_shots_fired( ) >= cfg::get < int >( FNV1A( "legitbot.aimbot.rcs.start_after" ) ) )
	{
		static auto recoil_scale = interfaces::m_cvar_system->find_var( FNV1A( "weapon_recoil_scale" ) )->get_float( );

		punch_angle = globals::m_local->get_aim_punch_angle( ) * recoil_scale;
		
		punch_angle.x *= cfg::get < float >( FNV1A( "legitbot.aimbot.rcs.pitch" ) );
		punch_angle.y *= cfg::get < float >( FNV1A( "legitbot.aimbot.rcs.yaw" ) );


	//	punch_angle /= ( ( interfaces::m_global_vars->m_interval_per_tick * ( 1.0 / interfaces::m_global_vars->m_interval_per_tick ) ) * cfg::get < float >( FNV1A( "legitbot.aimbot.rcs.smooth" ) ) );
	}

	auto final_angle = view_angle;

	if ( m_best_fov <= aimbot_fov_settings )
	{
		auto delta = aim_angle - ( view_angle + ( m_old_punch -  punch_angle )  );

		final_angle += ( delta / ( ( interfaces::m_global_vars->m_interval_per_tick * ( 1.0 / interfaces::m_global_vars->m_interval_per_tick ) ) * cfg::get < float >( FNV1A( "legitbot.aimbot.smooth" ) ) ) );
	}
	else
		final_angle += ( m_old_punch - punch_angle ) ;
	

	m_old_punch = punch_angle;

	if(!cfg::get(FNV1A("legitbot.aimbot.silent") ) )
		interfaces::m_engine->set_view_angles( final_angle );
	else
		globals::m_cur_cmd->m_view_angles = final_angle;
}