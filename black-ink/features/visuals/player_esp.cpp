#include "../features.h"

constexpr float SPEED_FREQ = 255 / 4.0f;

void VectorTransform( const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out )
{
	out[ 0 ] = in1.dot_product( in2[ 0 ] ) + in2[ 0 ][ 3 ];
	out[ 1 ] = in1.dot_product( in2[ 1 ] ) + in2[ 1 ][ 3 ];
	out[ 2 ] = in1.dot_product( in2[ 2 ] ) + in2[ 2 ][ 3 ];
}

RECT GetBBox( c_base_entity* pEntity ) 
{
	auto pCollideable = pEntity->get_collideable( );
	if ( !pCollideable )
		return { };

	vec3_t vecMins = pCollideable->obb_mins( );
	vec3_t vecMaxs = pCollideable->obb_maxs( );

	const matrix3x4_t& m_Transformation = pEntity->get_rgfl_coordinate_frame( );
	std::array < vec3_t, 8 > m_Points
		=
	{
		vec3_t( vecMins.x, vecMins.y, vecMins.z ),
		vec3_t( vecMins.x, vecMaxs.y, vecMins.z ),
		vec3_t( vecMaxs.x, vecMaxs.y, vecMins.z ),
		vec3_t( vecMaxs.x, vecMins.y, vecMins.z ),
		vec3_t( vecMaxs.x, vecMaxs.y, vecMaxs.z ),
		vec3_t( vecMins.x, vecMaxs.y, vecMaxs.z ),
		vec3_t( vecMins.x, vecMins.y, vecMaxs.z ),
		vec3_t( vecMaxs.x, vecMins.y, vecMaxs.z )
	};

	std::array < vec3_t, 8 > m_PointsTransformated;
	for ( int nVector = 0; nVector < 8; nVector++ )
		VectorTransform( m_Points[ nVector ], m_Transformation, m_PointsTransformated[ nVector ] );

	std::array < vec3_t, 8 > m_ScreenPoints;
	for ( int nVector = 0; nVector < 8; nVector++ )
	{
		if ( !render::world_to_screen( m_PointsTransformated[ nVector ], m_ScreenPoints[ nVector ] ) )
			return { };
	}

	float flLeft = m_ScreenPoints[ 0 ].x;
	float flTop = m_ScreenPoints[ 0 ].y;
	float flRight = m_ScreenPoints[ 0 ].x;
	float flBottom = m_ScreenPoints[ 0 ].y;

	for ( int nID = 1; nID < 8; nID++ )
	{
		if ( flLeft > m_ScreenPoints[ nID ].x )
			flLeft = m_ScreenPoints[ nID ].x;

		if ( flTop < m_ScreenPoints[ nID ].y )
			flTop = m_ScreenPoints[ nID ].y;

		if ( flRight < m_ScreenPoints[ nID ].x )
			flRight = m_ScreenPoints[ nID ].x;

		if ( flBottom > m_ScreenPoints[ nID ].y )
			flBottom = m_ScreenPoints[ nID ].y;
	}

	return RECT{ ( LONG ) ( flLeft ), ( LONG ) ( flBottom ), ( LONG ) ( flRight ), ( LONG ) ( flTop ) };
}

template < typename T>
__forceinline T Interpolate( const T& T1, const T& T2, float flProgress )
{
	return T1 + ( ( T2 - T1 ) * flProgress );
}

bool c_player_esp::force_dormant( c_base_player* pPlayer )
{
	float flTimeInDormant = dormant->get_time_in_dormant( pPlayer->get_index( ) );
	float flSinceLastData = dormant->get_time_since_last_data( pPlayer->get_index( ) );

	if ( flTimeInDormant < flSinceLastData )
	{
		if ( flTimeInDormant < 8.0f )
			pPlayer->set_abs_origin( dormant->get_last_network_origin( pPlayer->get_index( ) ) );
		else if ( flSinceLastData < 8.0f )
			pPlayer->set_abs_origin( dormant->get_last_dormant_origin( pPlayer->get_index( ) ) );
	}
	else
	{
		if ( flSinceLastData < 8.0f )
			pPlayer->set_abs_origin( dormant->get_last_dormant_origin( pPlayer->get_index( ) ) );
		else if ( flTimeInDormant < 8.0f )
			pPlayer->set_abs_origin( dormant->get_last_network_origin( pPlayer->get_index( ) ) );
	}

	return flTimeInDormant < 8.0f || flSinceLastData < 8.0f;
}

void c_player_esp::on_paint() {
	if ( !globals::m_local )
		return;

	for (auto i = 1; i < interfaces::m_global_vars->m_max_clients; i++)
	{
		const auto player = static_cast<c_cs_player*>(interfaces::m_entity_list->get_client_entity(i));

		if ( !player || !player->is_alive( ) || !player->is_player( ) )
		{
			if ( player && player->is_player( ) )
				dormant->reset_player( player );

			continue;
		}

		ESPPlayerData_t* m_Data = &m_PlayerData[ i ];
		if ( !m_Data )
			continue;

		if ( player->is_dormant( ) )
		{
			m_Data->m_flPlayerPercentage = Interpolate( m_Data->m_flPlayerPercentage, 0.15f, interfaces::m_global_vars->m_frame_time * 3.0f );
			if ( m_Data->m_flPlayerPercentage <= 0.15f )
				m_Data->m_flPlayerPercentage = 0.15f;

			if ( !force_dormant( player ) )
				continue;
		}
		else
		{
			m_Data->m_flPlayerPercentage = Interpolate( m_Data->m_flPlayerPercentage, 1.01f, interfaces::m_global_vars->m_frame_time * 3.0f );
			dormant->reset_player( player );
		}

		m_Data->m_flPlayerPercentage = fmin( m_Data->m_flPlayerPercentage, 1.00f );

		if ( ( c_cs_player* ) globals::m_local == player )
			continue;

		int nItemID = 0;
		if ( player->get_active_weapon( ) )
			nItemID = player->get_active_weapon( )->get_item_definition_index( );

		int nWeaponType = 0;
		if ( player->get_active_weapon( ) )
			if ( player->get_active_weapon( )->get_cs_weapon_data( ) )
				nWeaponType = player->get_active_weapon( )->get_cs_weapon_data( )->m_weapon_type;

		int nHealth = player->get_health( );

		if ( player->is_dormant( ) )
		{
			int nLastWeaponID = dormant->m_dormant_players.at(i ).m_last_weapon_id;

			if ( player->get_active_weapon( ) )
			{
				if ( nLastWeaponID > 0 )
					player->get_active_weapon( )->get_item_definition_index( ) = nLastWeaponID;

				int nLastWeaponType = dormant->m_dormant_players.at( i ).m_last_weapon_type;
				if ( player->get_active_weapon( )->get_cs_weapon_data( ) )
					if ( nLastWeaponType > -1 )
						player->get_active_weapon( )->get_cs_weapon_data( )->m_weapon_type = nLastWeaponType;
			}

			int nDormantHealth = dormant->m_dormant_players.at( i ).m_last_dormant_health;
			if ( nDormantHealth != nHealth )
				player->get_health( ) = nDormantHealth;
		}

		bool is_team = player->get_team() == globals::m_local->get_team();

		if (player->get_origin() == vec3_t(0, 0, 0))
			continue;

		m_Data->m_aBBox = GetBBox( player );
		m_Data->m_iWidth = abs( m_Data->m_aBBox.right - m_Data->m_aBBox.left );
		m_Data->m_iHeight = abs( m_Data->m_aBBox.top - m_Data->m_aBBox.bottom );

		if ( !is_team )
			render_enemy_draggable( player_esp_preview, player, m_Data ); 
		else 
			render_team_draggable( player_team_esp_preview, player, m_Data );

		if ( player->is_dormant( ) )
		{
			if ( player->get_active_weapon( ) )
			{
				player->get_active_weapon( )->get_item_definition_index( ) = nItemID;
				if ( player->get_active_weapon( )->get_cs_weapon_data( ) )
					player->get_active_weapon( )->get_cs_weapon_data( )->m_weapon_type = nWeaponType;
			}

			player->get_health( ) = nHealth;
		}
	}
}

ImVec2 c_player_esp::get_position( DraggableItemCondiction pos, ESPPlayerData_t* m_Data ) {
	if ( pos == RIGHT_COND )
		return ImVec2( m_Data->m_aBBox.right + 2 + m_Data->m_iRightOffset, m_Data->m_aBBox.top + m_Data->m_iRightDownOffset );
	if ( pos == LEFT_COND )
		return ImVec2( m_Data->m_aBBox.left - 2 - m_Data->m_iLeftOffset, m_Data->m_aBBox.top + m_Data->m_iLeftDownOffset );
	if ( pos == BOT_COND )
		return ImVec2( m_Data->m_aBBox.left + ( ( m_Data->m_aBBox.right - m_Data->m_aBBox.left ) * 0.5f ), m_Data->m_aBBox.bottom + 2 + m_Data->m_iDownOffset );
	if ( pos == TOP_COND )
		return ImVec2( m_Data->m_aBBox.left + ( ( m_Data->m_aBBox.right - m_Data->m_aBBox.left ) * 0.5f ), m_Data->m_aBBox.top - m_Data->m_iUpOffset );
}

ImVec2 c_player_esp::get_position_offsetless( DraggableItemCondiction pos, ESPPlayerData_t* m_Data ) {
	if ( pos == RIGHT_COND )
		return ImVec2( m_Data->m_aBBox.right + 3 + m_Data->m_iRightOffset, m_Data->m_aBBox.top );
	if ( pos == LEFT_COND )
		return ImVec2( m_Data->m_aBBox.left - 3 - m_Data->m_iLeftOffset, m_Data->m_aBBox.top );
	if ( pos == BOT_COND )
		return ImVec2( m_Data->m_aBBox.left, m_Data->m_aBBox.bottom + 2 );
	if ( pos == TOP_COND )
		return ImVec2( m_Data->m_aBBox.left, m_Data->m_aBBox.top - 4 );
}

__forceinline ImColor toColor( col_t color )
{
	return ImColor( color.r( ), color.g( ), color.b( ), color.a( ) );
}

void c_player_esp::add_text( std::string text, DraggableItemCondiction pos, ImColor color, ESPPlayerData_t* m_Data ) {
	ImVec2 ImTextSize = ImGui::CalcTextSize( text.c_str( ) );
	ImVec2 Position = get_position( pos, m_Data );

	if ( pos == LEFT_COND )
		Position = Position - ImVec2( ImTextSize.x + 4, 0 );
	if ( pos == RIGHT_COND )
		Position = Position + ImVec2( 4, 0 );
	if ( pos == BOT_COND )
		Position = Position - ImVec2( ImTextSize.x / 2, 0 );
	if ( pos == TOP_COND )
		Position = Position - ImVec2( ImTextSize.x / 2, ImTextSize.y );

	render::text( text, vec2_t(Position.x, Position.y), col_t( color.Value.x * 255, color.Value.y * 255, color.Value.z * 255, color.Value.w * 255 * m_Data->m_flPlayerPercentage ), fonts::m_minecraft12, FONT_DROP_SHADOW );
	
	if ( pos == RIGHT_COND )
		m_Data->m_iRightDownOffset = m_Data->m_iRightDownOffset + ImTextSize.y;
	if ( pos == LEFT_COND )
		m_Data->m_iLeftDownOffset = m_Data->m_iLeftDownOffset + ImTextSize.y;
	if ( pos == BOT_COND )
		m_Data->m_iDownOffset = m_Data->m_iDownOffset + ImTextSize.y;
	if ( pos == TOP_COND )
		m_Data->m_iUpOffset = m_Data->m_iUpOffset + ImTextSize.y;
}

void c_player_esp::add_bar( DraggableItemCondiction pos, float& percentage, float max, ImColor color, ImColor color1, ImColor color2, ESPPlayerData_t* m_Data ) {

	ImVec2 Position = get_position_offsetless( pos, m_Data );
	int XOffset, X2Offset;
	int YOffset, Y2Offset;

	auto widthSides = abs( m_Data->m_aBBox.bottom - m_Data->m_aBBox.top ) - ( ( ( abs( m_Data->m_aBBox.bottom - m_Data->m_aBBox.top ) * percentage ) / max ) );
	auto widthUpDown = abs( m_Data->m_aBBox.left - m_Data->m_aBBox.right ) - ( ( ( abs( m_Data->m_aBBox.left - m_Data->m_aBBox.right ) * percentage ) / max ) );

	if ( pos == RIGHT_COND ) {
		XOffset = 1 + m_Data->m_iRightOffset;
		X2Offset = 3 + m_Data->m_iRightOffset;
		YOffset = 1;
		Y2Offset = -1 + m_Data->m_iHeight;
	}

	if ( pos == LEFT_COND ) {
		XOffset = -1 - m_Data->m_iLeftOffset;
		X2Offset = -3 - m_Data->m_iLeftOffset;
		YOffset = 1;
		Y2Offset = -1 + m_Data->m_iHeight;
	}

	if ( pos == BOT_COND || pos == TOP_COND ) {
		XOffset = 1;
		X2Offset = m_Data->m_iWidth - 1;
		YOffset = m_Data->m_iDownOffset;
		Y2Offset = 2 + m_Data->m_iDownOffset;
	}

	render::rect_angle(
		( float ) Position.x + XOffset - 1 + ( pos == LEFT_COND ? 1 : 0 ), ( float ) Position.y + YOffset - 1,
		( float ) Position.x + X2Offset + 1 - ( pos == LEFT_COND ? 1 : 0 ), ( float ) Position.y + Y2Offset + 1,
		col_t( 
			( int ) ( color2.Value.x * 255 ),
			( int ) ( color2.Value.y * 255 ), 
			( int ) ( color2.Value.z * 255 ),
			( int ) ( color2.Value.w * 255 * m_Data->m_flPlayerPercentage )
		), 0, 0
	);

	render::rect_filled_int(
		( float ) Position.x + XOffset - 1, ( float ) Position.y + YOffset - 1,
		( float ) Position.x + X2Offset + 1, ( float ) Position.y + Y2Offset + 1,
		col_t(
			( int ) ( color1.Value.x * 255 ),
			( int ) ( color1.Value.y * 255 ),
			( int ) ( color1.Value.z * 255 ),
			( int ) ( color1.Value.w * 255 * m_Data->m_flPlayerPercentage )
		)
	);

	render::rect_filled_int(
		( float ) Position.x + XOffset, ( float ) Position.y + YOffset + ( pos == LEFT_COND || pos == RIGHT_COND ? widthSides : 0 ),
		( float ) Position.x + X2Offset - ( pos == BOT_COND || pos == TOP_COND ? widthUpDown : 0 ), ( float ) Position.y + Y2Offset,
		col_t(
			( int ) ( color.Value.x * 255 ),
			( int ) ( color.Value.y * 255 ),
			( int ) ( color.Value.z * 255 ),
			( int ) ( color.Value.w * 255 * m_Data->m_flPlayerPercentage)
		)
	);

	if ( pos == RIGHT_COND )
		m_Data->m_iRightOffset = m_Data->m_iRightOffset + 3;
	if ( pos == LEFT_COND )
		m_Data->m_iLeftOffset = m_Data->m_iLeftOffset + 3;
	if ( pos == BOT_COND )
		m_Data->m_iDownOffset = m_Data->m_iDownOffset + 5;
	if ( pos == TOP_COND )
		m_Data->m_iUpOffset = m_Data->m_iUpOffset + 5;
}

void c_player_esp::add_box( ESPPlayerData_t* m_Data, ImColor outer, ImColor inner, ImColor out )
{
	render::rect_angle(
		m_Data->m_aBBox.left, m_Data->m_aBBox.top,
		m_Data->m_aBBox.right, m_Data->m_aBBox.bottom,
		col_t(
			( int ) ( outer.Value.x * 255 ), 
			( int ) ( outer.Value.y * 255 ),
			( int ) ( outer.Value.z * 255 ),
			( int ) ( outer.Value.w * 255 * m_Data->m_flPlayerPercentage )
		), 1 , 0
	);

	render::rect_angle(
		m_Data->m_aBBox.left + 1,  m_Data->m_aBBox.top + 1,
		m_Data->m_aBBox.right - 1, m_Data->m_aBBox.bottom - 1,
		col_t(
			( int ) ( inner.Value.x * 255 ),
			( int ) ( inner.Value.y * 255 ),
			( int ) ( inner.Value.z * 255 ),
			( int ) ( inner.Value.w * 255 * m_Data->m_flPlayerPercentage )
		), 1, 0
	);

	render::rect_angle(
		m_Data->m_aBBox.left + 2, m_Data->m_aBBox.top + 2,
		m_Data->m_aBBox.right - 2, m_Data->m_aBBox.bottom - 2,
		col_t(
			( int ) ( out.Value.x * 255 ),
			( int ) ( out.Value.y * 255 ),
			( int ) ( out.Value.z * 255 ),
			( int ) ( out.Value.w * 255 * m_Data->m_flPlayerPercentage )
		), 1, 0
	);
}

void c_player_esp::render_enemy_draggable( c_esp_preview* preview, c_cs_player* player, ESPPlayerData_t* m_Data )
{
	for ( auto a = 0; a < POOL_COND; a++ )
	{
		m_Data->m_iDownOffset = 0;
		m_Data->m_iUpOffset = 0;

		m_Data->m_iLeftDownOffset = 0;
		m_Data->m_iLeftOffset = 0;

		m_Data->m_iRightDownOffset = 0;
		m_Data->m_iRightOffset = 0;

		for ( auto b = 0; b < preview->draggable_items[ a ].size( ); b++ )
		{
			if ( preview->draggable_items[ a ][ b ].Type == 0 ) {
				std::string Text;
				ImColor Color;

				if ( preview->draggable_items[ a ][ b ].ItemName == "Username" ) {
					player_info_t info;
					interfaces::m_engine->get_player_info( player->get_index( ), &info );

					Text = info.m_name;
					if ( Text.length( ) > 32 )
					{
						Text.erase( 32, Text.length( ) - 32 );
						Text.append( "..." );
					}
		
					Color = toColor( cfg::get<col_t>( FNV1A( "esp.enemies.nickname.color" ) ) );
				}

				if ( preview->draggable_items[ a ][ b ].ItemName == "Weapon" ) {
					c_base_combat_weapon* pWeapon = player->get_active_weapon( );
					if ( !pWeapon )
						continue;

					std::string weaponName = pWeapon->get_cs_weapon_data( )->m_hud_name;
					Text = std::string(weaponName.begin() + 13, weaponName.end());
					if ( Text.length( ) <= 0 )
						continue;

					Color = toColor( cfg::get<col_t>( FNV1A( "esp.enemies.weapon.color" ) ) );
				}

				add_text( Text, ( DraggableItemCondiction ) a, Color, m_Data );
			}
			if ( preview->draggable_items[ a ][ b ].Type == 1 ) {
				ImColor Main;
				ImColor Inner;
				ImColor Outer;

				if ( preview->draggable_items[ a ][ b ].ItemName == "Health" ) {
					auto iHealthValue = std::clamp( player->get_health( ), 0, 100 );
					auto percentage = iHealthValue / 100.f;
					if ( m_Data->m_flPrevHealth > iHealthValue )
						m_Data->m_flPrevHealth -= SPEED_FREQ * interfaces::m_global_vars->m_frame_time;
					else
						m_Data->m_flPrevHealth = iHealthValue;

					Main = toColor(cfg::get<col_t>( FNV1A( "esp.enemies.health.color" )));
					Outer = toColor( cfg::get<col_t>( FNV1A( "esp.enemies.health.border.outside.color" ) ) );
					Inner = toColor( cfg::get<col_t>( FNV1A( "esp.enemies.health.border.inside.color" ) ) );

					add_bar( ( DraggableItemCondiction ) a, m_Data->m_flPrevHealth, 100, Main, Inner, Outer, m_Data );
				}

				if ( preview->draggable_items[ a ][ b ].ItemName == "Armor" ) {
					auto iArmorValue = std::clamp( player->get_armor_value( ), 0, 100 );
					if ( m_Data->m_flPrevArmor > iArmorValue )
						m_Data->m_flPrevArmor -= SPEED_FREQ * interfaces::m_global_vars->m_frame_time;
					else
						m_Data->m_flPrevArmor = iArmorValue;

					Main = toColor( cfg::get<col_t>( FNV1A( "esp.enemies.armor.color" ) ) );
					Outer = toColor( cfg::get<col_t>( FNV1A( "esp.enemies.armor.border.outside.color" ) ) );
					Inner = toColor( cfg::get<col_t>( FNV1A( "esp.enemies.armor.border.inside.color" ) ) );

					add_bar( ( DraggableItemCondiction ) a, m_Data->m_flPrevArmor, 100, Main, Inner, Outer, m_Data );
				}
			}
			if ( preview->draggable_items[ a ][ b ].Type == 2 ) {
					add_box( m_Data, 
						toColor( cfg::get<col_t>( FNV1A( "esp.enemies.box.border.outside.color" ) ) ),
						toColor( cfg::get<col_t>( FNV1A( "esp.enemies.box.color" ) ) ),
						toColor( cfg::get<col_t>( FNV1A( "esp.enemies.box.border.inside.color" ) ) )
					);
			}
		}
	}
}

void c_player_esp::render_team_draggable( c_esp_preview* preview, c_cs_player* player, ESPPlayerData_t* m_Data )
{
	for ( auto a = 0; a < POOL_COND; a++ )
	{
		m_Data->m_iDownOffset = 0;
		m_Data->m_iUpOffset = 0;

		m_Data->m_iLeftDownOffset = 0;
		m_Data->m_iLeftOffset = 0;

		m_Data->m_iRightDownOffset = 0;
		m_Data->m_iRightOffset = 0;

		for ( auto b = 0; b < preview->draggable_items[ a ].size( ); b++ )
		{
			if ( preview->draggable_items[ a ][ b ].Type == 0 ) {
				std::string Text;
				ImColor Color;

				if ( preview->draggable_items[ a ][ b ].ItemName == "Username" ) {
					player_info_t info;
					interfaces::m_engine->get_player_info( player->get_index( ), &info );

					Text = info.m_name;
					if ( Text.length( ) > 32 )
					{
						Text.erase( 32, Text.length( ) - 32 );
						Text.append( "..." );
					}

					Color = toColor( cfg::get<col_t>( FNV1A( "esp.team.nickname.color" ) ) );
				}

				if ( preview->draggable_items[ a ][ b ].ItemName == "Weapon" ) {
					c_base_combat_weapon* pWeapon = player->get_active_weapon( );
					if ( !pWeapon )
						continue;

					std::string weaponName = pWeapon->get_cs_weapon_data( )->m_hud_name;
					Text = std::string( weaponName.begin( ) + 13, weaponName.end( ) );
					if ( Text.length( ) <= 0 )
						continue;

					Color = toColor( cfg::get<col_t>( FNV1A( "esp.team.weapon.color" ) ) );
				}

				add_text( Text, ( DraggableItemCondiction ) a, Color, m_Data );
			}
			if ( preview->draggable_items[ a ][ b ].Type == 1 ) {
				ImColor Main;
				ImColor Inner;
				ImColor Outer;

				if ( preview->draggable_items[ a ][ b ].ItemName == "Health" ) {
					auto iHealthValue = std::clamp( player->get_health( ), 0, 100 );
					auto percentage = iHealthValue / 100.f;
					if ( m_Data->m_flPrevHealth > iHealthValue )
						m_Data->m_flPrevHealth -= SPEED_FREQ * interfaces::m_global_vars->m_frame_time;
					else
						m_Data->m_flPrevHealth = iHealthValue;

					Main = toColor( cfg::get<col_t>( FNV1A( "esp.team.health.color" ) ) );
					Outer = toColor( cfg::get<col_t>( FNV1A( "esp.team.health.border.outside.color" ) ) );
					Inner = toColor( cfg::get<col_t>( FNV1A( "esp.team.health.border.inside.color" ) ) );

					add_bar( ( DraggableItemCondiction ) a, m_Data->m_flPrevHealth, 100, Main, Inner, Outer, m_Data );
				}

				if ( preview->draggable_items[ a ][ b ].ItemName == "Armor" ) {
					auto iArmorValue = std::clamp( player->get_armor_value( ), 0, 100 );
					if ( m_Data->m_flPrevArmor > iArmorValue )
						m_Data->m_flPrevArmor -= SPEED_FREQ * interfaces::m_global_vars->m_frame_time;
					else
						m_Data->m_flPrevArmor = iArmorValue;

					Main = toColor( cfg::get<col_t>( FNV1A( "esp.team.armor.color" ) ) );
					Outer = toColor( cfg::get<col_t>( FNV1A( "esp.team.armor.border.outside.color" ) ) );
					Inner = toColor( cfg::get<col_t>( FNV1A( "esp.team.armor.border.inside.color" ) ) );

					add_bar( ( DraggableItemCondiction ) a, m_Data->m_flPrevArmor, 100, Main, Inner, Outer, m_Data );
				}
			}
			if ( preview->draggable_items[ a ][ b ].Type == 2 ) {
				add_box( m_Data,
					toColor( cfg::get<col_t>( FNV1A( "esp.team.box.border.outside.color" ) ) ),
					toColor( cfg::get<col_t>( FNV1A( "esp.team.box.color" ) ) ),
					toColor( cfg::get<col_t>( FNV1A( "esp.team.box.border.inside.color" ) ) )
				);
			}
		}
	}
}