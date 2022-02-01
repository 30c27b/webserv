/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vneirinc <vneirinc@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/20 17:40:33 by vneirinc          #+#    #+#             */
/*   Updated: 2022/02/01 14:07:21 by vneirinc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include "index_of.h"
#include "../cgi/Launcher.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

namespace ws
{
	namespace core
	{
		bool	_hasBody(e_method method) { return method == POST || method == PUT; }

		Router::Router(const conf::Config& conf) : _config(conf) {}

		http::Res
		Router::process(http::Req& request, const conf::host_port& host) const
		{
			http::Res				response;
			const conf::Server*		serv;
			const conf::Location*	loc;
			
			if ((serv = this->_getServ(request.header(), host)))
			{
				if ((loc = this->_getLocation(request.path(), *serv)))
					this->_processServ(request, response, *loc, host);
				else
					this->_processServ(request, response, *serv, host);
			}
			else
				this->_setError(response, *serv, STATUS444, 444);
			return response;
		}

		const conf::Server*
		Router::_getServ(http::Req::header_m& header, const conf::host_port& host) const
		{
			conf::server_map::const_iterator it = this->_config.servers.find(host);

			if (it == this->_config.servers.end() || it->second.empty())
				return nullptr;
			return _getServerName(header["host"], it->second);
		}

		const conf::Server*
		Router::_getServerName(const std::string& host, const serv_lst& servLst) const
		{
			if (servLst.size() > 1 && !host.empty())
			{
				CI_Less	ci;

				for (serv_lst::const_iterator it = servLst.begin(); it != servLst.end(); ++it)
					for (std::vector<std::string>::const_iterator iter = it->server_names.begin(); iter != it->server_names.end(); ++iter)
						if (!ci(*iter, host) && !ci(host, *iter))
							return it.base();
			}
			return servLst.begin().base();
		}

		const conf::Location*
		Router::_getLocation(const std::string& uri, const conf::Server& serv) const
		{
			size_t	res;

			for (conf::location_v::const_iterator it = serv.locations.begin();
				it != serv.locations.end(); ++it)
			{
				res = uri.find(it->route);
				if (res == 0)
					if (!it->cgi_ext.size()
					|| uri.find_last_of(it->cgi_ext) == uri.size() - it->cgi_ext.size())
						return it.base();
			}
			return NULL;
		}

		void
		Router::_setError(
			http::Res& response,
			const conf::ServConfig& serv,
			const char* str,
			const uint16_t code) const
		{
			const std::string*	errorPage;

			response.setStatus(str);
			errorPage = this->_findErrorPage(serv, code);
			if (errorPage != NULL)
				response.body().join(*errorPage);
		}

		const std::string*
		Router::_findErrorPage(const conf::ServConfig& serv, const uint16_t code) const
		{
			conf::ErrorPages::const_iterator it = serv.error_pages.find(code);

			if (it == serv.error_pages.end())
				return NULL;
			return it->second;
		}

		void
		Router::_processServ(
			const http::Req& request,
			http::Res& response,
			const conf::ServConfig& mainConf,
			const conf::host_port& host) const
		{
			if (_hasBody(request.method()) && mainConf.upload_path.size())
				this->_upload(request, response, mainConf);
			else if (mainConf.cgi_ext.size())
				response = cgi::Launcher(request, host.first, host.second, mainConf.cgi_script, mainConf.cgi_ext).run();
			else
				this->_renderPage(request, response, mainConf);
		}

		void
		Router::_upload(
			const http::Req& request,
			http::Res& response,
			const conf::ServConfig& mainConf) const
		{
			if (!this->_writeFile(
				mainConf.upload_path + (request.path().c_str() + mainConf.route.size()),
				request.body()
			))
				return this->_setError(response, mainConf, STATUS403, 403);
			response.setStatus(STATUS201);
		}

		bool	
		Router::_writeFile(const std::string& path, const shared::Buffer& buff) const
		{
			std::ofstream	file(path);

			if (file)
			{
				file << buff.get_ptr();
				file.close();
				return true;
			}
			return false;
		}

		void
		Router::_renderPage(
			const http::Req& request,
			http::Res& response,
			const conf::ServConfig& mainConf) const
		{
			std::string				path;
			shared::Buffer			body;

			path = _getLocalPath(request.path(), mainConf);
			if (path.empty())
				return this->_setError(response, mainConf, STATUS404, 404);
			if (std::find(mainConf.accepted_methods.begin(), mainConf.accepted_methods.end(), request.method())
				== mainConf.accepted_methods.end())
				return this->_setError(response, mainConf, STATUS405, 405);
 			if (!(body = this->_getBody(path, request.path())).size())
				return this->_setError(response, mainConf, STATUS403, 403);
			if (!this->_checkMaxBodySize(mainConf, request.body().size()))
				return this->_setError(response, mainConf, STATUS413, 413);
			response.body().join(body);
		}

		std::string
		Router::_getLocalPath(
			const std::string& uri,
			const conf::ServConfig& serv) const
		{
			std::string	path;
			struct stat	info;

			path = serv.root;
			path += (uri.c_str() + serv.route.size());
			if (path.back() == '/')
				path.pop_back();
			if (stat(path.c_str(), &info) != 0)
				return std::string();
			if (S_ISDIR(info.st_mode))
			{
				path.push_back('/');
				if (!serv.autoindex)
				{
					path.append(serv.index);
					if (stat(path.c_str(), &info) != 0)
						return std::string();
				}
			}
			return path;
		}

		shared::Buffer
		Router::_getBody(const std::string& path, const std::string& uri) const
		{
			if (path.back() == '/') // autoindex on case
			{
				shared::Buffer	buff;
    			DIR* dirp = opendir(path.c_str());
				if (dirp)
				{
					buff = this->_getAutoIndexPage(dirp, uri);
					closedir(dirp);
				}
				return buff;
			}
			return this->_getFile(path);
		}

		shared::Buffer
		Router::_getAutoIndexPage(DIR* dirp ,const std::string& uri) const
		{
			char	_buff[512];
			size_t	size;

			size = sprintf(_buff, INDEX_OF1, uri.c_str(), uri.c_str());

			shared::Buffer	buff(_buff, size);

			const std::vector<struct dirent>	dirList = this->_getDirList(dirp);

			for (std::vector<struct dirent>::const_iterator it = ++dirList.begin(); it != dirList.end(); ++it)
			{
				if (it->d_type == DT_DIR)
					size = sprintf(_buff, DIR_TEMP, uri.c_str(), it->d_name, it->d_name);
				else
					size = sprintf(_buff, FILE_TEMP, uri.c_str(), it->d_name, it->d_name);
				buff.join(shared::Buffer(_buff, size));
			}
			buff.join(shared::Buffer(INDEX_OF2));
			return buff;
		}

		const std::vector<struct dirent>
		Router::_getDirList(DIR* dirp) const
		{
			std::vector<struct dirent>	v;
    		struct dirent*				dp;

    		while ((dp = readdir(dirp)) != NULL)
				v.push_back(*dp);
			return v;
		}

		shared::Buffer
		Router::_getFile(const std::string& path) const
		{
			std::ifstream		file(path);
			shared::Buffer		buff;

			if (file)
			{
				char	_buff[2048];
				size_t	len;

				while (!file.eof())
				{
					file.read(_buff, 2048);
					len = file.gcount();
					buff.join(shared::Buffer(_buff, len));
				}
				file.close();
			}
			return buff;
		}

		bool
		Router::_checkMaxBodySize(const conf::ServConfig& serv, size_t bodySize) const
		{
			if (serv.max_body_size != -1)
				return bodySize <= static_cast<size_t>(serv.max_body_size);
			return true;
		}
	}
}
