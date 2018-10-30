#include "Geometry.h"
#include "AcadFuncs.h"
#include "DBObject.h"

static AcDbObjectId DrawPolyline(AcDbDatabase* db, AcGePoint3dArray pnts)
{
	AcDbObjectId id = AcDbObjectId::kNull;

	ObjectWrap<AcDbPolyline> pline(new AcDbPolyline());

	for (int i = 0; i, pnts.length(); i++)
	{
		pline.object->addVertexAt(i, AcGePoint2d(pnts.at(i).x, pnts.at(i).y));
	}

	{
		ObjectWrap<AcDbBlockTableRecord> rcd_wrap(DBObject::GetModelSpace(db));
		rcd_wrap.object->upgradeOpen();
		rcd_wrap.object->appendAcDbEntity(id, pline.object);
	}

	return id;
}

double GeoFuncs::GetLengthPolyline(const AcDbObjectId & id)
{
	if (AcDbObjectId::kNull != id)
	{
		double length = 0.0;
		AcDbPolyline* pl = DBObject::OpenObjectById<AcDbPolyline>(id);
		ObjectWrap<AcDbPolyline> pl_wrap(pl);

		if (NULL == pl)
			throw std::exception("Could not get the length of the polyline");

		double paramEnd;
		pl->getEndParam(paramEnd);
		pl->getDistAtParam(paramEnd, length);

		return length;
	}
	else
		throw std::exception("Could not get the length of the polyline");
}

double GeoFuncs::GetLengthPolyline(const AcDbObjectId & id, const AcGePoint3d & vertex1, const AcGePoint3d & vertex2)
{
	if (AcDbObjectId::kNull != id)
	{
		double length = 0.0;
		AcDbPolyline* pl = DBObject::OpenObjectById<AcDbPolyline>(id);
		if (NULL != pl)
		{
			AcGePolyline3d pl1 = AcGePolyline3d(GetListVertexOfPolyline(pl));
			double param1 = pl1.paramOf(vertex1);
			double param2 = pl1.paramOf(vertex2);
			length = abs(pl1.length(param1, param2));
		}
		return length;
	}
	else
		throw std::exception("Could not get the length of the polyline");
}

double GeoFuncs::GetDisToStartOfPolyline(const AcDbObjectId & id, const AcGePoint3d & pnt)
{
	AcDbPolyline* pl = DBObject::OpenObjectById<AcDbPolyline>(id);

	if (NULL == pl)
		return 0.0;

	double ret = 0.0;
	for (int i = 0; i < pl->numVerts() - 1; i++)
	{
		AcGePoint2d tmp_pnt1 = AcGePoint2d::kOrigin;
		AcGePoint2d tmp_pnt2 = AcGePoint2d::kOrigin;

		pl->getPointAt(i, tmp_pnt1);
		pl->getPointAt(i + 1, tmp_pnt2);

		AcGePoint3d pnt1 = AcGePoint3d(tmp_pnt1.x, tmp_pnt1.y, 0.0);
		AcGePoint3d pnt2 = AcGePoint3d(tmp_pnt2.x, tmp_pnt2.y, 0.0);

		if (0.1 > std::abs(pnt1.distanceTo(pnt) + pnt2.distanceTo(pnt) - pnt1.distanceTo(pnt2)))
		{
			AcGePoint2d tmp_sp = AcGePoint2d::kOrigin;
			pl->getPointAt(0, tmp_sp);
			return GetLengthPolyline(id, AcGePoint3d(tmp_sp.x, tmp_sp.y, 0.0), pnt1) + pnt1.distanceTo(pnt);
		}
	}

	return 0.0;
}

double GeoFuncs::GetDisTwoPointOfPolyline(const AcDbObjectId & id, const AcGePoint3d & start, const AcGePoint3d & end)
{
	return std::abs(GetDisToStartOfPolyline(id, start) - GetDisToStartOfPolyline(id, end));
}

AcGePoint3dArray GeoFuncs::GetListVertexOfPolyline(AcDbPolyline * pline)
{
	AcGePoint3dArray pnts;

	for (int i = 0; i < pline->numVerts(); i++)
	{
		AcGePoint3d pnt(AcGePoint3d::kOrigin);
		pline->getPointAt(i, pnt);
		pnts.append(pnt);
	}

	return pnts;
}

AcGePoint3dArray GeoFuncs::GetListVertexOfPolyline(const AcDbObjectId & id)
{
	AcGePoint3dArray pnts;
	if (AcDbObjectId::kNull != id)
	{
		ObjectWrap<AcDbObject> pl_w(DBObject::OpenObjectById<AcDbObject>(id));
		if (NULL != pl_w.object && pl_w.object->isKindOf(AcDbPolyline::desc()))
		{
			AcDbPolyline* pl = AcDbPolyline::cast(pl_w.object);
			for (int i = 0; i < pl->numVerts(); i++)
			{
				AcGePoint3d pnt(AcGePoint3d::kOrigin);
				pl->getPointAt(i, pnt);
				pnts.append(pnt);
			}
		}
	}
	return pnts;
}

AcDbObjectId GeoFuncs::MergeTwoEntityPE(AcDbDatabase* db, AcDbObjectId id_1, AcDbObjectId id_2)
{
	ObjectWrap<AcDbEntity> obj_1(DBObject::OpenObjectById<AcDbEntity>(id_1));
	ObjectWrap<AcDbEntity> obj_2(DBObject::OpenObjectById<AcDbEntity>(id_2));

	if (NULL == obj_1.object || NULL == obj_2.object)
		return AcDbObjectId::kNull;

	
	AcDbJoinEntityPE* join = AcDbJoinEntityPE::cast(obj_1.object->queryX(AcDbJoinEntityPE::desc()));
	if (join == NULL)
		return AcDbObjectId::kNull;

	obj_1.object->upgradeOpen();
	obj_2.object->upgradeOpen();

	Acad::ErrorStatus ret = join->joinEntity(obj_1.object, obj_2.object);
	if (Acad::eOk == ret)
	{
		obj_2.object->erase();
		obj_1.object->downgradeOpen();
		return obj_1.object->id();
	}

	return AcDbObjectId::kNull;
}

bool GeoFuncs::IsPointOnStraightline(const AcGePoint3d & sp, const AcGePoint3d & ep, const AcGePoint3d & pnt)
{
	AcGeTol tol = AcGeTol();
	tol.setEqualPoint(1.0);
	if (pnt.isEqualTo(sp, tol) || pnt.isEqualTo(ep, tol))
		return true;
	tol.setEqualVector(1e-3);
	AcGeVector3d vec_b(ep.x - sp.x, ep.y - sp.y, ep.z - sp.z);
	AcGeVector3d vec_check(pnt.x - sp.x, pnt.y - sp.y, pnt.z - sp.z);
	AcGeVector3d v1 = vec_check.normal();
	AcGeVector3d v2 = vec_b.normal();
	if (vec_check.normal().isEqualTo(vec_b.normal(), tol))
	{
		if (sp.distanceTo(pnt) < sp.distanceTo(ep))
			return true;
		return false;
	}
	return false;
}
