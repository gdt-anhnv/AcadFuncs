#include "res_buf_wrap.h"
#include "../acad_header.h"

ResBufWrap::ResBufWrap(resbuf * rb) :
	res_buf(rb)
{
}

ResBufWrap::~ResBufWrap()
{
	if (NULL != res_buf)
		acutRelRb(res_buf);
}
