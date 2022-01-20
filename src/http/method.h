/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   method.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vneirinc <vneirinc@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/18 10:18:12 by vneirinc          #+#    #+#             */
/*   Updated: 2022/01/20 13:33:05 by vneirinc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef METHOD_H
# define METHOD_H
# define HTTPVER "HTTP/1.1"
# define METHODS {"GET ", "POST ", "PUT ", "DELETE "}

enum e_method
{
	UNDEF = -1,
	GET,
	POST,
	PUT,
	DELETE
};

#endif
