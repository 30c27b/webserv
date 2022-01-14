#include "Socket.hpp"

namespace ws
{
	namespace net
	{
		Socket::Socket(int fd)
			: _sockfd(fd)
		{}

		Socket::~Socket()
		{
			this->close();
		}

		void Socket::close()
		{
			::close(this->_sockfd);
		}

		int Socket::get_fd()
		{
			return this->_sockfd;
		}
	}
}
