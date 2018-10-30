#include "acad_header.h"
#include "AcadFuncs.h"
#include "acad_wrap.h"

#include "constants.h"
#include "dbapserv.h"
#include "adscodes.h"
#include "geassign.h"
#include "acedCmdNF.h"
#include <iostream>
#include <exception>
#include <regex>

static bool IsMatchRegex(wchar_t* regex, wchar_t* source)
{
#ifdef CPP11
	std::wstring regex_str(regex);
	std::size_t pos = 0;
	do
	{
		pos = regex_str.find(L"*", pos);
		if (std::wstring::npos == pos)
			break;
		regex_str.erase(pos, 1);
		regex_str.insert(pos, L"(.*?)");
		pos += 3;
	} while (true);

	std::wregex rg(regex_str.c_str());
	std::wcmatch m;
	return std::regex_match(source, m, rg);
#else
	return false;
#endif
}

static AcDbEntity* GetEntity(AcDbBlockTableRecordIterator* iter)
{
	AcDbEntity* ent = NULL;
	iter->getEntity(ent, AcDb::kForRead);
	return ent;
}

AcDbObjectIdArray AcadFuncs::GetEntityBelongToLayer(AcDbDatabase * db, const wchar_t * layer_name)
{
	AcDbObjectIdArray ids;
	{
		AcDbBlockTableRecordIterator *iter = NULL;
		{
			BlkTblRcd_Wrap model_space = AcadFuncs::GetModelSpace(db);
			model_space.blk_tbl_rcd->newIterator(iter);
		}

		while (!iter->done())
		{
			EntityWrap ent_wrap(GetEntity(iter));
			if (ent_wrap.entity != NULL && wcscmp(ent_wrap.entity->layer(), layer_name) == 0)
			{
				ids.append(ent_wrap.entity->id());
			}
			iter->step();
		}
		delete iter;
	}
	return ids;
}

AcDbObjectIdArray AcadFuncs::GetPointsBelongToLayer(AcDbDatabase* db, const wchar_t* entry_name, const wchar_t * layer_name)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbObjectIdArray ids;
	{
		AcDbBlockTableRecordIterator *iter = NULL;

		{
			BlkTblRcd_Wrap btr_wrap(GetBlkTblRcd(db, entry_name));
			btr_wrap.blk_tbl_rcd->newIterator(iter);
		}

		while (!iter->done())
		{
			EntityWrap ent_wrap(GetEntity(iter));
			if (ent_wrap.entity != NULL && ent_wrap.entity->isKindOf(AcDbPoint::desc()))
			{
				AcDbPoint* point = AcDbPoint::cast(ent_wrap.entity);
				if (0 == wcscmp((wchar_t*)point->layer(), layer_name))
					ids.append(point->objectId());
			}
			iter->step();
		}
		delete iter;
	}
	return ids;
}

AcDbBlockTable * AcadFuncs::GetBlockTable(AcDbDatabase * db)
{
	AcDbBlockTable* blk_tbl = NULL;
	if (Acad::eOk != db->getBlockTable(blk_tbl, AcDb::OpenMode::kForRead))
		throw std::exception("Failed when get Block Table");
	return blk_tbl;
}

AcDbBlockTableRecord * AcadFuncs::GetModelSpace(AcDbDatabase * db)
{
	return GetBlkTblRcd(db, ACDB_MODEL_SPACE);
}

AcDbBlockTableRecord * AcadFuncs::GetBlkTblRcd(AcDbDatabase * db, const wchar_t * blk_name)
{
	AcDbBlockTableRecord* blk_tbl_rcd = NULL;
	{
		BlkTbl_Wrap blk_tbl_wrap(GetBlockTable(db));
		Acad::ErrorStatus e = blk_tbl_wrap.blk_tbl->getAt(blk_name, blk_tbl_rcd, AcDb::kForRead);
		if (Acad::eOk != e)
			throw std::exception("Failed when get Block Table Record");
	}
	return blk_tbl_rcd;
}

AcDbEntity* AcadFuncs::GetEntityById(AcDbObjectId obj_id)
{
	AcDbEntity* ent = NULL;
	Acad::ErrorStatus err = acdbOpenAcDbEntity(ent, obj_id, AcDb::kForRead);

	return ent;
}

AcDbViewportTableRecord * AcadFuncs::GetActiveViewport(AcDbDatabase * db)
{
	AcDbViewportTable* vpt_tbl = NULL;
	db->getSymbolTable(vpt_tbl, AcDb::kForRead);
	AcDbViewportTableRecord* vpt_tbl_rcd = new AcDbViewportTableRecord();
	vpt_tbl->getAt(ACTIVE_VIEWPORT, vpt_tbl_rcd, AcDb::kForRead);
	vpt_tbl->close();
	return vpt_tbl_rcd;
}

AcDbObject * AcadFuncs::OpenObjectById(AcDbObjectId id)
{
	AcDbObject* obj = NULL;
	if (Acad::eOk == acdbOpenObject(obj, id, AcDb::kForRead))
		return obj;

	return NULL;
}

AcDbObjectId AcadFuncs::GetTextStyleId(AcDbDatabase * db, std::wstring & ts)
{
	AcDbTextStyleTable *txt_sty_tbl = NULL;
	db->getSymbolTable(txt_sty_tbl, AcDb::kForRead);

	AcDbObjectId _id = AcDbObjectId::kNull;
	txt_sty_tbl->getAt(ts.c_str(), _id);
	txt_sty_tbl->close();

	return _id;
}

AcDbLayerTable * AcadFuncs::GetLayerTable(AcDbDatabase * db)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbLayerTable* layer_tbl = NULL;
	ret = db->getLayerTable(layer_tbl, AcDb::kForRead);
	return layer_tbl;
}

AcDbLayerTableRecord* AcadFuncs::GetLayerTableRecord(AcDbDatabase * db, const wchar_t * layer_name)
{
	LayerTblWrap layer_tbl_wrap(GetLayerTable(db));
	AcDbLayerTableRecord* layer_tbl_rcd = NULL;
	layer_tbl_wrap.layer_tbl->getAt(layer_name, layer_tbl_rcd, AcDb::kForRead);
	return layer_tbl_rcd;
}

AcDbDimStyleTable * AcadFuncs::GetDimStyleTable(AcDbDatabase * db)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbDimStyleTable* dim_tbl = NULL;
	ret = db->getDimStyleTable(dim_tbl, AcDb::kForRead);
	return dim_tbl;
}

AcDbDimStyleTableRecord * AcadFuncs::GetDimStyleTableRecord(AcDbDatabase * db, const wchar_t * entryName)
{
	DimStyleTblWrap dim_wrap(GetDimStyleTable(db));
	AcDbDimStyleTableRecord* rcd = NULL;
	dim_wrap.dim_style_tbl->getAt(entryName, rcd, AcDb::kForRead);
	return rcd;
}

AcDbExtents* AcadFuncs::GetBoundary(AcDbDatabase* db)
{
	AcDbExtents* ext = new AcDbExtents;

	BlkTblRcd_Wrap btr_wrap(AcadFuncs::GetModelSpace(db));
	AcDbBlockTableRecordIterator *blk_tbl_rcd_iter = NULL;
	if (Acad::eOk == btr_wrap.blk_tbl_rcd->newIterator(blk_tbl_rcd_iter))
	{
		AcDbEntity *ent = NULL;
		Acad::ErrorStatus er = Acad::eOk;// = ext.addBlockExt(btr_wrap.blk_tbl_rcd);

		while (!blk_tbl_rcd_iter->done())
		{
			AcDbExtents temp = AcDbExtents();
			if (blk_tbl_rcd_iter->getEntity(ent, AcDb::kForRead) == Acad::eOk)
			{
				if (ent->isKindOf(AcDbBlockReference::desc()))
					AcDbBlockReference::cast(ent)->geomExtentsBestFit(temp);
				else
					er = ent->getGeomExtents(temp);
				ent->close();

				ext->addExt(temp);
			}

			blk_tbl_rcd_iter->step();
		}
		delete blk_tbl_rcd_iter;
	}

	return ext;
}

void AcadFuncs::GetBoundaryDrawing(const wchar_t* fp, double & width, double & height)
{
	AcDbDatabase* db = new AcDbDatabase(false);
	if (Acad::eOk != db->readDwgFile(fp))
		throw std::exception("Failed to get boundary of drawing");

	Acad::ErrorStatus err = db->updateExt(true);

	AcGePoint3d ext_min = db->extmin();
	AcGePoint3d ext_max = db->extmax();

	width = ext_max.x - ext_min.x;
	height = ext_max.y - ext_min.y;
	/*
	AcDbExtents* extents = GetBoundary(db);
	width = extents->maxPoint().x - extents->minPoint().x;
	chairs_h = extents->maxPoint().y - extents->minPoint().y;
	delete extents;
	*/

	delete db;
}

const std::wstring AcadFuncs::GetAttValueWithTag(AcDbDatabase * db, const wchar_t* entry_name, const wchar_t * tag)
{
	AcDbBlockTableRecordIterator *iter = NULL;
	{
		BlkTblRcd_Wrap btr_wrap(GetBlkTblRcd(db, entry_name));
		btr_wrap.blk_tbl_rcd->newIterator(iter);
	}

	while (!iter->done())
	{
		EntityWrap ent_wrap(GetEntity(iter));
		AcDbAttributeDefinition* att_def = NULL;
		if (ent_wrap.entity != NULL && ent_wrap.entity->isKindOf(AcDbAttributeDefinition::desc()))
		{
			att_def = AcDbAttributeDefinition::cast(ent_wrap.entity);
			ent_wrap.entity = NULL;
			if (0 == wcscmp(att_def->tag(), tag))
			{
				wchar_t* txt = att_def->textString();
				std::wstring wstr_txt = std::wstring(txt);
				delete[] txt;
#ifdef CPP11
				return std::move(wstr_txt);
#else
				return wstr_txt;
#endif
			}
		}
		iter->step();
	}

	return L"";
}

const std::wstring AcadFuncs::GetAttValueWithTag(AcDbObjectId blk_id, const wchar_t * tag)
{
	if (AcDbObjectId::kNull != blk_id)
	{
		EntityWrap ent_wrap(AcadFuncs::GetEntityById(blk_id));
		if (NULL == ent_wrap.entity || !ent_wrap.entity->isKindOf(AcDbBlockReference::desc()))
			return L"";

		AcDbBlockReference* blk_tendrop = AcDbBlockReference::cast(ent_wrap.entity);
		AcDbObjectIterator* prop_iter = blk_tendrop->attributeIterator();

		for (; !prop_iter->done(); prop_iter->step())
		{
			AcDbObjectId att_id = prop_iter->objectId();
			AcDbAttribute* att = NULL;
			if (Acad::eOk != blk_tendrop->openAttribute(att, att_id, AcDb::kForRead))
				continue;

			AttributeWrap att_wrap(att);
			if (0 == wcscmp(tag, att_wrap.attribute->tag()))
			{
				wchar_t* txt = att_wrap.attribute->textString();
				std::wstring wstr_txt = std::wstring(txt);
				delete[] txt;
#ifdef CPP11
				return std::move(wstr_txt);
#else
				return wstr_txt;
#endif
			}
		}
	}
	return L"";
}


AcDbObjectId AcadFuncs::BindingXref(AcDbDatabase * db, const wchar_t * source_data, const wchar_t* name, AcGePoint3d ins_pnt)
{
	AcDbObjectId obj_id = AcDbObjectId::kNull;
	Acad::ErrorStatus err = acdbAttachXref(db, source_data, name, obj_id);
	if (Acad::eOk != err)
		return AcDbObjectId::kNull;

	AcDbBlockReference* br = new AcDbBlockReference(ins_pnt, obj_id);
	{
		BlkTblRcd_Wrap blk_tbl_rcd_wrap(AcadFuncs::GetModelSpace(db));
		AcDbObjectId newEntId = AcDbObjectId::kNull;
		blk_tbl_rcd_wrap.blk_tbl_rcd->upgradeOpen();
		blk_tbl_rcd_wrap.blk_tbl_rcd->appendAcDbEntity(newEntId, br);
	}

	br->close();
	AcDbObjectIdArray binding_ids;
	binding_ids.append(obj_id);
	acdbBindXrefs(db, binding_ids, true);

	return br->id();
}

void AcadFuncs::ExplodeBlockReference(AcDbDatabase * db, AcDbObjectId id, bool del_obj)
{
	ObjectWrap obj_wrap(AcadFuncs::OpenObjectById(id));
	if (NULL != obj_wrap.obj)
	{
		if (obj_wrap.obj->isKindOf(AcDbBlockReference::desc()))
		{
			AcDbBlockReference* br = AcDbBlockReference::cast(obj_wrap.obj);
			AcDbVoidPtrArray ents(1);
			Acad::ErrorStatus err = br->explode(ents);

			BlkTblRcd_Wrap ms_wrap(AcadFuncs::GetModelSpace(db));
			ms_wrap.blk_tbl_rcd->upgradeOpen();
			AcDbObjectId ins_id = AcDbObjectId::kNull;
			for (int i = 0; i < ents.length(); i++)
			{
				AcDbEntity* sub_ent = AcDbEntity::cast((AcRxObject*)ents[i]);
				if (NULL != sub_ent)
					err = ms_wrap.blk_tbl_rcd->appendAcDbEntity(
						ins_id, EntityWrap(sub_ent).entity);
			}
		}

		if (del_obj)
		{
			obj_wrap.obj->upgradeOpen();
			obj_wrap.obj->erase();
		}
	}

}

AcDbObjectId AcadFuncs::GetAttributeByName(AcDbObjectId br_id, const wchar_t * att_name)
{
	ObjectWrap obj_wrap(OpenObjectById(br_id));
	if (NULL != obj_wrap.obj && obj_wrap.obj->isKindOf(AcDbBlockReference::desc()))
	{
		AcDbBlockReference* br = AcDbBlockReference::cast(obj_wrap.obj);
		AcDbObjectIterator* att_iter = br->attributeIterator();
		AcDbAttribute *att = NULL;

		while (!att_iter->done())
		{
			br->openAttribute(att, att_iter->objectId(), AcDb::kForRead);
			EntityWrap ent_wrap(att);
			if (NULL != att)
			{
				wchar_t* tag = att->tag();
				if (0 == wcscmp(att->tag(), att_name))
				{
					delete att_iter;
					delete[] tag;
					return att->id();
				}
				else
					delete[] tag;
			}
			att_iter->step();
		}
		delete att_iter;

		return AcDbObjectId::kNull;
	}

	return AcDbObjectId::kNull;
}

AcDbObjectId AcadFuncs::FindBlockByName(AcDbDatabase * db, const wchar_t * bn)
{
	BlkTbl_Wrap blk_tbl_wrap(AcadFuncs::GetBlockTable(db));
	AcDbObjectId blk_id = AcDbObjectId::kNull;
	blk_tbl_wrap.blk_tbl->getAt(bn, blk_id);

	return blk_id;
}

AcDbObjectIdArray AcadFuncs::FindBlockRefsByName(AcDbDatabase * db, const wchar_t * name)
{
	AcDbObjectIdArray br_ids;

	AcDbObjectId obj_id = FindBlockByName(db, name);
	ObjectWrap obj_wrap(OpenObjectById(obj_id));

	if (NULL != obj_wrap.obj && obj_wrap.obj->isKindOf(AcDbBlockTableRecord::desc()))
	{
		AcDbBlockTableRecord::cast(obj_wrap.obj)->getBlockReferenceIds(br_ids);
	}

	return br_ids;
}

void AcadFuncs::CloneAttributes(AcDbObjectId br_id, AcDbObjectId blk_id)
{
	ObjectWrap br_wrap(OpenObjectById(br_id));
	if (NULL == br_wrap.obj || !br_wrap.obj->isKindOf(AcDbBlockReference::desc()))
		return;

	AcDbBlockReference* br = AcDbBlockReference::cast(br_wrap.obj);

	ObjectWrap obj_wrap(AcadFuncs::OpenObjectById(blk_id));
	if (NULL == obj_wrap.obj || !obj_wrap.obj->isKindOf(AcDbBlockTableRecord::desc()))
		return;

	AcDbBlockTableRecord* btr = AcDbBlockTableRecord::cast(obj_wrap.obj);
	// Set up a block table record iterator to iterate over the attribute definitions.
	AcDbBlockTableRecordIterator *blk_tbl_rcd_iter = NULL;
	btr->newIterator(blk_tbl_rcd_iter);

	while (!blk_tbl_rcd_iter->done())
	{
		EntityWrap ent_wrap(GetEntity(blk_tbl_rcd_iter));
		if (ent_wrap.entity->isKindOf(AcDbAttributeDefinition::desc()))
		{
			AcDbAttributeDefinition* att_def = AcDbAttributeDefinition::cast(ent_wrap.entity);
			if (NULL != att_def)
			{
				AttributeWrap att_wrap(new AcDbAttribute());
				att_wrap.attribute->setAttributeFromBlock(att_def, br->blockTransform());
				AcDbObjectId att_id = AcDbObjectId::kNull;
				br->upgradeOpen();
				Acad::ErrorStatus err = br->appendAttribute(att_id, att_wrap.attribute);
				if (Acad::eOk != err)
				{
					delete att_wrap.attribute;
					att_wrap.attribute = NULL;
				}
				br->downgradeOpen();
			}
		}

		blk_tbl_rcd_iter->step();
	}

	delete blk_tbl_rcd_iter;
}

void AcadFuncs::SetAttributePosition(AcDbObjectId br_id, const wchar_t * tag, AcGePoint3d pos)
{
	ObjectWrap obj_wrap(OpenObjectById(br_id));

	if (obj_wrap.obj == NULL || !obj_wrap.obj->isKindOf(AcDbBlockReference::desc()))
	{
		return;
	}

	BlkRefWrap br_wrap(AcDbBlockReference::cast(obj_wrap.obj));
	
	AcDbObjectIterator* prop_iter = br_wrap.blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		Acad::ErrorStatus val = br_wrap.blk_ref->openAttribute(att, att_id, AcDb::kForRead);
		if (Acad::eOk != val)
			continue;

		AttributeWrap att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		att_wrap.attribute->upgradeOpen();
		att_wrap.attribute->setPosition(pos);
		att_wrap.attribute->setAlignmentPoint(pos);
	}

	delete prop_iter;
}

void AcadFuncs::SetAttributeColor(AcDbObjectId br_id, const wchar_t * tag, int color_index)
{
	ObjectWrap obj_wrap(OpenObjectById(br_id));

	if (obj_wrap.obj == NULL || !obj_wrap.obj->isKindOf(AcDbBlockReference::desc()))
	{
		return;
	}

	BlkRefWrap br_wrap(AcDbBlockReference::cast(obj_wrap.obj));

	AcDbObjectIterator* prop_iter = br_wrap.blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		Acad::ErrorStatus val = br_wrap.blk_ref->openAttribute(att, att_id, AcDb::kForRead);
		if (Acad::eOk != val)
			continue;

		AttributeWrap att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		att_wrap.attribute->upgradeOpen();
		att_wrap.attribute->setColorIndex(color_index);
	}

	delete prop_iter;
}

void AcadFuncs::UpdateDimForBlock(AcDbDatabase * db, const wchar_t* blk_name)
{
	{
		AcDbBlockTableRecord* rcd = NULL;
		{
			BlkTbl_Wrap blk_tbl(AcadFuncs::GetBlockTable(db));
			blk_tbl.blk_tbl->getAt(blk_name, rcd, AcDb::kForRead);
		}

		AcDbBlockTableRecordIterator *iter = NULL;
		{
			rcd->newIterator(iter);
			rcd->close();
		}

		AcDbEntity* ent = NULL;
		while (!iter->done())
		{
			iter->getEntity(ent, AcDb::kForRead);
			EntityWrap ent_wrap(ent);
			if (ent != NULL && ent->isKindOf(AcDbDimension::desc()))
			{
				AcDbDimension* dim = NULL;
				dim = AcDbDimension::cast(ent);
				wchar_t* rt = dim->dimensionText();
				dim->upgradeOpen();
				dim->recomputeDimBlock();
			}
			ent->recordGraphicsModified();
			iter->step();
		}
	}
}

void AcadFuncs::ChangeLayer(AcDbDatabase * db, wchar_t * base, wchar_t * des, bool del_layer)
{
	Acad::ErrorStatus ret = Acad::eOk;

	wchar_t* layer_trim = new wchar_t[wcslen(base)];
	wcsncpy(layer_trim, base, wcslen(base) - 1);
	layer_trim[wcslen(base) - 1] = '\0';

	AcDbLayerTableIterator* iter = NULL;
	{
		LayerTblWrap layer_tbl(GetLayerTable(db));
		layer_tbl.layer_tbl->newIterator(iter);
	}
	while (!iter->done())
	{
		AcDbLayerTableRecord* rcd = NULL;
		if (iter->getRecord(rcd, AcDb::kForRead) == Acad::eOk)
		{
			LayerTblRcd_Wrap layer_wrap(rcd);
			wchar_t* layer_name = L"";
			layer_wrap.layer_tbl_rcd->getName(layer_name);
			std::size_t length = std::wcslen(base);
			wchar_t cmp_c = base[length - 1];
			if (base[length - 1] == L'*')
			{
				if (!wcsncmp(layer_trim, layer_name, wcslen(layer_trim)))
				{
					MoveAllEntityOnLayer(db, layer_name, des);
					if (del_layer)
					{
						ret = layer_wrap.layer_tbl_rcd->upgradeOpen();
						ret = layer_wrap.layer_tbl_rcd->erase();
					}
				}
			}
			else if (!wcscmp(layer_name, base))
			{
				MoveAllEntityOnLayer(db, layer_name, des);
				if (del_layer)
				{
					ret = layer_wrap.layer_tbl_rcd->upgradeOpen();
					ret = layer_wrap.layer_tbl_rcd->erase();
				}
			}
		}
		iter->step();
	}

	delete[] layer_trim;
	delete iter;
}


bool AcadFuncs::CheckExistedLayer(AcDbDatabase * db, const wchar_t * layer)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbLayerTableIterator* iter = NULL;
	{
		LayerTblWrap layer_tbl(GetLayerTable(db));
		layer_tbl.layer_tbl->newIterator(iter);
	}

	while (!iter->done())
	{
		AcDbLayerTableRecord* rcd = NULL;
		if (iter->getRecord(rcd, AcDb::kForRead) == Acad::eOk)
		{
			LayerTblRcd_Wrap layer_wrap(rcd);
			wchar_t* layer_name;
			layer_wrap.layer_tbl_rcd->getName(layer_name);
			std::size_t length = std::wcslen(layer);
			if (IsMatchRegex((wchar_t*)layer, layer_name))
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

bool AcadFuncs::CheckExistedDimByText(AcDbDatabase * db, const wchar_t * text)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbBlockTableRecordIterator* iter = NULL;
	{
		BlkTblRcd_Wrap rcd(GetModelSpace(db));
		rcd.blk_tbl_rcd->newIterator(iter);
	}

	while (!iter->done())
	{
		AcDbEntity* ent = NULL;
		if (iter->getEntity(ent, AcDb::kForRead) == Acad::eOk)
		{
			EntityWrap ent_wrap(ent);
			if (ent != NULL && ent->isKindOf(AcDbDimension::desc()))
			{
				AcDbDimension* dim = AcDbDimension::cast(ent);
				wchar_t* dim_text = dim->dimensionText();
				if (IsMatchRegex((wchar_t*)text, dim_text))
				{
					delete iter;
					return true;
				}
			}
		}
		iter->step();
	}

	delete iter;
	return false;
}

void AcadFuncs::DeleteDimByTextOverride(AcDbDatabase * db, const wchar_t * text)
{
	AcDbBlockTableRecordIterator* iter = NULL;
	{
		BlkTblRcd_Wrap rcd(GetModelSpace(db));
		rcd.blk_tbl_rcd->newIterator(iter);
	}

	while (!iter->done())
	{
		AcDbEntity* ent = NULL;
		if (iter->getEntity(ent, AcDb::kForRead) == Acad::eOk)
		{
			EntityWrap ent_wrap(ent);
			if (ent != NULL && ent->isKindOf(AcDbDimension::desc()))
			{
				AcDbDimension* dim = AcDbDimension::cast(ent);
				wchar_t* dim_text = dim->dimensionText();
				if (IsMatchRegex((wchar_t*)text, dim_text))
				{
					dim->upgradeOpen();
					dim->erase();
				}
			}
		}
		iter->step();
	}
	delete iter;
}

AcGePoint3dArray AcadFuncs::GetListVertexOfPolyline(AcDbPolyline * pline)
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

AcGePoint3dArray AcadFuncs::GetListVertexOfPolyline(const AcDbObjectId & id)
{
	AcGePoint3dArray pnts;
	if (AcDbObjectId::kNull != id)
	{
		ObjectWrap pl_w(AcadFuncs::OpenObjectById(id));
		if (NULL != pl_w.obj && pl_w.obj->isKindOf(AcDbPolyline::desc()))
		{
			AcDbPolyline* pl = AcDbPolyline::cast(pl_w.obj);
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

double AcadFuncs::GetLengthPolyline(const AcDbObjectId & id)
{
	if (AcDbObjectId::kNull != id)
	{
		double length = 0.0;
		ObjectWrap obj_wrap(AcadFuncs::OpenObjectById(id));
		if (obj_wrap.obj->isKindOf(AcDbPolyline::desc()))
		{
			AcDbPolyline* pl = AcDbPolyline::cast(obj_wrap.obj);
			AcGePolyline3d pl1 = AcGePolyline3d(AcadFuncs::GetListVertexOfPolyline(pl));
			double paramEnd;
			pl->getEndParam(paramEnd);
			pl->getDistAtParam(paramEnd, length);
		}

		return length;
	}
	else
		throw std::exception("Could not get the length of the polyline");
}

double AcadFuncs::GetLengthPolyline(const AcDbObjectId & id, const AcGePoint3d & vertex1, const AcGePoint3d & vertex2)
{
	if (AcDbObjectId::kNull != id)
	{
		double length = 0.0;
		ObjectWrap obj_wrap(AcadFuncs::OpenObjectById(id));
		if (obj_wrap.obj->isKindOf(AcDbPolyline::desc()))
		{
			AcDbPolyline* pl = AcDbPolyline::cast(obj_wrap.obj);
			AcGePolyline3d pl1 = AcGePolyline3d(AcadFuncs::GetListVertexOfPolyline(pl));
			double param1 = pl1.paramOf(vertex1);
			double param2 = pl1.paramOf(vertex2);
			length = abs(pl1.length(param1, param2));
		}
		return length;
	}
	else
		throw std::exception("Could not get the length of the polyline");
}

double AcadFuncs::GetDisToStartOfPolyline(const AcDbObjectId & id, const AcGePoint3d & pnt)
{
	AcDbPolyline* pl = NULL;
	{
		ObjectWrap obj_wrap(OpenObjectById(id));
		if (NULL == obj_wrap.obj || !obj_wrap.obj->isKindOf(AcDbPolyline::desc()))
			return 0.0;

		pl = AcDbPolyline::cast(obj_wrap.obj);
	}

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

double AcadFuncs::GetDisTwoPointOfPolyline(const AcDbObjectId & id, const AcGePoint3d & start, const AcGePoint3d & end)
{
	return std::abs(GetDisToStartOfPolyline(id, start) - GetDisToStartOfPolyline(id, end));
}

AcDbObjectId AcadFuncs::GetDimStyleByName(AcDbDatabase * db, const wchar_t * name)
{
	AcDbObjectId id = AcDbObjectId::kNull;
	DimStlTblRcdWrap rcd_wrap(GetDimStyleTableRecord(db, name));

	if (rcd_wrap.dim_style_tbl_rcd != NULL)
	{
		id = rcd_wrap.dim_style_tbl_rcd->id();
	}

	return id;
}

AcDbLinetypeTableRecord* AcadFuncs::GetLineType(AcDbDatabase * db, const wchar_t * linetype_name)
{

	Acad::ErrorStatus ret = Acad::eOk;
	AcDbLinetypeTable* linetype = NULL;
	ret = db->getLinetypeTable(linetype, AcDb::kForRead);

	if (Acad::eOk != ret)
	{
		return NULL;
	}

	AcDbLinetypeTableRecord* linetype_record = NULL;
	linetype->getAt(linetype_name, linetype_record, AcDb::kForRead);
	linetype_record->close();
	linetype->close();
	return linetype_record;

}

const AcDbEvalVariant AcadFuncs::GetDynBlkValue(const AcDbObjectId& br_id, const wchar_t * name)
{
	ObjectWrap obj_wrap(OpenObjectById(br_id));
	if (NULL == obj_wrap.obj || !obj_wrap.obj->isKindOf(AcDbBlockReference::desc()))
		return AcDbEvalVariant();

	AcDbBlockReference* blk_ref = AcDbBlockReference::cast(obj_wrap.obj);

	DynBlkRefWrap dbr_wrap(new AcDbDynBlockReference(blk_ref));
	AcDbDynBlockReferencePropertyArray props = AcDbDynBlockReferencePropertyArray();
	if (!dbr_wrap.dyn_blk_ref->isDynamicBlock())
		return AcDbEvalVariant();

	dbr_wrap.dyn_blk_ref->getBlockProperties(props);
	for (int i = 0; i < props.length(); i++)
	{
		if (props.at(i).propertyName() == AcString(name))
			return props.at(i).value();
	}

	return AcDbEvalVariant();
}

bool AcadFuncs::DynBlkHas(AcDbObjectId& br_id, const wchar_t * name)
{
	ObjectWrap obj_wrap(OpenObjectById(br_id));
	if (NULL == obj_wrap.obj || !obj_wrap.obj->isKindOf(AcDbBlockReference::desc()))
		return false;

	AcDbBlockReference* blk_ref = AcDbBlockReference::cast(obj_wrap.obj);

	DynBlkRefWrap dbr_wrap(new AcDbDynBlockReference(blk_ref));
	AcDbDynBlockReferencePropertyArray props = AcDbDynBlockReferencePropertyArray();
	if (!dbr_wrap.dyn_blk_ref->isDynamicBlock())
		return false;

	dbr_wrap.dyn_blk_ref->getBlockProperties(props);
	for (int i = 0; i < props.length(); i++)
	{
		if (props.at(i).propertyName() == AcString(name))
			return true;
	}

	return false;
}

void AcadFuncs::SetDoubleDynBlk(const AcDbObjectId & br_id, const wchar_t * name, double val)
{
	ObjectWrap obj_wrap(OpenObjectById(br_id));
	if (NULL == obj_wrap.obj || !obj_wrap.obj->isKindOf(AcDbBlockReference::desc()))
		return;

	AcDbBlockReference* blk_ref = AcDbBlockReference::cast(obj_wrap.obj);

	DynBlkRefWrap dbr_wrap(new AcDbDynBlockReference(blk_ref));
	AcDbDynBlockReferencePropertyArray props = AcDbDynBlockReferencePropertyArray();
	if (!dbr_wrap.dyn_blk_ref->isDynamicBlock())
		return;

	dbr_wrap.dyn_blk_ref->getBlockProperties(props);
	for (int i = 0; i < props.length(); i++)
	{
		if (props.at(i).propertyName() == AcString(name))
		{
			AcDbDynBlockReferenceProperty prop = props.at(i);
			AcDbEvalVariant prop_val = AcDbEvalVariant(val);
			Acad::ErrorStatus err = prop.setValue(prop_val);
			return;
		}
	}
}

std::wstring AcadFuncs::GetPropValue(const AcDbObjectId& id, const wchar_t * tag)
{
	EntityWrap ent_wrap(GetEntityById(id));
	if (NULL == ent_wrap.entity || !ent_wrap.entity->isKindOf(AcDbBlockReference::desc()))
		return std::wstring(L"");

	AcDbBlockReference* blk_ref = AcDbBlockReference::cast(ent_wrap.entity);
	AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		Acad::ErrorStatus val = blk_ref->openAttribute(att, att_id, AcDb::kForRead);
		if (Acad::eOk != val)
			continue;

		AttributeWrap att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		delete prop_iter;
		return std::wstring(att->textString());
	}

	return std::wstring(L"");
}

void AcadFuncs::SetPropValue(const AcDbObjectId& id, const wchar_t * tag, const wchar_t * val)
{
	Acad::ErrorStatus ret = Acad::eOk;
	EntityWrap ent_wrap(GetEntityById(id));
	if (NULL == ent_wrap.entity || !ent_wrap.entity->isKindOf(AcDbBlockReference::desc()))
		return;

	AcDbBlockReference* blk_ref = AcDbBlockReference::cast(ent_wrap.entity);
	AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		if (Acad::eOk != blk_ref->openAttribute(att, att_id, AcDb::kForRead))
			continue;

		AttributeWrap att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		ret = att->upgradeOpen();
		ret = att->setTextString(val);
	}

	delete prop_iter;
}

double AcadFuncs::GetPropRotation(const AcDbObjectId & id, const wchar_t * tag)
{
	EntityWrap ent_wrap(GetEntityById(id));
	if (NULL == ent_wrap.entity || !ent_wrap.entity->isKindOf(AcDbBlockReference::desc()))
		return 0.0;

	AcDbBlockReference* blk_ref = AcDbBlockReference::cast(ent_wrap.entity);
	AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		Acad::ErrorStatus val = blk_ref->openAttribute(att, att_id, AcDb::kForRead);
		if (Acad::eOk != val)
			continue;

		AttributeWrap att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		delete prop_iter;
		return att->rotation();
	}

	return 0.0;
}

void AcadFuncs::SetPropRotation(const AcDbObjectId & id, const wchar_t * tag, double rota)
{
	Acad::ErrorStatus ret = Acad::eOk;
	EntityWrap ent_wrap(GetEntityById(id));
	if (NULL == ent_wrap.entity || !ent_wrap.entity->isKindOf(AcDbBlockReference::desc()))
		return;

	AcDbBlockReference* blk_ref = AcDbBlockReference::cast(ent_wrap.entity);
	AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		if (Acad::eOk != blk_ref->openAttribute(att, att_id, AcDb::kForRead))
			continue;

		AttributeWrap att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		ret = att->upgradeOpen();
		ret = att->setRotation(rota);
	}

	delete prop_iter;
}

AcDbRegAppTable * AcadFuncs::GetRegAppTbl(AcDbDatabase * db)
{
	AcDbRegAppTable* reg_app_tbl = NULL;
	if (Acad::eOk == db->getRegAppTable(reg_app_tbl, AcDb::kForRead))
		return reg_app_tbl;
	return NULL;
}

bool AcadFuncs::HasRegAppName(AcDbDatabase * db, const wchar_t * app_name)
{
	RegAppTblWrap rat_wrap(GetRegAppTbl(db));
	if (Adesk::kTrue == rat_wrap.reg_app_tbl->has(app_name))
		return true;
	return false;
}

void AcadFuncs::AddRegAppName(AcDbDatabase * db, const wchar_t * app_name)
{
	RegAppTblWrap rat_wrap(GetRegAppTbl(db));
	rat_wrap.reg_app_tbl->upgradeOpen();
	AcDbRegAppTableRecord* ratr = new AcDbRegAppTableRecord();
	ratr->setName(app_name);
	if (Acad::eOk != rat_wrap.reg_app_tbl->add(ratr))
		delete ratr;
	else
		ratr->close();
}

void AcadFuncs::AssignAttributeVal(const AcDbObjectId& br_id, const wchar_t * tag, const wchar_t * val)
{
	AcDbObjectId id = GetAttributeByName(br_id, tag);
	ObjectWrap att_wrap(AcadFuncs::OpenObjectById(id));
	AcDbAttribute *att = AcDbAttribute::cast(att_wrap.obj);
	if (NULL != att)
	{
		att->upgradeOpen();
		att->setTextString(val);
		att->downgradeOpen();
	}
}

bool AcadFuncs::GetCentroidClosedPolyline(AcDbDatabase* db, const AcDbObjectId & id, AcGePoint2d& centroid)
{
	//AcDbObjectId reg_id = AcDbObjectId::kNull;
	AcDbRegion* reg = new AcDbRegion();
	{
		EntityWrap ent_wrap(GetEntityById(id));
		if (NULL == ent_wrap.entity || !ent_wrap.entity->isKindOf(AcDbPolyline::desc()))
			return false;

		AcDbVoidPtrArray ents = AcDbVoidPtrArray();
		ents.append(ent_wrap.entity);

		AcDbVoidPtrArray regs;
		Acad::ErrorStatus err = AcDbRegion::createFromCurves(ents, regs);
		if (0 == regs.length())
			return false;
		reg = (AcDbRegion*)regs.first();
	}

	AcGePoint3d origin = AcGePoint3d::kOrigin;
	AcGeVector3d x_axis = AcGeVector3d::kXAxis;
	AcGeVector3d y_axis = AcGeVector3d::kYAxis;
	AcGePlane plane = AcGePlane();
	reg->getPlane(plane);
	plane.getCoordSystem(origin, x_axis, y_axis);

	double perimeter = 0.0;
	double area = 0.0;
	double mom_inerita[2] = { 0.0, 0.0 };
	double prod_inerita = 0.0;
	double prin_moments[2] = { 0.0, 0.0 };
	AcGeVector2d prin_axes[2] = { AcGeVector2d(), AcGeVector2d() };
	double radii_gyration[2] = { 0.0, 0.0 };
	AcGePoint2d extend_low = AcGePoint2d();
	AcGePoint2d extend_hi = AcGePoint2d();
	Acad::ErrorStatus err = reg->getAreaProp(origin, x_axis, y_axis,
		perimeter, area, centroid, mom_inerita, prod_inerita, prin_moments,
		prin_axes, radii_gyration, extend_low, extend_hi);

	delete reg;

	centroid.x += origin.x;
	centroid.y += origin.y;

	if (Acad::eOk != err)
		return false;
	return true;
}

AcDbObjectId AcadFuncs::GetLayerId(AcDbDatabase * db, const wchar_t * layer)
{
	LayerTblRcd_Wrap lyr_rcd_wrap = GetLayerTableRecord(db, layer);
	if (NULL == lyr_rcd_wrap.layer_tbl_rcd)
	{
		LayerTblWrap lyr(GetLayerTable(db));
		LayerTblRcd_Wrap lyr_rcd(new AcDbLayerTableRecord());
		lyr_rcd.layer_tbl_rcd->setName(layer);
		lyr.layer_tbl->upgradeOpen();
		lyr.layer_tbl->add(lyr_rcd.layer_tbl_rcd);
		return lyr_rcd.layer_tbl_rcd->id();
	}
	LayerTblRcd_Wrap lyr_rcd(GetLayerTableRecord(db, layer));
	return lyr_rcd.layer_tbl_rcd->id();
}

void AcadFuncs::RemoveAllPersistentReactors(const AcDbObjectId & id)
{
	EntityWrap ent_wrap(GetEntityById(id));
	if (NULL != ent_wrap.entity)
	{
		ent_wrap.entity->upgradeOpen();
		const AcDbVoidPtrArray* reactor_ids = ent_wrap.entity->reactors();
		if (NULL == reactor_ids || 0 == reactor_ids->length())
			return;

		for (int i = 0; i < reactor_ids->length(); i++)
		{
			if (!acdbPersistentReactorObjectId(reactor_ids->at(i)))
				continue;

			ent_wrap.entity->removePersistentReactor(acdbPersistentReactorObjectId(reactor_ids->at(i)));
		}
	}
}

AcDbObjectIdArray AcadFuncs::GetAllPersistentReactors(const AcDbObjectId & id)
{
	AcDbObjectIdArray ret_ids = AcDbObjectIdArray();
	EntityWrap ent_wrap(GetEntityById(id));
	if (NULL != ent_wrap.entity)
	{
		ent_wrap.entity->upgradeOpen();
		const AcDbVoidPtrArray* reactor_ids = ent_wrap.entity->reactors();
		if (NULL == reactor_ids || 0 == reactor_ids->length())
			return ret_ids;

		for (int i = 0; i < reactor_ids->length(); i++)
		{
			if (!acdbPersistentReactorObjectId(reactor_ids->at(i)))
				continue;
			ret_ids.append(acdbPersistentReactorObjectId(reactor_ids->at(i)));
		}
	}
	return ret_ids;
}

void AcadFuncs::RotateAttribute(const AcDbObjectId & br_id, const wchar_t * att_name, double rot_val)
{
	AcDbObjectId att_id = AcadFuncs::GetAttributeByName(br_id, att_name);

	if (AcDbObjectId::kNull != att_id)
	{
		ObjectWrap att_wrap(AcadFuncs::OpenObjectById(att_id));
		AcDbAttribute *att = AcDbAttribute::cast(att_wrap.obj);
		if (NULL != att)
		{
			att->upgradeOpen();
			Acad::ErrorStatus err = att->setRotation(rot_val);
		}
	}
}

void AcadFuncs::RemoveBlkRef(AcDbDatabase * db, const wchar_t * br_name)
{
	AcDbObjectId br_id = FindBlockByName(db, br_name);
	if (AcDbObjectId::kNull == br_id)
		return;

	ObjectWrap obj_wrap(OpenObjectById(br_id));
	if (NULL == obj_wrap.obj)
		return;

	obj_wrap.obj->upgradeOpen();
	obj_wrap.obj->erase();
}

void AcadFuncs::CreateBlkTblRcd(AcDbDatabase * db, const wchar_t * btr_name)
{
	BlkTbl_Wrap blk_tbl_wrap(GetBlockTable(db));

	blk_tbl_wrap.blk_tbl->upgradeOpen();

	BlkTblRcd_Wrap blk_tbl_rcd_wrap(new AcDbBlockTableRecord());
	blk_tbl_rcd_wrap.blk_tbl_rcd->setName(btr_name);

	if (Acad::eOk != blk_tbl_wrap.blk_tbl->add(blk_tbl_rcd_wrap.blk_tbl_rcd))
	{
		delete blk_tbl_rcd_wrap.blk_tbl_rcd;
		blk_tbl_rcd_wrap.blk_tbl_rcd = NULL;
	}
}

void AcadFuncs::ChangeLayer(AcDbDatabase * db, AcDbObjectIdArray ids, const wchar_t * layer)
{
	for (int i = 0; i < ids.length(); i++)
	{
		EntityWrap ent_wrap(GetEntityById(ids.at(i)));
		if (ent_wrap.entity != NULL)
		{
			ent_wrap.entity->upgradeOpen();
			ent_wrap.entity->setLayer(layer);
		}
	}
}

AcGePoint3d AcadFuncs::GetAttPos(const AcDbObjectId & br_id, const wchar_t * att_name)
{
	AcDbObjectId att_id = GetAttributeByName(br_id, att_name);
	if (AcDbObjectId::kNull == att_id)
		return AcGePoint3d::kOrigin;

	ObjectWrap obj_wrap(OpenObjectById(att_id));
	if (NULL == obj_wrap.obj || !obj_wrap.obj->isKindOf(AcDbAttribute::desc()))
		return AcGePoint3d::kOrigin;

	AcDbAttribute* att = AcDbAttribute::cast(obj_wrap.obj);
	return att->position();
}

AcGePoint3d AcadFuncs::GetAllignmentPoint(const AcDbObjectId & br_id, const wchar_t * att_name)
{
	AcDbObjectId att_id = AcadFuncs::GetAttributeByName(br_id, att_name);
	if (att_id.isNull())
		return AcGePoint3d::kOrigin;
	ObjectWrap obj_wrap(OpenObjectById(att_id));
	if (obj_wrap.obj == NULL)
		return AcGePoint3d::kOrigin;
	if (!obj_wrap.obj->isKindOf(AcDbAttribute::desc()))
		return AcGePoint3d::kOrigin;
	AcDbAttribute * att = AcDbAttribute::cast(obj_wrap.obj);
	return att->alignmentPoint();
}

void AcadFuncs::SetXData(AcDbDatabase * db, const AcDbObjectId & id, const resbuf * xdata)
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

	AcDbEntity* ent = AcadFuncs::GetEntityById(id);
	ent->upgradeOpen();

	Acad::ErrorStatus err = ent->setXData(xdata);
	ent->close();
}

void AcadFuncs::GetIdsToAds(const std::list<AcDbObjectId>& ids, ads_name & ss)
{
	acedSSAdd(NULL, NULL, ss);
	for (std::list<AcDbObjectId>::const_iterator id = ids.begin(); id != ids.end(); ++id)
	{
		ads_name picking;
		acdbGetAdsName(picking, *id);
		acedSSAdd(picking, ss, ss);
	}
	return;
}

static resbuf *AppendToResbuf(resbuf *head, // Head of list
	resbuf *tail)  // New tail of list
{
	if (head == NULL) return tail;
	resbuf *rb = head;
	while (rb->rbnext) rb = rb->rbnext;
	rb->rbnext = tail;
	return head;
}
AcDbObjectIdArray AcadFuncs::GetEntsInsidePolyline(const AcDbObjectId & id)
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AcGePoint3dArray pts = AcGePoint3dArray();

	{
		ObjectWrap obj_wrap(AcadFuncs::OpenObjectById(id));
		if (obj_wrap.obj->isKindOf(AcDbPolyline::desc()))
		{
			AcDbPolyline* pl = AcDbPolyline::cast(obj_wrap.obj);
			if (pl->isClosed())
			{
				pts = AcadFuncs::GetListVertexOfPolyline(pl);
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
		pnts = AppendToResbuf(pnts,
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

void AcadFuncs::ZoomIntoZone(const AcDbObjectId & id)
{
	ads_name ssname;
	acdbGetAdsName(ssname, id);
	int rtrn = acedCommandS(RTSTR, L"_ZOOM", RTSTR, L"OBJECT", RTENAME, ssname, RTSTR, L"", RTNONE);
	acedSSFree(ssname);
}

void AcadFuncs::ZoomIntoZoneExtent(const AcDbObjectId & id, int exten_val)
{
	ads_point p1 = { 0, 0, 0 };
	ads_point p2 = { 0, 0, 0 };
	if (AcDbObjectId::kNull != id)
	{
		AcDbExtents ex = AcDbExtents();
		ObjectWrap obj_w(AcadFuncs::OpenObjectById(id));
		EntityWrap ent_w(AcadFuncs::GetEntityById(id));
		ent_w.entity->getGeomExtents(ex);
		ConvertToAdsPoint(ex.minPoint() + AcGeVector3d(-500, -500, 0), p1);
		ConvertToAdsPoint(ex.maxPoint() + AcGeVector3d(+500, +500, 0), p2);
		if (RTNORM != acedCommandS(RTSTR, L"_ZOOM", RTSTR, L"W", RTPOINT, p1, RTPOINT, p2, RTNONE))
			return;
	}
}

AcDbObjectIdArray AcadFuncs::GetIdsFromSelectionSet(const ads_name & an)
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

AcDbObjectIdArray AcadFuncs::GeLastCreatedObjId()
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AdsNameWrap anw;
	acedSSGet(L"L", NULL, NULL, NULL, anw.ads);
	AcDbHandle hndl = AcDbHandle(anw.ads[0], anw.ads[1]);
	if (!hndl.isNull())
		acedGetCurrentSelectionSet(ids);
	return ids;
}

AcDbObjectIdArray AcadFuncs::GetObjIdsByPicking()
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

AcDbObjectIdArray AcadFuncs::GetObjIdsByPicking(wchar_t * prompt)
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

AcDbObjectIdArray AcadFuncs::GetObjIdsInSelected()
{
	AcDbObjectIdArray ids = AcDbObjectIdArray();
	AdsNameWrap anw;
	acedSSGet(NULL, NULL, NULL, NULL, anw.ads);
	AcDbHandle hndl = AcDbHandle(anw.ads[0], anw.ads[1]);
	if (!hndl.isNull())
		Acad::ErrorStatus err = acedGetCurrentSelectionSet(ids);

	return ids;
}

AcDbObjectIdArray AcadFuncs::GetObjIdsInSelected(wchar_t* prompt)
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

void AcadFuncs::ConvertToAdsPoint(AcGePoint3d pnt, ads_point ads_pnt)
{
	ads_pnt[X] = pnt.x;
	ads_pnt[Y] = pnt.y;
	ads_pnt[Z] = pnt.z;
}

AcGePoint3d AcadFuncs::ConvertAdsToPoint(const ads_point ads)
{
	return AcGePoint3d(ads[0], ads[1], ads[2]);
}

AcDbObjectIdArray AcadFuncs::UserGetEnts()
{

	AdsNameWrap ads_wrap;
	// Get the current PICKFIRST or ask user for a selection
	acedSSGet(NULL, NULL, NULL, NULL, ads_wrap.ads);
	// Get the length (how many entities were selected)

	return GetEntsFromSS(ads_wrap.ads);
}

AcGePoint3d AcadFuncs::UserGetPoint(std::wstring prompt)
{
	ads_point ads;

	int chk = acedGetPoint(NULL, prompt.c_str(), ads);
	if (chk != RTNORM)
	{
		acutPrintf(L"Something incorrect!");
		throw RTCAN;
	}
	return ConvertAdsToPoint(ads);
}

#define MAX_DISTANCE_TENDPROP_TO_SLAB		3000
AcGePoint3d AcadFuncs::GetMinPerpendicularPoint(AcDbPolyline * pl, AcGePoint3d check_pnt)
{
	AcGePoint3d return_pnt = AcGePoint3d::kOrigin;
	AcDbExtents ext = AcDbExtents();
	pl->getGeomExtents(ext);
	double distance = ext.minPoint().distanceTo(ext.maxPoint()) * 1000;
	AcGePoint3dArray pnts = AcadFuncs::GetListVertexOfPolyline(pl);
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

AcDbObjectIdArray AcadFuncs::GetEntsFromSS(const ads_name & ss)
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

void AcadFuncs::MergeIntoPolyline(const AcDbObjectIdArray & ids)
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
AcGePoint3dArray AcadFuncs::GetVerticesBetweenTwoPoint(const AcDbObjectId & cable_id, const AcGePoint3d & pnt_1, const AcGePoint3d & pnt_2)
{
	AcGePoint3dArray ret_points = AcGePoint3dArray();
	AcGeTol tol = AcGeTol();
	tol.setEqualPoint(1e-1);
	if (AcDbObjectId::kNull != cable_id)
	{
		AcGePoint3dArray tmp_pnts = AcGePoint3dArray();
		ObjectWrap pl_w(AcadFuncs::OpenObjectById(cable_id));
		if (NULL != pl_w.obj && pl_w.obj->isKindOf(AcDbPolyline::desc()))
		{
			AcDbPolyline * pl = AcDbPolyline::cast(pl_w.obj);
			AcGePoint3dArray vertices = AcadFuncs::GetListVertexOfPolyline(pl);
			bool first = true;
			AcGePoint3d end_check = AcGePoint3d::kOrigin;
			for (int i = 0; i < vertices.length() - 1; i++)
			{
				AcGePoint3d ver_1 = vertices.at(i);
				AcGePoint3d ver_2 = vertices.at(i + 1);
				if (first)
				{
					if (AcadFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_1) &&
						AcadFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_2))
						return ret_points;
					if (AcadFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_1) &&
						!AcadFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_2))
						if (first)
						{
							if (ver_1.isEqualTo(pnt_1, tol))
								tmp_pnts.append(vertices.at(i));
							end_check = pnt_2;
							first = false;
						}
					if (AcadFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_2) &&
						!AcadFuncs::IsPointOnStraightline(ver_1, ver_2, pnt_1))
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
					if (AcadFuncs::IsPointOnStraightline(ver_1, ver_2, end_check))
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

bool AcadFuncs::IsPointOnStraightline(const AcGePoint3d & sp, const AcGePoint3d & ep, const AcGePoint3d & pnt)
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

int AcadFuncs::GetInt(std::wstring promp)
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

AcDbObjectIdArray AcadFuncs::GetObjIdsInWindow(const AcGePoint3d& pnt1, const AcGePoint3d& pnt2)
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

void AcadFuncs::DeleteLayer(AcDbDatabase * db, wchar_t * layer)
{
	Acad::ErrorStatus ret = Acad::eOk;

	wchar_t* layer_trim = new wchar_t[wcslen(layer)];
	wcsncpy(layer_trim, layer, wcslen(layer) - 1);
	layer_trim[wcslen(layer) - 1] = '\0';

	AcDbLayerTableIterator* iter = NULL;
	{
		LayerTblWrap layer_tbl(GetLayerTable(db));
		layer_tbl.layer_tbl->newIterator(iter);
	}
	while (!iter->done())
	{
		AcDbLayerTableRecord* rcd = NULL;
		if (iter->getRecord(rcd, AcDb::kForRead) == Acad::eOk)
		{
			LayerTblRcd_Wrap layer_wrap(rcd);
			wchar_t* layer_name;
			layer_wrap.layer_tbl_rcd->getName(layer_name);
			std::size_t length = std::wcslen(layer);
			wchar_t cmp_c = layer[length - 1];
			if (layer[length - 1] == L'*')
			{
				if (!wcsncmp(layer_trim, layer_name, wcslen(layer_trim)))
				{
					EraseAllEntityOnLayer(db, layer_name);
					ret = layer_wrap.layer_tbl_rcd->upgradeOpen();
					ret = layer_wrap.layer_tbl_rcd->erase();
				}
			}
			else if (!wcscmp(layer_name, layer))
			{
				EraseAllEntityOnLayer(db, layer_name);
				layer_wrap.layer_tbl_rcd->upgradeOpen();
				layer_wrap.layer_tbl_rcd->erase();
			}
		}
		iter->step();
	}

	delete[] layer_trim;
	delete iter;
}

void AcadFuncs::DeleteLayerByRegex(AcDbDatabase * db, wchar_t * regex)
{
	AcDbLayerTableIterator* iter = NULL;
	{
		LayerTblWrap layer_tbl(GetLayerTable(db));
		layer_tbl.layer_tbl->newIterator(iter);
	}
	while (!iter->done())
	{
		AcDbLayerTableRecord* rcd = NULL;
		if (Acad::eOk == iter->getRecord(rcd, AcDb::kForRead))
		{
			LayerTblRcd_Wrap layer_wrap(rcd);
			wchar_t* layer_name = L"";
			layer_wrap.layer_tbl_rcd->getName(layer_name);

			if (IsMatchRegex(regex, layer_name))
			{
				EraseAllEntityOnLayer(db, layer_name);
				layer_wrap.layer_tbl_rcd->upgradeOpen();
				layer_wrap.layer_tbl_rcd->erase();
			}
		}
		iter->step();
	}

	delete iter;
}

void AcadFuncs::DeleteLayerFromFile(const wchar_t * file_name, const wchar_t * layer)
{
	AcDbDatabase* db = new AcDbDatabase(Adesk::kFalse);
	if (db->readDwgFile(file_name) != Acad::eOk)
		throw std::exception("Read dwg failed");
	else
		acdbHostApplicationServices()->setWorkingDatabase(db);
	DeleteLayerByRegex(db, (wchar_t*)layer);
	delete db;
}

void AcadFuncs::EraseAllEntityOnLayer(AcDbDatabase * db, wchar_t * layer)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbBlockTableRecordIterator *iter = NULL;
	{
		BlkTblRcd_Wrap model_space = AcadFuncs::GetModelSpace(db);
		model_space.blk_tbl_rcd->newIterator(iter);
	}

	while (!iter->done())
	{
		EntityWrap ent_wrap(GetEntity(iter));
		if (ent_wrap.entity != NULL && wcscmp(ent_wrap.entity->layer(), layer) == 0)
		{
			ret = ent_wrap.entity->upgradeOpen();
			ret = ent_wrap.entity->erase();
		}
		iter->step();
	}
	delete iter;
}

void AcadFuncs::MoveAllEntityOnLayer(AcDbDatabase * db, wchar_t * layer, wchar_t * des)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbBlockTableRecordIterator *iter = NULL;
	{
		BlkTblRcd_Wrap model_space = AcadFuncs::GetModelSpace(db);
		model_space.blk_tbl_rcd->newIterator(iter);
	}

	while (!iter->done())
	{
		EntityWrap ent_wrap(GetEntity(iter));
		if (ent_wrap.entity != NULL && wcscmp(ent_wrap.entity->layer(), layer) == 0)
		{
			ret = ent_wrap.entity->upgradeOpen();
			ret = ent_wrap.entity->setLayer(des);
		}
		iter->step();
	}
	delete iter;
}

AcDbObjectId AcadFuncs::CloneBlock(AcDbDatabase* master, const wchar_t* fn_in, const wchar_t* _blk_name)
{
	//Clone block
	AcDbDatabase *db = new AcDbDatabase(Adesk::kFalse);
	if (db->readDwgFile(fn_in) != Acad::ErrorStatus::eOk)
		throw std::exception("Read dwg failed");
	else
		acdbHostApplicationServices()->setWorkingDatabase(db);

	AcDbObjectId blk_id = AcadFuncs::FindBlockByName(db, _blk_name);
	if (AcDbObjectId::kNull == blk_id)
		return AcDbObjectId::kNull;

	AcDbObjectIdArray ids;
	ids.append(blk_id);

	if (NULL == master)
		throw std::exception("Master is null!");

	AcDbIdMapping id_map;
	id_map.setDestDb(master);

	AcDbObjectId space_id = AcDbObjectId::kNull;
	space_id = master->currentSpaceId();
	master->wblockCloneObjects(ids, space_id, id_map, AcDb::DuplicateRecordCloning::kDrcIgnore, false);

	//Create new block ref using exiting clone block ref
	AcDbObjectId br_id = AcDbObjectId::kNull;
	AcDbObjectId new_blk_id = AcadFuncs::FindBlockByName(master, _blk_name);
	{
		BlkRefWrap br_wrap(new AcDbBlockReference(AcGePoint3d(0.0, 0.0, 0.0), new_blk_id));
		BlkTblRcd_Wrap blk_tbl_rcd_wrap(AcadFuncs::GetModelSpace(master));
		blk_tbl_rcd_wrap.blk_tbl_rcd->upgradeOpen();
		if (Acad::eOk != blk_tbl_rcd_wrap.blk_tbl_rcd->appendAcDbEntity(br_id, br_wrap.blk_ref))
		{
			delete br_wrap.blk_ref;
			br_wrap.blk_ref = NULL;
			return AcDbObjectId::kNull;
		}
	}

	CloneAttributes(br_id, new_blk_id);

	delete db;
	return br_id;
}

AcDbObjectIdArray AcadFuncs::GetDimsByXData(AcDbDatabase * db, const wchar_t * app_name)
{
	AcDbObjectIdArray ids;
	BlkTblRcd_Wrap model_space(GetModelSpace(db));

	AcDbBlockTableRecordIterator *blk_tbl_rcd_iter = NULL;
	model_space.blk_tbl_rcd->newIterator(blk_tbl_rcd_iter);

	while (!blk_tbl_rcd_iter->done())
	{
		EntityWrap ent_wrap(GetEntity(blk_tbl_rcd_iter));
		if (NULL != ResBufWrap(ent_wrap.entity->xData(app_name)).res_buf)
			ids.append(ent_wrap.entity->id());

		blk_tbl_rcd_iter->step();
	}

	delete blk_tbl_rcd_iter;
	return ids;
}

void AcadFuncs::ScaleModelSpace(AcDbDatabase * db, double val)
{
	AcDbBlockTableRecordIterator *blk_tbl_rcd_iter = NULL;
	{
		BlkTblRcd_Wrap btr_wrap(GetModelSpace(db));
		// Set up a block table record iterator to iterate over the attribute definitions.
		btr_wrap.blk_tbl_rcd->newIterator(blk_tbl_rcd_iter);
	}

	AcGeMatrix3d transform = AcGeMatrix3d::kIdentity;
	transform.setToScaling(val);

	while (!blk_tbl_rcd_iter->done())
	{
		EntityWrap ent_wrap(GetEntity(blk_tbl_rcd_iter));
		ent_wrap.entity->upgradeOpen();
		ent_wrap.entity->transformBy(transform);
		blk_tbl_rcd_iter->step();
	}
	delete blk_tbl_rcd_iter;
}

double AcadFuncs::GetDimLength(AcDbObjectId id)
{
	ObjectWrap dim_obj(OpenObjectById(id));

	if (NULL != dim_obj.obj && dim_obj.obj->isKindOf(AcDbDimension::desc()))
	{
		double val = 0.0;
		AcDbDimension::cast(dim_obj.obj)->measurement(val);
		return val;
	}

	return 0.0;
}

void AcadFuncs::EraserDimOnLayer(AcDbDatabase * db, const wchar_t * layer)
{
	Acad::ErrorStatus ret = Acad::eOk;
	BlkTblRcd_Wrap rcd(AcadFuncs::GetModelSpace(db));
	AcDbBlockTableRecordIterator *iter = NULL;
	rcd.blk_tbl_rcd->newIterator(iter);
	AcDbEntity* ent = NULL;
	int i = 0;
	while (!iter->done())
	{
		iter->getEntity(ent, AcDb::kForRead);
		if (ent != NULL && ent->isKindOf(AcDbDimension::desc()))
		{
			if (0 == wcscmp(ent->layer(), layer))
			{
				ret = ent->upgradeOpen();
				ent->erase();
				ent->close();
			}
		}
		iter->step();
	}
	delete iter;
}

void AcadFuncs::DrawOrderFront(AcDbObjectIdArray ids)
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

void AcadFuncs::DrawOrderBot(AcDbObjectIdArray ids)
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

void AcadFuncs::EraseObjects(AcDbObjectIdArray ids)
{
	Acad::ErrorStatus err;
	for (int i = 0; i < ids.length(); i++)
	{
		if (AcDbObjectId::kNull != ids.at(i))
		{
			EntityWrap ent_wrap(AcadFuncs::GetEntityById(ids.at(i)));
			if ((NULL != ent_wrap.entity) && (!ent_wrap.entity->isErased()))
			{
				err = ent_wrap.entity->upgradeOpen();
				err = ent_wrap.entity->erase();
			}
		}
	}
}
