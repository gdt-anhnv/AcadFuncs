#include "UserFuncs.h"
#include "AcadFuncs.h"
#include "ARXFuncs.h"
#include "../wrap_header.h"

AcDbObjectIdArray UserFuncs::GetIdsFromSelectionSet(const ads_name & an)
{
	Adesk::Int32 len = 0;
	if (RTNORM != acedSSLength(an, &len))
		throw std::exception("Failed when get length of selection set");

	AcDbObjectIdArray ids = AcDbObjectIdArray();
	for (int i = 0; i < len; i++)
	{
		ads_name tmp = { 0, 0 };
		if (RTNORM != acedSSName(an, i, tmp))
		{
			acedSSFree(tmp);
			throw std::exception("Could not get selection properly");
		}

		AcDbObjectId tmp_id = AcDbObjectId::kNull;
		if (Acad::eOk == acdbGetObjectId(tmp_id, tmp))
			ids.append(tmp_id);

		acedSSFree(tmp);
	}

	return ids;
}

AcDbObjectIdArray UserFuncs::UserGetEnts()
{
	AdsNameWrap ads_wrap;
	// Get the current PICKFIRST or ask user for a selection
	acedSSGet(NULL, NULL, NULL, NULL, ads_wrap.ads);
	// Get the length (how many entities were selected)

	return GetEntsFromSS(ads_wrap.ads);
}

AcDbObjectIdArray UserFuncs::GetEntsFromSS(const ads_name & ss)
{
	ads_name ent_name;
	AcDbObjectId id = AcDbObjectId::kNull;

	AcDbObjectIdArray ids = AcDbObjectIdArray();
	Adesk::Int32 length = 0;
	if (RTNORM != acedSSLength(ss, &length))
		return ids;
	// Walk through the selection set and open each entity
	for (long i = 0; i < length; i++)
	{
		if (RTNORM != acedSSName(ss, i, ent_name))
			continue;

		if (Acad::eOk != acdbGetObjectId(id, ent_name))
			continue;

		ids.append(id);
	}
	return ids;
}

int UserFuncs::GetInt(std::wstring promp)
{
	int value = 0;
	int rc = acedGetInt(promp.c_str(), &value);
	switch (rc)
	{
	case RTCAN:
		throw RTCAN;
	case RTERROR:
		acutPrintf(L"\nInvalid number!");
		break;
	case RTNONE:
		// defaul table
		return int();
		break;
	case RTNORM:
		return value;
		break;
	}
}

AcGePoint3d UserFuncs::UserGetPoint(std::wstring prompt)

{
	ads_point ads;

	int chk = acedGetPoint(NULL, prompt.c_str(), ads);
	if (chk != RTNORM)
	{
		acutPrintf(L"Something incorrect!");
		throw RTCAN;
	}
	return ARXFuncs::ConvertAdsToPoint(ads);
}

AcGePoint3d UserFuncs::UserGetPointByPoint(std::wstring prompt, const AcGePoint3d & base_point)
{
	double ads_pnt[3];
	ads_point tmp_first;
	tmp_first[0] = base_point.x;
	tmp_first[1] = base_point.y;
	tmp_first[2] = base_point.z;
	ads_point tmp_second;

	int rc = acedGetPoint(tmp_first, prompt.c_str(), tmp_second);

	switch (rc)
	{
	case RTCAN:
		throw RTCAN;
	case RTERROR:
		acutPrintf(L"\nInvalid selection!");
		break;
	case RTNONE:
		// defaul table
		return AcGePoint3d::kOrigin;
		break;
	case RTNORM:
	{
		return AcGePoint3d(tmp_second[0], tmp_second[1], 0.0);
	}
	break;
	}
}
