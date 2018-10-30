#include "DBObject.h"
#include "../wrap_header.h"

AcDbBlockTable * DBObject::GetBlockTable(AcDbDatabase * db)
{
	AcDbBlockTable* blk_tbl = NULL;
	if (Acad::eOk != db->getBlockTable(blk_tbl, AcDb::OpenMode::kForRead))
		throw std::exception("Failed when get Block Table");
	return blk_tbl;
}

AcDbBlockTableRecord * DBObject::GetModelSpace(AcDbDatabase * db)
{
	return GetBlkTblRcd(db, ACDB_MODEL_SPACE);
}

AcDbBlockTableRecord * DBObject::GetBlkTblRcd(AcDbDatabase * db, const wchar_t * blk_name)
{
	AcDbBlockTableRecord* blk_tbl_rcd = NULL;
	{
		ObjectWrap<AcDbBlockTable> blk_tbl_wrap(GetBlockTable(db));
		if (Acad::eOk != blk_tbl_wrap.object->getAt(blk_name, blk_tbl_rcd, AcDb::kForRead))
			throw std::exception("Failed when get Block Table Record");
	}
	return blk_tbl_rcd;
}

AcDbObjectId DBObject::FindBlockByName(AcDbDatabase * db, const wchar_t * bn)
{
		ObjectWrap<AcDbBlockTable> blk_tbl_wrap(DBObject::GetBlockTable(db));
		AcDbObjectId blk_id = AcDbObjectId::kNull;
		blk_tbl_wrap.object->getAt(bn, blk_id);

		return blk_id;
}

AcDbObjectIdArray DBObject::FindBlockRefsByName(AcDbDatabase * db, const wchar_t * name)
{
	AcDbObjectIdArray br_ids;

	AcDbObjectId obj_id = FindBlockByName(db, name);
	ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(obj_id));

	if (NULL != obj_wrap.object && obj_wrap.object->isKindOf(AcDbBlockTableRecord::desc()))
	{
		AcDbBlockTableRecord::cast(obj_wrap.object)->getBlockReferenceIds(br_ids);
	}

	return br_ids;
}

AcDbDatabase * DBObject::GetDataBase()
{
	return acdbHostApplicationServices()->workingDatabase();
}


