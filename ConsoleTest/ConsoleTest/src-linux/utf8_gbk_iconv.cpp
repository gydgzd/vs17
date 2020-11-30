/*
 * utf8_gbk_iconv.cpp
 *
 *  Created on: 2018��11��9��
 *      Author: gyd
 */
#include <stdlib.h>
#include <iconv.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>


int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	iconv_t cd;
	char *p_in = inbuf;
	char *p_out = outbuf;


	cd = iconv_open(to_charset, from_charset);
	if (cd == (iconv_t) -1)
		return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, &p_in, &inlen, &p_out, &outlen) == ((size_t) -1))
	{
		iconv_close(cd);
		return -1;
	}

	iconv_close(cd);
	*p_out = '\0';
	return 0;
}
int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
}


int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
}


