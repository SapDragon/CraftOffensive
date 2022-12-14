#pragma once
#include "../globals.h"
#include "../features/features.h"

namespace hooks {
	void init();

	void undo();

	inline unsigned int get_virtual(void* _class, unsigned int index) { return static_cast<unsigned int>((*static_cast<int**>(_class))[index]); }

	namespace d3d_device {
		namespace reset {
			long __stdcall hook(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* present_params);
			using fn = long(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
		}

		namespace present {
			long __stdcall hook(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region);
			using fn = long(__stdcall*)(IDirect3DDevice9*, RECT*, RECT*, HWND, RGNDATA*);
		}
	}

	namespace client_dll {
		namespace frame_stage_notify {
			void __stdcall hook(e_client_frame_stage stage);
			using fn = void(__stdcall*)(e_client_frame_stage);
		}

		namespace create_move {
			void __stdcall hook(int sequence_number, float sample_frametime, bool active, bool& packet);
			void __stdcall gate(int sequence_number, float sample_frametime, bool active);
			using fn = void(__stdcall*)(int, float, bool);
		}

		namespace level_init_pre_entity
		{
				void __fastcall hook( void* ecx, void* edx, const char* map_name );
				using fn = void( __thiscall* )( void*, const char* );
			
		}
	}

	namespace client_mode {
		namespace override_view {
			void __stdcall hook(view_setup_t* view);
			using fn = void(__stdcall*)(view_setup_t*);
		}
	}

	namespace model_render {
		namespace draw_model_execute {
			void __fastcall hook(i_model_render* ecx, void* edx, void* context, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bones);
			using fn = void(__thiscall*)(i_model_render*, void*, const draw_model_state_t&, const model_render_info_t&, matrix3x4_t*);
		}
	}

	namespace panel {
		namespace paint_traverse {
			void __fastcall hook(void* ecx, void* edx, uint32_t id, bool force_repaint, bool allow_force);
			using fn = void(__thiscall*)(void*, uint32_t, bool, bool);
		}
	}

	namespace surface {
		namespace lock_cursor {
			void __fastcall hook(i_surface* ecx, void* edx);
			using fn = void(__fastcall*)(i_surface*, void*);
		}
	}

	namespace player {
		namespace eye_angles {
			qangle_t* __fastcall hook(void* ecx, void* edx);
			using fn = qangle_t* (__fastcall*)(void*, void*);
		}
	}

	namespace networking
	{
		namespace send_datagram
		{
			int __fastcall hook( void* ecx, uint32_t edx, void* data );
			using fn = int ( __fastcall* )( void*, uint32_t, void* );

		}

		namespace packet_end
		{
			void __fastcall hook( void* ecx, void* edx );
			using fn = void( __fastcall* )( void*, void* );
		}

		namespace cl_move
		{
			void __cdecl hook( float frame_time, bool final_tick );
			using fn = void( __cdecl* )( float, bool );
		}
	}

	namespace other
	{
		namespace process_spotted_entity_update
		{
			bool __fastcall hook( void* ecx, void* edx, c_process_spotted_entity_update* message);
			using fn = bool( __fastcall* )( void*, void*, c_process_spotted_entity_update* );

		}

		namespace allocate_memory
		{
			void* __fastcall hook( void* ecx, void* edx, int size );
			using fn = void* ( __fastcall* )( void*, void*, int );

		}

		namespace loose_file_allowed
		{
			bool __fastcall hook( void* ecx, void* edx );
			using fn = bool ( __fastcall* )( void*, void* );
		}

		namespace get_unverified_file_hashes
		{
			int __stdcall hook( void* this_pointer, int max_files );
			using fn = int( __stdcall* )( void*, int* );
		}
	}

	namespace mdl_cache
	{
		namespace find_mdl
		{
			unsigned short __fastcall hook( void* ecx, void* edx, char* path );
			using fn = unsigned short ( __thiscall* )( void*, char* path );
		}
	}

	namespace engine_dll
	{
		namespace host_run_frame_input
		{
			void __cdecl hook( void*, void*, float accumulated_extra_samples, bool final_tick );
			using fn = void( __cdecl* )( float, bool );
		}

		namespace get_player_info
		{
			void __fastcall hook( void* ecx, void* edx, int index, player_info_t* info );
			using fn = unsigned short( __fastcall* )( void*, void*, int, player_info_t* );
		}
	}

	inline hooks::d3d_device::present::fn present_original = nullptr;
	inline hooks::d3d_device::reset::fn reset_original = nullptr;
	inline hooks::client_dll::frame_stage_notify::fn frame_stage_notify_original = nullptr;
	inline hooks::client_dll::create_move::fn create_move_original = nullptr;
	inline hooks::client_mode::override_view::fn override_view_original = nullptr;
	inline hooks::model_render::draw_model_execute::fn draw_model_execute_original = nullptr;
	inline hooks::panel::paint_traverse::fn paint_traverse_original = nullptr;
	inline hooks::player::eye_angles::fn eye_angles_original = nullptr;
	inline hooks::surface::lock_cursor::fn lock_cursor_original = nullptr;
	inline hooks::networking::send_datagram::fn send_datagram_original = nullptr;
	inline hooks::networking::packet_end::fn packet_end_original = nullptr;
	inline hooks::other::process_spotted_entity_update::fn process_spotted_entity_update_original = nullptr;
	inline hooks::other::allocate_memory::fn allocate_memory_original = nullptr;
	inline hooks::engine_dll::host_run_frame_input::fn host_run_frame_input_original = nullptr;
	inline hooks::mdl_cache::find_mdl::fn find_mdl_original = nullptr;
	inline hooks::networking::cl_move::fn cl_move_original = nullptr;
	inline hooks::client_dll::level_init_pre_entity::fn level_init_pre_entity_original = nullptr;
	inline hooks::other::loose_file_allowed::fn loose_file_allowed_original = nullptr;
	inline hooks::engine_dll::get_player_info::fn get_player_info_original = nullptr;

}
