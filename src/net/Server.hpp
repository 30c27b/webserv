#ifndef WS_NET_SERVER_HPP
# define WS_NET_SERVER_HPP

# include <string>
# include <iostream>

# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <fcntl.h>

# include "shared/Log.hpp"
# include "Socket.hpp"
# include "Connection.hpp"

namespace ws
{
	namespace net
	{
		class Server: public Socket
		{
		public:

			Server(std::string host = "localhost", in_port_t port = 0, int backlog = 500);

			void listen();

			Connection accept();

			std::string get_host();
			in_port_t get_port();
		
		private:

			std::string _host;
			in_port_t _port;
			int _backlog;

		};
	}
}

#endif
