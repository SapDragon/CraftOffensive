#include "menu.h"

#include "notifies.h"
#include "elements.h"

#include "../api/api.h"
#include "../features/visuals/model/model.h"
std::string tabLabels[ 6 ] = { _( "Legit" ), _( "Visuals" ), _( "Misc" ), _( "Skins" ), _( "Files" ), _( "Dashboard" ) };

static char login[ 32 ];
static char password[ 32 ];
static char email[ 32 ];

void user( ) {
	ImGui::Begin( _( "123" ), 0 );
	{
		ImGui::SetWindowSize( ImVec2( 400, 400 ), ImGuiCond_Once );


		static int stage = 0;

		if ( ImGui::Button( "auth" ) ) stage = 0;
		ImGui::SameLine( );
		if ( ImGui::Button( "register" ) ) stage = 1;

		if ( stage == 0 ) {
			ImGui::InputText( "login", login, 32 );
			ImGui::InputText( "password", password, 32 );

			if ( ImGui::Button( "Sign In" ) ) {
				cloud->sigin( login, password );
			}

			if ( ImGui::Button( "login as moneyharassment" ) ) {
				cloud->sigin( "moneyharassment", "moneyharassment" );
			}
		}

		if ( stage == 1 ) {
			ImGui::InputText( "login", login, 32 );
			ImGui::InputText( "password", password, 32 );
			ImGui::InputText( "email", email, 32 );

			if ( ImGui::Button( "Sign Up" ) ) {
				cloud->signup( login, password, email );
			}
		}
	}
	ImGui::End( );
}

void chams_page( std::vector < chams_material_settings_t>& chams ) {
	static int selected_material = 0;
	static char label[ 12 ];

	elements::child( _( "Added Materials" ), { 220, 335 }, [ & ] ( ) {
		ImGui::BeginChild( "Layers", ImVec2( 200, 245 ), false, ImGuiWindowFlags_NoBackground );
		{
			for ( auto mats = 0; mats < chams.size( ); mats++ ) {
				if ( elements::chams_item( mats, chams ) ) {
					selected_material = mats;
					memset( label, 0, sizeof label );
				}
			}
		}
		ImGui::EndChild( );

		ImGui::SetCursorPos( { 0, 255 } );
		if ( elements::button( _( "New Material" ), ImVec2( 200, 30 ) ) ) {
			chams.push_back( chams_material_settings_t{} );
		}
	} );

	if ( !chams.empty( ) ) {
		ImGui::SameLine( 230 );

		elements::child( _( "Edit Material" ), { 400, 240 }, [ & ] ( ) {

			if ( ImGui::InputText( "Material Label", label, 12 ) ) {
				chams[ selected_material ].label = std::string( label );
			}
			ImGui::InputInt( "Material", &chams[ selected_material ].m_material, 0, 1 );
		} );
	}
}

void c_menu::on_paint() {
	notifies::handle( ImGui::GetForegroundDrawList( ) );

	if (!(input::m_blocked = input::get_key<TOGGLE>(VK_INSERT)))
		return;

	ImGui::Begin( _( "Back" ), 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus );
	{
		ImGui::SetWindowSize( ImGui::GetIO( ).DisplaySize );
		ImGui::SetWindowPos( {} );

		ImGui::GetWindowDrawList( )->AddRectFilled( { 0, 0 }, ImGui::GetIO( ).DisplaySize, ImColor( 24, 24, 24, 100 ) );
		ImGui::GetWindowDrawList( )->AddImage( assets::logotype,
			ImGui::GetIO( ).DisplaySize / 2 - ImVec2( 367, 51 ),
			ImGui::GetIO( ).DisplaySize / 2 + ImVec2( 367, 51 )
		);
	}
	ImGui::End( );

	if ( !cloud->user_profile.m_authorized ) {
		user( );
	}
	else {
		ImGui::Begin( _( "CraftOffensive" ), 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration );
		{
			ImGui::SetWindowSize( ImVec2( 660, 475 ), ImGuiCond_Once );

			auto draw = ImGui::GetWindowDrawList( );
			auto pos = ImGui::GetWindowPos( );

			draw->AddRectFilled( pos, pos + ImVec2( 660, 50 ), ImColor( 26, 26, 26 ) );
			draw->AddImage( assets::background, pos + ImVec2( 0, 50 ), pos + ImVec2( 600, 475 ) );
			draw->AddImage( assets::background, pos + ImVec2( 600, 50 ), pos + ImVec2( 1200, 475 ) );

			for ( auto a = 0; a < 6; a++ ) {
				ImGui::SetCursorPos( { float( a * 110 ), 0 } );
				elements::tab( tabLabels[ a ], assets::icons[ a ], m_selected_tab, a );
			}

			draw->AddRectFilled( pos + ImVec2( 0, 50 ), pos + ImVec2( 660, 80 ), ImColor( 65, 65, 65, 210 ) );

			ImGui::SetCursorPos( { 0, 50 } );
			ImGui::BeginGroup( );
			{
				if ( m_selected_tab == 0 ) {
					elements::subtab( _( "Aimbot" ), { 330, 30 }, m_selected_subtab[ 0 ], 0 );
					ImGui::SameLine( );
					elements::subtab( _( "Triggerbot" ), { 330, 30 }, m_selected_subtab[ 0 ], 1 );
				}
				if ( m_selected_tab == 1 ) {
					elements::subtab( _( "ESP" ), { 220, 30 }, m_selected_subtab[ 1 ], 0 );
					ImGui::SameLine( );
					elements::subtab( _( "Chams" ), { 220, 30 }, m_selected_subtab[ 1 ], 1 );
					ImGui::SameLine( );
					elements::subtab( _( "Glow" ), { 220, 30 }, m_selected_subtab[ 1 ], 2 );
				}
				if ( m_selected_tab == 4 ) {
					elements::subtab( _( "Configurations" ), { 660, 30 }, m_selected_subtab[ 4 ], 0 );
				}
			}
			ImGui::EndGroup( );

			static int selected_chams_tab = 0;

			if ( m_selected_tab == 1 && m_selected_subtab[ 1 ] == 1 ) {
				draw->AddRectFilled( pos + ImVec2( 0, 80 ), pos + ImVec2( 660, 110 ), ImColor( 25, 25, 25, 210 ) );

				ImGui::SetCursorPos( { 0, 80 } );
				ImGui::BeginGroup( );
				{
					elements::subtab( _( "Local Visible" ), { 110, 30 }, selected_chams_tab, 0 );
					ImGui::SameLine( );
					elements::subtab( _( "Local Invisible" ), { 110, 30 }, selected_chams_tab, 1 );
					ImGui::SameLine( );
					elements::subtab( _( "Enemy Visible" ), { 110, 30 }, selected_chams_tab, 2 );
					ImGui::SameLine( );
					elements::subtab( _( "Enemy Invisible" ), { 110, 30 }, selected_chams_tab, 3 );
					ImGui::SameLine( );
					elements::subtab( _( "Team Visible" ), { 110, 30 }, selected_chams_tab, 4 );
					ImGui::SameLine( );
					elements::subtab( _( "Team Invisible" ), { 110, 30 }, selected_chams_tab, 5 );
				}
				ImGui::EndGroup( );
			}

			ImGui::SetCursorPos( { 15, ( m_selected_tab == 1 && m_selected_subtab[ 1 ]  == 1) ? 125.f : 95.f } );
			ImGui::BeginGroup( );
			{
				/*if ( g_Model.get_preview_texture( ) != nullptr )
				{
					ImGui::GetForegroundDrawList( )->AddImage(
						g_Model.get_preview_texture( )->m_handles[ 0 ]->m_texture,
						ImGui::GetWindowPos( ),
						ImGui::GetWindowPos( ) + ImVec2( g_Model.get_preview_texture( )->get_actual_width( ), g_Model.get_preview_texture( )->get_actual_height( ) ),
						ImVec2( 0, 0 ), ImVec2( 1, 1 ),
						ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

				}*/

				if ( m_selected_tab == 0 ) {
					elements::child( _( "General" ), { 220, 500 }, [ ] ( ) {
						elements::checkbox( _( "auto_jump" ), FNV1A( "auto_jump" ) );

						elements::checkbox( _( "fake_lags enable" ), FNV1A( "fakelags.enable" ) );
						elements::slider_int( _( "fake lags amount" ), FNV1A( "fakelags.amount" ), 0, 14, "Slider Value: %i" );

						elements::checkbox( _( "viewmodel enable" ), FNV1A( "misc.view_model.enable" ) );
						elements::slider_float( _( "viewmodel x" ), FNV1A( "misc.view_model.x" ), -50, 50, "viewmodel x: %0.1f" );
						elements::slider_float( _( "viewmodel y" ), FNV1A( "misc.view_model.y" ), -50, 50, "viewmodel y: %0.1f" );
						elements::slider_float( _( "viewmodel z" ), FNV1A( "misc.view_model.z" ), -50, 50, "viewmodel z: %0.1f" );


						elements::checkbox( _( "override view model fov  when scoped" ), FNV1A( "misc.view_model.override_while_scoped" ) );
						elements::slider_int( _( "viewmodel fov" ), FNV1A( "misc.view_model.fov" ), 60, 120, "viewmodel fov: %i" );

						

						if ( elements::button( _( "Button" ), ImVec2( 200, 30 ) ) )
						{
							cloud->get_configs( );
							notifies::push( "CraftOffensive.pw", "Coming soon..." );
						}
					} );
				}

				if ( m_selected_tab == 1 ) 
				{
					if ( m_selected_subtab[ 1 ] == 1 ) 
					{
						if ( selected_chams_tab == 0 )
							chams_page( cfg::local_player_visible );
						if ( selected_chams_tab == 1 )
							chams_page( cfg::local_player_invisible );
						if ( selected_chams_tab == 2 )
							chams_page( cfg::enemy_visible );
						if ( selected_chams_tab == 3 )
							chams_page( cfg::enemy_invisible );
						if ( selected_chams_tab == 4 )
							chams_page( cfg::teammates_visible );
						if ( selected_chams_tab == 5 )
							chams_page( cfg::teammates_invisible );
					}
				}

				if ( m_selected_tab == 4 ) {
					elements::child( _( "Actions" ), { 190, 365 }, [ ] ( ) {
						if ( elements::button( _( "Refresh configurations" ), ImVec2( 170, 30 ) ) )
							cloud->get_configs( );

						ImGui::Separator( );

						static char config_name[ 24 ];
						ImGui::InputText( _("Configuration name" ), config_name, 24 );

						if ( elements::button( _( "Create configuration" ), ImVec2( 170, 30 ) ) )
							if( !std::string( config_name ).empty() )
								cloud->create_config( config_name );
							else 
								notifies::push( "Cloud Configs", "Configure name is too short" );

					} );

					ImGui::SameLine( 201 );

					elements::child( _( "List" ), { 430, 365 }, [ ] ( ) {
						if ( cloud->user_configs.empty( ) ) {
							auto drawchild = ImGui::GetWindowDrawList( );
							auto poschild = ImGui::GetWindowPos( );

							drawchild->AddImage( assets::creeper, poschild + ImVec2( 165, 90 ), poschild + ImVec2( 245, 170 ));
							drawchild->AddText( poschild + ImVec2( 205 - ImGui::CalcTextSize( "Nothing to show ;(" ).x / 2, 190 ), ImColor( 255, 255, 255 ), "Nothing to show ;(" );
						}
						else {
							for ( auto conf : cloud->user_configs ) {
								elements::config( conf );
							}
						}
					} );
				}
			}
			ImGui::EndGroup( );
		}
		ImGui::End( );
	}
}