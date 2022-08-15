#pragma once
#include "../../globals.h"
#include "../../utils/steam_sockets/steam_sockets.hpp"

class c_shared : public c_singleton<c_shared>
{
public:
	void init( );

	steamsockets::c_socket_netchannel* m_socket;
};

#define shared c_shared::instance()