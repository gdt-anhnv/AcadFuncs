#include "ads_name_wrap.h"

AdsNameWrap::AdsNameWrap() :
	ads()
{
}

AdsNameWrap::~AdsNameWrap()
{
	acedSSFree(ads);
}
