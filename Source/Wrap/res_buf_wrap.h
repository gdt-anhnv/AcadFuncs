#ifndef _RES_BUF_WRAP_H_
#define _RES_BUF_WRAP_H_

struct resbuf;

struct ResBufWrap
{
	resbuf* res_buf;
	ResBufWrap(resbuf*);
	~ResBufWrap();
};

#endif