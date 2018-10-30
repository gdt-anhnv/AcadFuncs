#ifndef _USER_FUNCS_H_
#define _USER_FUNC_H_

#include "../acad_header.h"

class UserFuncs
{
public:

	static AcDbObjectIdArray GetIdsFromSelectionSet(const ads_name&);
	static AcDbObjectIdArray UserGetEnts();
	static AcDbObjectIdArray GetEntsFromSS(const ads_name& ss);
	static int GetInt(std::wstring promp);
	static AcGePoint3d UserGetPoint(std::wstring prompt);
	static AcGePoint3d UserGetPointByPoint(std::wstring prompt, const AcGePoint3d & base_point);

};


#endif // !_USER_FUNCS_H_


