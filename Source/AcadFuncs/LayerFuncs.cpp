#include "LayerFuncs.h"
#include "../wrap_header.h"
#include "../AcadFuncs/DBObject.h"
#include "../AcadFuncs/AcadFuncs.h"

AcDbLayerTable * LayerFuncs::GetLayerTable(AcDbDatabase * db)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbLayerTable* layer_tbl = NULL;
	ret = db->getLayerTable(layer_tbl, AcDb::kForRead);
	return layer_tbl;
}

AcDbLayerTableRecord* LayerFuncs::GetLayerTableRecord(AcDbDatabase * db, const wchar_t * layer_name)
{
	ObjectWrap<AcDbLayerTable> layer_tbl_wrap(GetLayerTable(db));
	AcDbLayerTableRecord* layer_tbl_rcd = NULL;
	layer_tbl_wrap.object->getAt(layer_name, layer_tbl_rcd, AcDb::kForRead);
	return layer_tbl_rcd;
}


bool LayerFuncs::CheckExistedLayer(AcDbDatabase * db, const wchar_t * layer)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbLayerTableIterator* iter = NULL;
	{
		ObjectWrap<AcDbLayerTable> layer_tbl(GetLayerTable(db));
		layer_tbl.object->newIterator(iter);
	}

	while (!iter->done())
	{
		AcDbLayerTableRecord* rcd = NULL;
		if (iter->getRecord(rcd, AcDb::kForRead) == Acad::eOk)
		{
			ObjectWrap<AcDbLayerTableRecord> layer_wrap(rcd);
			wchar_t* layer_name;
			layer_wrap.object->getName(layer_name);
			std::size_t length = std::wcslen(layer);
			if (Functions::IsMatchRegex((wchar_t*)layer, layer_name))
			{
				delete iter;
				return true;
			}
		}
		iter->step();
	}

	delete iter;
	return false;
}

void LayerFuncs::ChangeLayer(AcDbDatabase * db, wchar_t * base, wchar_t * des, bool del_layer)
{
	Acad::ErrorStatus ret = Acad::eOk;

	wchar_t* layer_trim = new wchar_t[wcslen(base)];
	wcsncpy(layer_trim, base, wcslen(base) - 1);
	layer_trim[wcslen(base) - 1] = '\0';

	AcDbLayerTableIterator* iter = NULL;
	{
		ObjectWrap<AcDbLayerTable> layer_tbl(GetLayerTable(db));
		layer_tbl.object->newIterator(iter);
	}
	while (!iter->done())
	{
		AcDbLayerTableRecord* rcd = NULL;
		if (iter->getRecord(rcd, AcDb::kForRead) == Acad::eOk)
		{
			ObjectWrap<AcDbLayerTableRecord> layer_wrap(rcd);
			wchar_t* layer_name = L"";
			layer_wrap.object->getName(layer_name);
			std::size_t length = std::wcslen(base);
			wchar_t cmp_c = base[length - 1];
			if (base[length - 1] == L'*')
			{
				if (!wcsncmp(layer_trim, layer_name, wcslen(layer_trim)))
				{
					MoveAllEntityOnLayer(db, layer_name, des);
					if (del_layer)
					{
						ret = layer_wrap.object->upgradeOpen();
						ret = layer_wrap.object->erase();
					}
				}
			}
			else if (!wcscmp(layer_name, base))
			{
				MoveAllEntityOnLayer(db, layer_name, des);
				if (del_layer)
				{
					ret = layer_wrap.object->upgradeOpen();
					ret = layer_wrap.object->erase();
				}
			}
		}
		iter->step();
	}

	delete[] layer_trim;
	delete iter;
}

void LayerFuncs::ChangeLayer(AcDbDatabase * db, AcDbObjectIdArray ids, const wchar_t * layer)
{
	for (int i = 0; i < ids.length(); i++)
	{
		ObjectWrap<AcDbEntity> ent_wrap(DBObject::OpenObjectById<AcDbEntity>(ids.at(i)));
		if (ent_wrap.object != NULL)
		{
			ent_wrap.object->upgradeOpen();
			ent_wrap.object->setLayer(layer);
		}
	}
}


void LayerFuncs::DeleteLayer(AcDbDatabase * db, wchar_t * layer)
{
	Acad::ErrorStatus ret = Acad::eOk;

	wchar_t* layer_trim = new wchar_t[wcslen(layer)];
	wcsncpy(layer_trim, layer, wcslen(layer) - 1);
	layer_trim[wcslen(layer) - 1] = '\0';

	AcDbLayerTableIterator* iter = NULL;
	{
		ObjectWrap<AcDbLayerTable> layer_tbl(GetLayerTable(db));
		layer_tbl.object->newIterator(iter);
	}
	while (!iter->done())
	{
		AcDbLayerTableRecord* rcd = NULL;
		if (iter->getRecord(rcd, AcDb::kForRead) == Acad::eOk)
		{
			ObjectWrap<AcDbLayerTableRecord> layer_wrap(rcd);
			wchar_t* layer_name;
			layer_wrap.object->getName(layer_name);
			std::size_t length = std::wcslen(layer);
			wchar_t cmp_c = layer[length - 1];
			if (layer[length - 1] == L'*')
			{
				if (!wcsncmp(layer_trim, layer_name, wcslen(layer_trim)))
				{
					EraseAllEntityOnLayer(db, layer_name);
					ret = layer_wrap.object->upgradeOpen();
					ret = layer_wrap.object->erase();
				}
			}
			else if (!wcscmp(layer_name, layer))
			{
				EraseAllEntityOnLayer(db, layer_name);
				layer_wrap.object->upgradeOpen();
				layer_wrap.object->erase();
			}
		}
		iter->step();
	}

	delete[] layer_trim;
	delete iter;
}

void LayerFuncs::DeleteLayerByRegex(AcDbDatabase * db, wchar_t * regex)
{
	AcDbLayerTableIterator* iter = NULL;
	{
		ObjectWrap<AcDbLayerTable> layer_tbl(GetLayerTable(db));
		layer_tbl.object->newIterator(iter);
	}
	while (!iter->done())
	{
		AcDbLayerTableRecord* rcd = NULL;
		if (Acad::eOk == iter->getRecord(rcd, AcDb::kForRead))
		{
			ObjectWrap<AcDbLayerTableRecord> layer_wrap(rcd);
			wchar_t* layer_name = L"";
			layer_wrap.object->getName(layer_name);

			if (Functions::IsMatchRegex(regex, layer_name))
			{
				EraseAllEntityOnLayer(db, layer_name);
				layer_wrap.object->upgradeOpen();
				layer_wrap.object->erase();
			}
		}
		iter->step();
	}

	delete iter;
}

void LayerFuncs::DeleteLayerFromFile(const wchar_t * file_name, const wchar_t * layer)
{
	AcDbDatabase* db = new AcDbDatabase(Adesk::kFalse);
	if (db->readDwgFile(file_name) != Acad::eOk)
		throw std::exception("Read dwg failed");
	else
		acdbHostApplicationServices()->setWorkingDatabase(db);
	DeleteLayerByRegex(db, (wchar_t*)layer);
	delete db;
}

void LayerFuncs::EraseAllEntityOnLayer(AcDbDatabase * db, wchar_t * layer)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbBlockTableRecordIterator *iter = NULL;
	{
		ObjectWrap<AcDbBlockTableRecord> model_space(DBObject::GetModelSpace(db));
		model_space.object->newIterator(iter);
	}

	while (!iter->done())
	{
		ObjectWrap<AcDbEntity> ent_wrap(Functions::GetEntityInterTableRecord(iter));
		if (ent_wrap.object != NULL && wcscmp(ent_wrap.object->layer(), layer) == 0)
		{
			ret = ent_wrap.object->upgradeOpen();
			ret = ent_wrap.object->erase();
		}
		iter->step();
	}
	delete iter;
}

void LayerFuncs::MoveAllEntityOnLayer(AcDbDatabase * db, wchar_t * layer, wchar_t * des)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbBlockTableRecordIterator *iter = NULL;
	{
		ObjectWrap<AcDbBlockTableRecord> model_space(DBObject::GetModelSpace(db));
		model_space.object->newIterator(iter);
	}

	while (!iter->done())
	{
		ObjectWrap<AcDbEntity> ent_wrap(Functions::GetEntityInterTableRecord(iter));
		if (ent_wrap.object != NULL && wcscmp(ent_wrap.object->layer(), layer) == 0)
		{
			ret = ent_wrap.object->upgradeOpen();
			ret = ent_wrap.object->setLayer(des);
		}
		iter->step();
	}
	delete iter;
}

AcDbObjectId LayerFuncs::GetLayerId(AcDbDatabase * db, const wchar_t * layer)
{
	ObjectWrap<AcDbLayerTableRecord> lyr_rcd_wrap = GetLayerTableRecord(db, layer);
	if (NULL == lyr_rcd_wrap.object)
	{
		ObjectWrap<AcDbLayerTable> lyr(GetLayerTable(db));
		ObjectWrap<AcDbLayerTableRecord>  lyr_rcd(new AcDbLayerTableRecord());
		lyr_rcd.object->setName(layer);
		lyr.object->upgradeOpen();
		lyr.object->add(lyr_rcd.object);
		return lyr_rcd.object->id();
	}
	ObjectWrap<AcDbLayerTableRecord> lyr_rcd(GetLayerTableRecord(db, layer));
	return lyr_rcd.object->id();
}


AcDbObjectIdArray LayerFuncs::GetEntityBelongToLayer(AcDbDatabase * db, const wchar_t * layer_name)
{
	AcDbObjectIdArray ids;
	{
		AcDbBlockTableRecordIterator *iter = NULL;
		{
			ObjectWrap<AcDbBlockTableRecord> model_space = DBObject::GetModelSpace(db);
			model_space.object->newIterator(iter);
		}

		while (!iter->done())
		{
			ObjectWrap<AcDbEntity> ent_wrap(Functions::GetEntityInterTableRecord(iter));
			if (ent_wrap.object != NULL && wcscmp(ent_wrap.object->layer(), layer_name) == 0)
			{
				ids.append(ent_wrap.object->id());
			}
			iter->step();
		}
		delete iter;
	}
	return ids;
}

AcDbObjectIdArray LayerFuncs::GetPointsBelongToLayer(AcDbDatabase* db, const wchar_t* entry_name, const wchar_t * layer_name)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbObjectIdArray ids;
	{
		AcDbBlockTableRecordIterator *iter = NULL;

		{
			ObjectWrap<AcDbBlockTableRecord> btr_wrap(DBObject::GetBlkTblRcd(db, entry_name));
			btr_wrap.object->newIterator(iter);
		}

		while (!iter->done())
		{
			ObjectWrap<AcDbEntity> ent_wrap(Functions::GetEntityInterTableRecord(iter));
			if (ent_wrap.object != NULL && ent_wrap.object->isKindOf(AcDbPoint::desc()))
			{
				AcDbPoint* point = AcDbPoint::cast(ent_wrap.object);
				if (0 == wcscmp((wchar_t*)point->layer(), layer_name))
					ids.append(point->objectId());
			}
			iter->step();
		}
		delete iter;
	}
	return ids;
}