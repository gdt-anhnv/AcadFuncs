#include "../acad_header.h"
#include "AcadFuncs.h"

#include "DBObject.h"
#include "LayerFuncs.h"
#include "AttributeFuncs.h"

#include "../wrap_header.h"
#include "../constants.h"
#include "dbapserv.h"
#include "adscodes.h"
#include "geassign.h"
#include "acedCmdNF.h"
#include <iostream>
#include <exception>
#include <regex>

//using namespace wrap;



resbuf * Functions::AppendToResbuf(resbuf * head, resbuf * tail)
{
	if (head == NULL) return tail;
	resbuf *rb = head;
	while (rb->rbnext) rb = rb->rbnext;
	rb->rbnext = tail;
	return head;
}

bool Functions::IsMatchRegex(wchar_t * regex, wchar_t * source)
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

AcDbEntity * Functions::GetEntityInterTableRecord(AcDbBlockTableRecordIterator * iter)
{
	{
		AcDbEntity* ent = NULL;
		iter->getEntity(ent, AcDb::kForRead);
		return ent;
	}
}



AcDbViewportTableRecord * Functions::GetActiveViewport(AcDbDatabase * db)
{
	AcDbViewportTable* vpt_tbl = NULL;
	db->getSymbolTable(vpt_tbl, AcDb::kForRead);
	AcDbViewportTableRecord* vpt_tbl_rcd = new AcDbViewportTableRecord();
	vpt_tbl->getAt(ACTIVE_VIEWPORT, vpt_tbl_rcd, AcDb::kForRead);
	vpt_tbl->close();
	return vpt_tbl_rcd;
}


AcDbObjectId Functions::GetTextStyleId(AcDbDatabase * db, std::wstring & ts)
{
	AcDbTextStyleTable *txt_sty_tbl = NULL;
	db->getSymbolTable(txt_sty_tbl, AcDb::kForRead);

	AcDbObjectId _id = AcDbObjectId::kNull;
	txt_sty_tbl->getAt(ts.c_str(), _id);
	txt_sty_tbl->close();

	return _id;
}



AcDbDimStyleTable * Functions::GetDimStyleTable(AcDbDatabase * db)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbDimStyleTable* dim_tbl = NULL;
	ret = db->getDimStyleTable(dim_tbl, AcDb::kForRead);
	return dim_tbl;
}

AcDbDimStyleTableRecord * Functions::GetDimStyleTableRecord(AcDbDatabase * db, const wchar_t * entryName)
{
	ObjectWrap<AcDbDimStyleTable> dim_wrap(GetDimStyleTable(db));
	AcDbDimStyleTableRecord* rcd = NULL;
	dim_wrap.object->getAt(entryName, rcd, AcDb::kForRead);
	return rcd;
}

AcDbExtents* Functions::GetBoundary(AcDbDatabase* db)
{
	AcDbExtents* ext = new AcDbExtents;

	ObjectWrap<AcDbBlockTableRecord> btr_wrap(DBObject::GetModelSpace(db));
	AcDbBlockTableRecordIterator *blk_tbl_rcd_iter = NULL;
	if (Acad::eOk == btr_wrap.object->newIterator(blk_tbl_rcd_iter))
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

void Functions::GetBoundaryDrawing(const wchar_t* fp, double & width, double & height)
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


AcDbObjectId Functions::BindingXref(AcDbDatabase * db, const wchar_t * source_data, const wchar_t* name, AcGePoint3d ins_pnt)
{
	AcDbObjectId obj_id = AcDbObjectId::kNull;
	Acad::ErrorStatus err = acdbAttachXref(db, source_data, name, obj_id);
	if (Acad::eOk != err)
		return AcDbObjectId::kNull;

	AcDbBlockReference* br = new AcDbBlockReference(ins_pnt, obj_id);
	{
		ObjectWrap<AcDbBlockTableRecord> blk_tbl_rcd_wrap(DBObject::GetModelSpace(db));
		AcDbObjectId newEntId = AcDbObjectId::kNull;
		blk_tbl_rcd_wrap.object->upgradeOpen();
		blk_tbl_rcd_wrap.object->appendAcDbEntity(newEntId, br);
	}

	br->close();
	AcDbObjectIdArray binding_ids;
	binding_ids.append(obj_id);
	acdbBindXrefs(db, binding_ids, true);

	return br->id();
}

void Functions::ExplodeBlockReference(AcDbDatabase * db, AcDbObjectId id, bool del_obj)
{
	ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(id));
	if (NULL != obj_wrap.object)
	{
		if (obj_wrap.object->isKindOf(AcDbBlockReference::desc()))
		{
			AcDbBlockReference* br = AcDbBlockReference::cast(obj_wrap.object);
			AcDbVoidPtrArray ents(1);
			Acad::ErrorStatus err = br->explode(ents);

			ObjectWrap<AcDbBlockTableRecord> ms_wrap(DBObject::GetModelSpace(db));
			ms_wrap.object->upgradeOpen();
			AcDbObjectId ins_id = AcDbObjectId::kNull;
			for (int i = 0; i < ents.length(); i++)
			{
				AcDbEntity* sub_ent = AcDbEntity::cast((AcRxObject*)ents[i]);
				if (NULL != sub_ent)
					err = ms_wrap.object->appendAcDbEntity(
						ins_id, ObjectWrap<AcDbEntity>(sub_ent).object);
			}
		}

		if (del_obj)
		{
			obj_wrap.object->upgradeOpen();
			obj_wrap.object->erase();
		}
	}

}


void Functions::UpdateDimForBlock(AcDbDatabase * db, const wchar_t* blk_name)
{
	{
		AcDbBlockTableRecord* rcd = NULL;
		{
			ObjectWrap<AcDbBlockTable> blk_tbl(DBObject::GetBlockTable(db));
			blk_tbl.object->getAt(blk_name, rcd, AcDb::kForRead);
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
			ObjectWrap<AcDbEntity> ent_wrap(ent);
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


bool Functions::CheckExistedDimByText(AcDbDatabase * db, const wchar_t * text)
{
	Acad::ErrorStatus ret = Acad::eOk;
	AcDbBlockTableRecordIterator* iter = NULL;
	{
		ObjectWrap<AcDbBlockTableRecord> rcd(DBObject::GetModelSpace(db));
		rcd.object->newIterator(iter);
	}

	while (!iter->done())
	{
		AcDbEntity* ent = NULL;
		if (iter->getEntity(ent, AcDb::kForRead) == Acad::eOk)
		{
			ObjectWrap<AcDbEntity> ent_wrap(ent);
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

void Functions::DeleteDimByTextOverride(AcDbDatabase * db, const wchar_t * text)
{
	AcDbBlockTableRecordIterator* iter = NULL;
	{
		ObjectWrap<AcDbBlockTableRecord> rcd(DBObject::GetModelSpace(db));
		rcd.object->newIterator(iter);
	}

	while (!iter->done())
	{
		AcDbEntity* ent = NULL;
		if (iter->getEntity(ent, AcDb::kForRead) == Acad::eOk)
		{
			ObjectWrap<AcDbEntity> ent_wrap(ent);
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




AcDbObjectId Functions::GetDimStyleByName(AcDbDatabase * db, const wchar_t * name)
{
	AcDbObjectId id = AcDbObjectId::kNull;
	ObjectWrap<AcDbDimStyleTableRecord> rcd_wrap(GetDimStyleTableRecord(db, name));

	if (rcd_wrap.object != NULL)
	{
		id = rcd_wrap.object->id();
	}

	return id;
}

AcDbLinetypeTableRecord* Functions::GetLineType(AcDbDatabase * db, const wchar_t * linetype_name)
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

const AcDbEvalVariant Functions::GetDynBlkValue(const AcDbObjectId& br_id, const wchar_t * name)
{
	AcDbBlockReference* blk_ref = DBObject::OpenObjectById<AcDbBlockReference>(br_id); 
	if(NULL == blk_ref)
		return AcDbEvalVariant();

	AcDbDynBlockReference* dbr(new AcDbDynBlockReference(blk_ref));


	AcDbDynBlockReferencePropertyArray props = AcDbDynBlockReferencePropertyArray();
	if (!dbr->isDynamicBlock())
		return AcDbEvalVariant();

	dbr->getBlockProperties(props);
	for (int i = 0; i < props.length(); i++)
	{
		if (props.at(i).propertyName() == AcString(name))
			return props.at(i).value();
	}

	delete dbr;
	return AcDbEvalVariant();
}

bool Functions::DynBlkHas(AcDbObjectId& br_id, const wchar_t * name)
{

	AcDbBlockReference* blk_ref = DBObject::OpenObjectById<AcDbBlockReference>(br_id);
	if (NULL == blk_ref)
		return false;

	AcDbDynBlockReference* dbr(new AcDbDynBlockReference(blk_ref));

	AcDbDynBlockReferencePropertyArray props = AcDbDynBlockReferencePropertyArray();
	if (!dbr->isDynamicBlock())
		return false;

	dbr->getBlockProperties(props);
	for (int i = 0; i < props.length(); i++)
	{
		if (props.at(i).propertyName() == AcString(name))
			return true;
	}

	return false;
}

void Functions::SetDoubleDynBlk(const AcDbObjectId & br_id, const wchar_t * name, double val)
{
	AcDbBlockReference* blk_ref = DBObject::OpenObjectById<AcDbBlockReference>(br_id);
	ObjectWrap<AcDbBlockReference> blk_ref_wrap(blk_ref);
	if (NULL == blk_ref)
		return;


	AcDbDynBlockReference* dbr(new AcDbDynBlockReference(blk_ref));

	AcDbDynBlockReferencePropertyArray props = AcDbDynBlockReferencePropertyArray();
	if (!dbr->isDynamicBlock())
		return;

	dbr->getBlockProperties(props);
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



AcDbRegAppTable * Functions::GetRegAppTbl(AcDbDatabase * db)
{
	AcDbRegAppTable* reg_app_tbl = NULL;
	if (Acad::eOk == db->getRegAppTable(reg_app_tbl, AcDb::kForRead))
		return reg_app_tbl;
	return NULL;
}

bool Functions::HasRegAppName(AcDbDatabase * db, const wchar_t * app_name)
{
	ObjectWrap<AcDbRegAppTable> rat_wrap(GetRegAppTbl(db));
	if (Adesk::kTrue == rat_wrap.object->has(app_name))
		return true;
	return false;
}

void Functions::AddRegAppName(AcDbDatabase * db, const wchar_t * app_name)
{
	ObjectWrap<AcDbRegAppTable> rat_wrap(GetRegAppTbl(db));
	rat_wrap.object->upgradeOpen();
	AcDbRegAppTableRecord* ratr = new AcDbRegAppTableRecord();
	ratr->setName(app_name);
	if (Acad::eOk != rat_wrap.object->add(ratr))
		delete ratr;
	else
		ratr->close();
}



bool Functions::GetCentroidClosedPolyline(AcDbDatabase* db, const AcDbObjectId & id, AcGePoint2d& centroid)
{
	//AcDbObjectId reg_id = AcDbObjectId::kNull;
	AcDbRegion* reg = new AcDbRegion();
	{
		AcDbPolyline* pl = DBObject::OpenObjectById<AcDbPolyline>(id);
		ObjectWrap<AcDbPolyline> pl_wrap(pl);
		if (NULL == pl)
			return false;

		AcDbVoidPtrArray ents = AcDbVoidPtrArray();
		ents.append(pl);

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

void Functions::RemoveAllPersistentReactors(const AcDbObjectId & id)
{
	ObjectWrap<AcDbEntity> ent_wrap(DBObject::OpenObjectById<AcDbEntity>(id));
	if (NULL != ent_wrap.object)
	{
		ent_wrap.object->upgradeOpen();
		const AcDbVoidPtrArray* reactor_ids = ent_wrap.object->reactors();
		if (NULL == reactor_ids || 0 == reactor_ids->length())
			return;

		for (int i = 0; i < reactor_ids->length(); i++)
		{
			if (!acdbPersistentReactorObjectId(reactor_ids->at(i)))
				continue;

			ent_wrap.object->removePersistentReactor(acdbPersistentReactorObjectId(reactor_ids->at(i)));
		}
	}
}

AcDbObjectIdArray Functions::GetAllPersistentReactors(const AcDbObjectId & id)
{
	AcDbObjectIdArray ret_ids = AcDbObjectIdArray();
	ObjectWrap<AcDbEntity> ent_wrap(DBObject::OpenObjectById<AcDbEntity>(id));
	if (NULL != ent_wrap.object)
	{
		ent_wrap.object->upgradeOpen();
		const AcDbVoidPtrArray* reactor_ids = ent_wrap.object->reactors();
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


void Functions::RemoveBlkRef(AcDbDatabase * db, const wchar_t * br_name)
{
	AcDbObjectId br_id = DBObject::FindBlockByName(db, br_name);
	if (AcDbObjectId::kNull == br_id)
		return;

	ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(br_id));
	if (NULL == obj_wrap.object)
		return;

	obj_wrap.object->upgradeOpen();
	obj_wrap.object->erase();
}

void Functions::CreateBlkTblRcd(AcDbDatabase * db, const wchar_t * btr_name)
{
	ObjectWrap<AcDbBlockTable> blk_tbl_wrap(DBObject::GetBlockTable(db));

	blk_tbl_wrap.object->upgradeOpen();

	ObjectWrap<AcDbBlockTableRecord> blk_tbl_rcd_wrap(new AcDbBlockTableRecord());
	blk_tbl_rcd_wrap.object->setName(btr_name);

	if (Acad::eOk != blk_tbl_wrap.object->add(blk_tbl_rcd_wrap.object))
	{
		delete blk_tbl_rcd_wrap.object;
		blk_tbl_rcd_wrap.object = NULL;
	}
}


AcGePoint3d Functions::GetAllignmentPoint(const AcDbObjectId & br_id, const wchar_t * att_name)
{
	AcDbObjectId att_id = AttributeFuncs::GetAttributeByName(br_id, att_name);
	if (att_id.isNull())
		return AcGePoint3d::kOrigin;
	ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(att_id));
	if (obj_wrap.object == NULL)
		return AcGePoint3d::kOrigin;
	if (!obj_wrap.object->isKindOf(AcDbAttribute::desc()))
		return AcGePoint3d::kOrigin;
	AcDbAttribute * att = AcDbAttribute::cast(obj_wrap.object);
	return att->alignmentPoint();
}


void Functions::GetIdsToAds(const std::list<AcDbObjectId>& ids, ads_name & ss)
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

void Functions::ScaleBlockReference(const AcDbObjectId & id, const int & val)
{
	ObjectWrap<AcDbBlockReference> br_wrap(DBObject::OpenObjectById<AcDbBlockReference>(id));
	if (NULL == br_wrap.object)
		return;
	br_wrap.object->upgradeOpen();

	AcGeMatrix3d trans = br_wrap.object->blockTransform();
	trans.setToScaling(val, br_wrap.object->position());
	br_wrap.object->transformBy(trans);
}



AcDbObjectId Functions::CloneBlock(AcDbDatabase* master, const wchar_t* fn_in, const wchar_t* _blk_name)
{
	//Clone block
	AcDbDatabase *db = new AcDbDatabase(Adesk::kFalse);
	if (db->readDwgFile(fn_in) != Acad::ErrorStatus::eOk)
		throw std::exception("Read dwg failed");
	else
		acdbHostApplicationServices()->setWorkingDatabase(db);

	AcDbObjectId blk_id = DBObject::FindBlockByName(db, _blk_name);
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
	AcDbObjectId new_blk_id = DBObject::FindBlockByName(master, _blk_name);
	{
		ObjectWrap<AcDbBlockReference> br_wrap(new AcDbBlockReference(AcGePoint3d(0.0, 0.0, 0.0), new_blk_id));
		ObjectWrap<AcDbBlockTableRecord> blk_tbl_rcd_wrap(DBObject::GetModelSpace(master));
		blk_tbl_rcd_wrap.object->upgradeOpen();
		if (Acad::eOk != blk_tbl_rcd_wrap.object->appendAcDbEntity(br_id, br_wrap.object))
		{
			delete br_wrap.object;
			br_wrap.object = NULL;
			return AcDbObjectId::kNull;
		}
	}

	AttributeFuncs::CloneAttributes(br_id, new_blk_id);

	delete db;
	return br_id;
}

AcDbObjectIdArray Functions::GetDimsByXData(AcDbDatabase * db, const wchar_t * app_name)
{
	AcDbObjectIdArray ids;
	ObjectWrap<AcDbBlockTableRecord> model_space(DBObject::GetModelSpace(db));

	AcDbBlockTableRecordIterator *blk_tbl_rcd_iter = NULL;
	model_space.object->newIterator(blk_tbl_rcd_iter);

	while (!blk_tbl_rcd_iter->done())
	{
		ObjectWrap<AcDbEntity> ent_wrap(GetEntityInterTableRecord(blk_tbl_rcd_iter));
		if (NULL != ResBufWrap(ent_wrap.object->xData(app_name)).res_buf)
			ids.append(ent_wrap.object->id());

		blk_tbl_rcd_iter->step();
	}

	delete blk_tbl_rcd_iter;
	return ids;
}

void Functions::ScaleModelSpace(AcDbDatabase * db, double val)
{
	AcDbBlockTableRecordIterator *blk_tbl_rcd_iter = NULL;
	{
		ObjectWrap<AcDbBlockTableRecord> btr_wrap(DBObject::GetModelSpace(db));
		// Set up a block table record iterator to iterate over the attribute definitions.
		btr_wrap.object->newIterator(blk_tbl_rcd_iter);
	}

	AcGeMatrix3d transform = AcGeMatrix3d::kIdentity;
	transform.setToScaling(val);

	while (!blk_tbl_rcd_iter->done())
	{
		ObjectWrap<AcDbEntity> ent_wrap(GetEntityInterTableRecord(blk_tbl_rcd_iter));
		ent_wrap.object->upgradeOpen();
		ent_wrap.object->transformBy(transform);
		blk_tbl_rcd_iter->step();
	}
	delete blk_tbl_rcd_iter;
}

double Functions::GetDimLength(AcDbObjectId id)
{
	ObjectWrap<AcDbObject> dim_obj(DBObject::OpenObjectById<AcDbObject>(id));

	if (NULL != dim_obj.object && dim_obj.object->isKindOf(AcDbDimension::desc()))
	{
		double val = 0.0;
		AcDbDimension::cast(dim_obj.object)->measurement(val);
		return val;
	}

	return 0.0;
}

void Functions::EraserDimOnLayer(AcDbDatabase * db, const wchar_t * layer)
{
	Acad::ErrorStatus ret = Acad::eOk;
	ObjectWrap<AcDbBlockTableRecord> rcd(DBObject::GetModelSpace(db));
	AcDbBlockTableRecordIterator *iter = NULL;
	rcd.object->newIterator(iter);
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


