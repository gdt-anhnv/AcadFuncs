#ifndef _DB_OBJECT_H_
#define _DB_OBJECT_H_

#include "../acad_header.h"

class DBObject
{
public:
	template <class T>
	static T* OpenObjectById(const AcDbObjectId & id);

	template <class T>
	static bool CheckObjectById(const AcDbObjectId & id);

	static AcDbBlockTable* GetBlockTable(AcDbDatabase* db);
	static AcDbBlockTableRecord* GetModelSpace(AcDbDatabase* db);
	static AcDbBlockTableRecord* GetBlkTblRcd(AcDbDatabase* db, const wchar_t* blk_name);

	static AcDbObjectId FindBlockByName(AcDbDatabase * db, const wchar_t * bn);
	static AcDbObjectIdArray FindBlockRefsByName(AcDbDatabase* db, const wchar_t* name);

	static AcDbDatabase *GetDataBase();

};

template<class T>
T * DBObject::OpenObjectById(const AcDbObjectId & id)
{
	if (id.isNull())
		return NULL;
	AcDbObject* obj = NULL;
	if (Acad::eOk != acdbOpenObject(obj, id, AcDb::kForRead))
		return NULL;

	try
	{
		T* ret = T::cast(obj);
		if (NULL == ret && NULL != obj)
			obj->close();

		return ret;
	}
	catch (...)
	{

	}
	return NULL;
}

template<class T>
bool DBObject::CheckObjectById(const AcDbObjectId & id)
{
	ObjectWrap<T> obj_wrap(OpenObjectById<T>(id));
	if (NULL == obj_wrap.object)
		return false;
	return true;
}

#endif
