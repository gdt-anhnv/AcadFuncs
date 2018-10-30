#include "ARXFuncs.h"
#include "../wrap_header.h"
#include "DBObject.h"
#include "Geometry.h"
#include "AcadFuncs.h"
#include "acedCmdNF.h"


AcDbObjectIdArray ARXFuncs::GetEntsInsidePolyline(const AcDbObjectId & id)
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AcGePoint3dArray pts = AcGePoint3dArray();

	{
		ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(id));
		if (obj_wrap.object->isKindOf(AcDbPolyline::desc()))
		{
			AcDbPolyline* pl = AcDbPolyline::cast(obj_wrap.object);
			if (pl->isClosed())
			{
				pts = GeoFuncs::GetListVertexOfPolyline(pl);
			}
			else
			{
				return ids;
			}
		}
	}

	ZoomIntoZone(id);
	if (pts.isEmpty())
	{
		return ids;
	}

	ads_point pt;
	resbuf *pnts = acutBuildList(RT3DPOINT, asDblArray(pts[0]));
	for (int i = 1; i < pts.length(); i++)
	{
		pnts = Functions::AppendToResbuf(pnts,
			acutBuildList(
				RT3DPOINT, asDblArray(pts[i]), // Next point
				RTNONE)
		);
	}
	AdsNameWrap cp_wrap;
	int ret = acedSSGet(L"CP", pnts, NULL, NULL, cp_wrap.ads);
	acutRelRb(pnts);
	Adesk::Int32 num_ent = 0;
	ret = acedSSLength(cp_wrap.ads, &num_ent);
	if (RTNORM != ret)
		return ids;

	for (int i = 0; i < num_ent; i++)
	{
		ads_name tmp = { 0, 0 };
		if (RTNORM != acedSSName(cp_wrap.ads, i, tmp))
			continue;
		AcDbObjectId tmp_id = AcDbObjectId::kNull;
		if (Acad::eOk == acdbGetObjectId(tmp_id, tmp))
		{
			ids.append(tmp_id);
		}
	}

	return ids;
}

void ARXFuncs::ZoomIntoZone(const AcDbObjectId & id)
{
	ads_name ssname;
	acdbGetAdsName(ssname, id);
	int rtrn = acedCommandS(RTSTR, L"_ZOOM", RTSTR, L"OBJECT", RTENAME, ssname, RTSTR, L"", RTNONE);
	acedSSFree(ssname);
}

void ARXFuncs::ZoomIntoZoneExtent(const AcDbObjectId & id, int exten_val)
{
	ads_point p1 = { 0, 0, 0 };
	ads_point p2 = { 0, 0, 0 };
	if (AcDbObjectId::kNull != id)
	{
		AcDbExtents ex = AcDbExtents();
		ObjectWrap<AcDbObject> obj_w(DBObject::OpenObjectById<AcDbObject>(id));
		ObjectWrap<AcDbEntity> ent_w(DBObject::OpenObjectById<AcDbEntity>(id));
		ent_w.object->getGeomExtents(ex);
		ConvertToAdsPoint(ex.minPoint() + AcGeVector3d(-500, -500, 0), p1);
		ConvertToAdsPoint(ex.maxPoint() + AcGeVector3d(+500, +500, 0), p2);
		if (RTNORM != acedCommandS(RTSTR, L"_ZOOM", RTSTR, L"W", RTPOINT, p1, RTPOINT, p2, RTNONE))
			return;
	}
}


AcDbObjectIdArray ARXFuncs::GeLastCreatedObjId()
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AdsNameWrap anw;
	acedSSGet(L"L", NULL, NULL, NULL, anw.ads);
	AcDbHandle hndl = AcDbHandle(anw.ads[0], anw.ads[1]);
	if (!hndl.isNull())
		acedGetCurrentSelectionSet(ids);
	return ids;
}

AcDbObjectIdArray ARXFuncs::GetObjIdsByPicking()
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AdsNameWrap anw;
	if (RTNORM != acedSSGet(L":S", NULL, NULL, NULL, anw.ads))
		return ids;

	ads_name tmp_ads;
	if (RTNORM != acedSSName(anw.ads, 0L, tmp_ads))
		return ids;

	AcDbObjectId tmp_id = AcDbObjectId::kNull;
	if (Acad::eOk == acdbGetObjectId(tmp_id, tmp_ads))
		ids.append(tmp_id);

	return ids;
}

AcDbObjectIdArray ARXFuncs::GetObjIdsByPicking(wchar_t * prompt)
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AdsNameWrap anw;

	wchar_t* pts[] = { prompt, L"" };

	if (RTNORM != acedSSGet(L"_+.:S:$", pts, NULL, NULL, anw.ads))
		return ids;

	ads_name tmp_ads;
	if (RTNORM != acedSSName(anw.ads, 0L, tmp_ads))
		return ids;

	AcDbObjectId tmp_id = AcDbObjectId::kNull;
	if (Acad::eOk == acdbGetObjectId(tmp_id, tmp_ads))
		ids.append(tmp_id);

	return ids;
}

AcDbObjectIdArray ARXFuncs::GetObjIdsInSelected()
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AdsNameWrap anw;
	acedSSGet(NULL, NULL, NULL, NULL, anw.ads);
	AcDbHandle hndl = AcDbHandle(anw.ads[0], anw.ads[1]);
	if (!hndl.isNull())
		Acad::ErrorStatus err = acedGetCurrentSelectionSet(ids);

	return ids;
}

AcDbObjectIdArray ARXFuncs::GetObjIdsInSelected(wchar_t* prompt)
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AdsNameWrap anw;
	wchar_t* pts[] = { prompt, L"" };
	int ret = acedSSGet(L":$", pts, NULL, NULL, anw.ads);
	AcDbHandle hndl = AcDbHandle(anw.ads[0], anw.ads[1]);
	if (!hndl.isNull())
		Acad::ErrorStatus err = acedGetCurrentSelectionSet(ids);

	return ids;
}

void ARXFuncs::ConvertToAdsPoint(AcGePoint3d pnt, ads_point ads_pnt)
{
	ads_pnt[X] = pnt.x;
	ads_pnt[Y] = pnt.y;
	ads_pnt[Z] = pnt.z;
}

AcGePoint3d ARXFuncs::ConvertAdsToPoint(const ads_point ads)
{
	return AcGePoint3d(ads[0], ads[1], ads[2]);
}

#define MAX_DISTANCE_TENDPROP_TO_SLAB		3000
AcGePoint3d ARXFuncs::GetMinPerpendicularPoint(AcDbPolyline * pl, AcGePoint3d check_pnt)
{
	AcGePoint3d return_pnt = AcGePoint3d::kOrigin;
	AcDbExtents ext = AcDbExtents();
	pl->getGeomExtents(ext);
	double distance = ext.minPoint().distanceTo(ext.maxPoint()) * 1000;
	AcGePoint3dArray pnts = GeoFuncs::GetListVertexOfPolyline(pl);
	pnts.append(pnts.first());
	double length_tmp = 0.0;
	for (int i = 0; i < pnts.length() - 1; i++)
	{
		AcGeLine3d l = AcGeLine3d(pnts.at(i), pnts.at(i + 1));
		AcGeVector3d vec_dir = l.direction().crossProduct(AcGeVector3d::kZAxis);

		AcDbLine * l1 = new AcDbLine(pnts.at(i), pnts.at(i + 1));
		AcDbLine * l_tmp = new AcDbLine(check_pnt, check_pnt + vec_dir.normal()*distance);
		//get leng intersec
		int num_intersec_pnts = 0;
		AcGePoint3dArray intersec_pnts = AcGePoint3dArray();
		double length_intersec = 0.0;
		AcGeTol tol = AcGeTol();
		tol.setEqualPoint(1.0);
		l1->intersectWith(l_tmp, AcDb::Intersect::kOnBothOperands, intersec_pnts);
		if (0 != intersec_pnts.length())
		{
			length_intersec = check_pnt.distanceTo(intersec_pnts.first());
			if (check_pnt.isEqualTo(intersec_pnts.first(), tol))
				return check_pnt + vec_dir * 100;
			if (0 == length_tmp)
			{
				length_tmp = length_intersec;
				return_pnt = intersec_pnts.first() + vec_dir * 100;
			}
			if (length_intersec < length_tmp)
			{
				length_tmp = length_intersec;
				return_pnt = intersec_pnts.first() + vec_dir * 100;
			}
		}
		else
		{
			vec_dir = vec_dir*(-1);
			AcDbLine * l_tmp_2 = new AcDbLine(check_pnt, check_pnt + vec_dir.normal()*distance);
			l1->intersectWith(l_tmp_2, AcDb::Intersect::kOnBothOperands, intersec_pnts);
			if (0 != intersec_pnts.length())
			{
				length_intersec = check_pnt.distanceTo(intersec_pnts.first());
				if (check_pnt.isEqualTo(intersec_pnts.first(), tol))
					return check_pnt + vec_dir * 1000;
				if (0 == length_tmp)
				{
					length_tmp = length_intersec;
					return_pnt = intersec_pnts.first() + vec_dir * 100;
				}
				if (length_intersec < length_tmp)
				{
					length_tmp = length_intersec;
					return_pnt = intersec_pnts.first() + vec_dir * 100;
				}
			}
			l_tmp_2->close();
		}
		l1->close();
		l_tmp->close();
	}
	if (MAX_DISTANCE_TENDPROP_TO_SLAB < length_tmp)
		return check_pnt;
	return return_pnt;
}

void ARXFuncs::MergeIntoPolyline(const AcDbObjectIdArray & ids)
{
	ads_name ads;
	acedSSAdd(NULL, NULL, ads);
	for (int i = 0; i < ids.length(); i++)
	{
		ads_name tmp = { 0, 0 };
		acdbGetAdsName(tmp, ids.at(i));
		acedSSAdd(tmp, ads, ads);
	}

	Adesk::Int32 len = 0;
	acedSSLength(ads, &len);

	int ret = acedCommandS(RTSTR, _T("_PEDIT"),
		RTSTR, _T("M"),
		RTPICKS, ads,
		RTSTR, _T(""),
		RTSTR, _T("Y"),
		RTSTR, _T("JOIN"),
		RTSTR, _T(""),
		RTSTR, _T(""),
		RTNONE);
}

/*two point must belong polyline*/
AcGePoint3dArray ARXFuncs::GetVerticesBetweenTwoPoint(const AcDbObjectId & cable_id, const AcGePoint3d & pnt_1, const AcGePoint3d & pnt_2)
{
	AcGePoint3dArray ret_points = AcGePoint3dArray();
	AcGeTol tol = AcGeTol();
	tol.setEqualPoint(1e-1);
	if (AcDbObjectId::kNull != cable_id)
	{
		AcGePoint3dArray tmp_pnts = AcGePoint3dArray();
		ObjectWrap<AcDbObject> pl_w(DBObject::OpenObjectById<AcDbObject>(cable_id));
		if (NULL != pl_w.object && pl_w.object->isKindOf(AcDbPolyline::desc()))
		{
			AcDbPolyline * pl = AcDbPolyline::cast(pl_w.object);
			ObjectWrap<AcDbPolyline> obj(pl);
			AcGePoint3dArray vertices = GeoFuncs::GetListVertexOfPolyline(pl);
			bool first = true;
			AcGePoint3d end_check = AcGePoint3d::kOrigin;
			for (int i = 0; i < vertices.length() - 1; i++)
			{
				AcGePoint3d ver_1 = vertices.at(i);
				AcGePoint3d ver_2 = vertices.at(i + 1);
				if (first)
				{
					if (GeoFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_1) &&
						GeoFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_2))
						return ret_points;
					if (GeoFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_1) &&
						!GeoFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_2))
						if (first)
						{
							if (ver_1.isEqualTo(pnt_1, tol))
								tmp_pnts.append(vertices.at(i));
							end_check = pnt_2;
							first = false;
						}
					if (GeoFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_2) &&
						!GeoFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_1))
						if (first)
						{
							if (ver_1.isEqualTo(pnt_2, tol))
								tmp_pnts.append(vertices.at(i));
							end_check = pnt_1;
							first = false;
						}
				}
				else
				{
					tmp_pnts.append(vertices.at(i));
					if (GeoFuncs::IsPointOnStraightline(ver_1, ver_2, end_check))
					{
						tmp_pnts.append(vertices.at(i + 1));
						break;
					}
				}
			}
			if (0 == tmp_pnts.length())
			{
				acutPrintf(_T("two point not belong polyline"));
				return ret_points;
			}
			else if (2 == tmp_pnts.length())
			{
				for (int i = 0; i < tmp_pnts.length(); i++)
					if (!tmp_pnts.at(i).isEqualTo(pnt_1, tol) && !tmp_pnts.at(i).isEqualTo(pnt_2, tol))
						ret_points.append(tmp_pnts.at(i));
				return ret_points;
			}
			else if (3 <= tmp_pnts.length())
			{
				tmp_pnts.removeFirst();
				tmp_pnts.removeLast();

				for (int i = 0; i < tmp_pnts.length(); i++)
					if (!tmp_pnts.at(i).isEqualTo(pnt_1, tol) && !tmp_pnts.at(i).isEqualTo(pnt_2, tol))
						ret_points.append(tmp_pnts.at(i));
				return ret_points;
			}
		}
	}
	return AcGePoint3dArray();
}




AcDbObjectIdArray ARXFuncs::GetObjIdsInWindow(const AcGePoint3d& pnt1, const AcGePoint3d& pnt2)
{
	ads_point p1 = { 0, 0, 0 };
	ads_point p2 = { 0, 0, 0 };

	ConvertToAdsPoint(pnt1, p1);
	ConvertToAdsPoint(pnt2, p2);

	AcDbObjectIdArray ids = AcDbObjectIdArray();
	{
		AdsNameWrap anw;
		int err = acedSSGet(L"C", p1, p2, NULL, anw.ads);
		if (RTNORM != err)
			return ids;

		Adesk::Int32 len = 0;
		if (RTNORM != acedSSLength(anw.ads, &len))
			return ids;

		Acad::ErrorStatus eStat = acedGetCurrentSelectionSet(ids);
	}
	return ids;
}

void ARXFuncs::DrawOrderFront(AcDbObjectIdArray ids)
{
	ads_name ads;
	acedSSAdd(NULL, NULL, ads);
	for (int i = 0; i < ids.length(); i++)
	{
		ads_name tmp = { 0, 0 };
		acdbGetAdsName(tmp, ids.at(i));
		acedSSAdd(tmp, ads, ads);
	}

	Adesk::Int32 len = 0;
	acedSSLength(ads, &len);

	int ret = acedCommandS(RTSTR, _T("_DRAWORDER"),
		RTPICKS, ads,
		RTSTR, _T(""),
		RTSTR, _T("Front"),
		RTSTR, _T(""),
		RTSTR, _T(""),
		RTNONE);
}

void ARXFuncs::DrawOrderBot(AcDbObjectIdArray ids)
{
	ads_name ads;
	acedSSAdd(NULL, NULL, ads);
	for (int i = 0; i < ids.length(); i++)
	{
		ads_name tmp = { 0, 0 };
		acdbGetAdsName(tmp, ids.at(i));
		acedSSAdd(tmp, ads, ads);
	}

	Adesk::Int32 len = 0;
	acedSSLength(ads, &len);

	int ret = acedCommandS(RTSTR, _T("_DRAWORDER"),
		RTPICKS, ads,
		RTSTR, _T(""),
		RTSTR, _T("Back"),
		RTSTR, _T(""),
		RTSTR, _T(""),
		RTNONE);
}

void ARXFuncs::EraseObjects(AcDbObjectIdArray ids)
{
	Acad::ErrorStatus err;
	for (int i = 0; i < ids.length(); i++)
	{
		if (AcDbObjectId::kNull != ids.at(i))
		{
			ObjectWrap<AcDbEntity> ent_wrap(DBObject::OpenObjectById<AcDbEntity>(ids.at(i)));
			if ((NULL != ent_wrap.object) && (!ent_wrap.object->isErased()))
			{
				err = ent_wrap.object->upgradeOpen();
				err = ent_wrap.object->erase();
			}
		}
	}
}
