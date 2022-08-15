#pragma once
#include "../../dormant/dormant.hpp"
namespace shared_esp
{
	bool __stdcall handler( size_t message_size, const char* message )
	{
		SharedESP esp_message;

		/* parse message*/
		esp_message.ParseFromArray( message, message_size );

		/* iterate all players data*/
		for ( int i = 0; i < esp_message.players_size( ); i++ )
		{

			/* get player from index */
			auto player = reinterpret_cast<c_cs_player*> ( interfaces::m_entity_list->get_client_entity( esp_message.players( ).Get( i ).player_index( ) ) ) ;

			if ( !player || !player->is_dormant( ) || player->is_alive( ) )
				continue;

			vec3_t origin = vec3_t( esp_message.players( ).Get( i ).origin_x( ), esp_message.players( ).Get( i ).origin_y( ), esp_message.players( ).Get( i ).origin_z( ) );

			if ( origin.empty( )  || !origin.is_valid( ) )
				continue;

			dormant->on_shared_esp_received( esp_message.players( ).Get( i ).player_index( ), origin, esp_message.players( ).Get( i ).health( ) );

			
		}
		return true;
	}

	void SendData( )
	{
		if ( !globals::m_local )
			return;

		SharedESP message;

		for ( int i = 0; i < interfaces::m_global_vars->m_max_clients; i++ )
		{
			auto player = reinterpret_cast< c_cs_player* > ( interfaces::m_entity_list->get_client_entity( i ) );

			if ( !player || !player->is_player( ) || player->is_dormant( ) || !player->is_alive( ) )
				continue;

			auto shared_player = message.add_players( );

			shared_player->set_player_index( i );
			shared_player->set_health( player->get_health( ) );
			shared_player->set_origin_x( player->get_origin( ).x );
			shared_player->set_origin_y( player->get_origin( ).y );
			shared_player->set_origin_z( player->get_origin( ).z );

		}
		shared->m_socket->SendMessageToAll( steamsockets::message_type_t::SHARED_ESP, &message );
	}
}