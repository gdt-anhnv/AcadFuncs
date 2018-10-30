#include "XData.h"
#include "../wrap_header.h"
#include "../AcadFuncs/AcadFuncs.h"
#include "DBObject.h"

void XDataFuncs::AddXDataIntVal(const AcDbObjectId & id, const std::wstring & app_name, const int & value)
{
	if (AcDbObjectId::kNull == id)
		return;
	ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(id));

	if (NULL == obj_wrap.object)
		return;

	if (!Functions::HasRegAppName(acdbHostApplicationServices()->workingDatabase(), app_name.c_str()))
		Functions::AddRegAppName(acdbHostApplicationServices()->workingDatabase(), app_name.c_str());

	ResBufWrap res_wrap(obj_wrap.object->xData(app_name.c_str()));

	if (NULL == res_wrap.res_buf)
	{
		res_wrap.res_buf = acutBuildList(
			AcDb::kDxfRegAppName, app_name.c_str(),
			RTNONE);

		struct resbuf* new_rb = acutNewRb(AcDb::kDxfXdInteger16);
		new_rb->resval.rint = value;
		res_wrap.res_buf->rbnext = new_rb;
	}
	else
	{
		resbuf* tmp_rb = res_wrap.res_buf;

		if (NULL != tmp_rb)
		{
			while (NULL != tmp_rb->rbnext)
			{
				tmp_rb = tmp_rb->rbnext;
			}
		}

		struct resbuf* new_rb = acutNewRb(AcDb::kDxfXdInteger16);
		new_rb->resval.rint = value;
		tmp_rb->rbnext = new_rb;
	}
	obj_wrap.object->upgradeOpen();
	obj_wrap.object->setXData(res_wrap.res_buf);
	obj_wrap.object->downgradeOpen();
}

void XDataFuncs::AddXDataWstringVal(const AcDbObjectId & id, const std::wstring & app_name, const ACHAR * value)
{
	if (AcDbObjectId::kNull == id)
		return;
	ObjectWrap<AcDbObject>obj_wrap(DBObject::OpenObjectById<AcDbObject>(id));

	if (NULL == obj_wrap.object)
		return;

	if (!Functions::HasRegAppName(acdbHostApplicationServices()->workingDatabase(), app_name.c_str()))
		Functions::AddRegAppName(acdbHostApplicationServices()->workingDatabase(), app_name.c_str());

	ResBufWrap res_wrap(obj_wrap.object->xData(app_name.c_str()));

	if (NULL == res_wrap.res_buf)
	{

		res_wrap.res_buf = acutBuildList(
			AcDb::kDxfRegAppName, app_name.c_str(),
			RTNONE);

		struct resbuf* new_rb = acutBuildList(
			AcDb::kDxfXdAsciiString, value,
			RTNONE);

		res_wrap.res_buf->rbnext = new_rb;
	}
	else
	{
		resbuf* tmp_rb = res_wrap.res_buf;

		if (NULL != tmp_rb)
		{
			while (NULL != tmp_rb->rbnext)
			{
				tmp_rb = tmp_rb->rbnext;
			}
		}

		struct resbuf* new_rb = acutBuildList(
			AcDb::kDxfXdAsciiString, value,
			RTNONE);

		tmp_rb->rbnext = new_rb;
	}
	obj_wrap.object->upgradeOpen();
	obj_wrap.object->setXData(res_wrap.res_buf);
	obj_wrap.object->downgradeOpen();
}

bool XDataFuncs::CheckXDataIntVal(const AcDbObjectId & id, const std::wstring & app_name, const int & value)
{
	ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(id));
	if (NULL == obj_wrap.object)
		return false;

	ResBufWrap res_wrap(obj_wrap.object->xData(app_name.c_str()));

	struct resbuf* tmp_res;
	if (NULL != res_wrap.res_buf)
		tmp_res = res_wrap.res_buf->rbnext;

	while (NULL != tmp_res)
	{
		if (value == tmp_res->resval.rint)
			return true;
		tmp_res = tmp_res->rbnext;
	}
	return false;
}

bool XDataFuncs::CheckXDataStringVal(const AcDbObjectId & id, const std::wstring & app_name, const std::wstring & value)
{
	ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(id));
	if (NULL == obj_wrap.object)
		return false;

	ResBufWrap res_wrap(obj_wrap.object->xData(app_name.c_str()));

	struct resbuf* tmp_res;
	if (NULL != res_wrap.res_buf)
		tmp_res = res_wrap.res_buf->rbnext;

	while (NULL != tmp_res)
	{
		if (value == tmp_res->resval.rstring)
			return true;
		tmp_res = tmp_res->rbnext;
	}
	return false;
}

void XDataFuncs::SetXData(AcDbDatabase* db, const AcDbObjectId & id, const resbuf * xdata)
{
	AcDbRegAppTable* reg_app_tbl = NULL;
	if (Acad::eOk == db->getRegAppTable(reg_app_tbl, AcDb::kForWrite))
	{
		if (Adesk::kFalse == reg_app_tbl->has(xdata->resval.rstring))
		{
			AcDbRegAppTableRecord *reg_tbl_rcd = new AcDbRegAppTableRecord;
			reg_tbl_rcd->setName(xdata->resval.rstring);
			Acad::ErrorStatus err = reg_app_tbl->add(reg_tbl_rcd);
			if (Acad::eDuplicateRecordName == err)
			{
				reg_tbl_rcd->erase(true);
			}
			reg_tbl_rcd->close();
		}
		reg_app_tbl->close();
	}

	ObjectWrap<AcDbEntity> ent(DBObject::OpenObjectById<AcDbEntity>(id));
	ent.object->upgradeOpen();

	Acad::ErrorStatus err = ent.object->setXData(xdata);
	ent.object->close();
}
