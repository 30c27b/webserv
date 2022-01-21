/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Res.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vneirinc <vneirinc@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/18 14:13:56 by vneirinc          #+#    #+#             */
/*   Updated: 2022/01/21 16:51:04 by vneirinc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include "res_header.h"
#include "../shared/Buffer.hpp"
#include "../net/Connection.hpp"

namespace http
{

	class Res
	{
	private:
		std::string		_statusMsg;
		std::string		_contentType;
		size_t			_contentLength;

		void	_defaultRes(std::string& buff) const;
		void	_getDate(std::string& buff) const;

	public:
		Res(void);

		void	sendRes(ws::net::Connection& conn) const;

		void	setStatus(const std::string& statusMsg);
	};
}