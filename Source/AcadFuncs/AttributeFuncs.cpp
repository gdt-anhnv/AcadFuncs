#include "AttributeFuncs.h"
#include "../wrap_header.h"
#include "../AcadFuncs/AcadFuncs.h"
#include "../AcadFuncs/DBObject.h"

const std::wstring AttributeFuncs::GetAttValueWithTag(AcDbDatabase * db, const wchar_t* entry_name, const wchar_t * tag)
{
	AcDbBlockTableRecordIterator *iter = NULL;
	{
		ObjectWrap<AcDbBlockTableRecord> btr_wrap(DBObject::GetBlkTblRcd(db, entry_name));
		btr_wrap.object->newIterator(iter);
	}

	while (!iter->done())
	{
		ObjectWrap<AcDbEntity> ent_wrap(Functions::GetEntityInterTableRecord(iter));
		AcDbAttributeDefinition* att_def = NULL;
		if (ent_wrap.object != NULL && ent_wrap.object->isKindOf(AcDbAttributeDefinition::desc()))
		{
			att_def = AcDbAttributeDefinition::cast(ent_wrap.object);
			ent_wrap.object = NULL;
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

const std::wstring AttributeFuncs::GetAttValueWithTag(AcDbObjectId blk_id, const wchar_t * tag)
{
	if (AcDbObjectId::kNull != blk_id)
	{
		AcDbBlockReference* blk_ref = DBObject::OpenObjectById<AcDbBlockReference>(blk_id);
		ObjectWrap<AcDbBlockReference> blk_ref_wrap(blk_ref);
		if (NULL == blk_ref)
			return L"";

		AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

		for (; !prop_iter->done(); prop_iter->step())
		{
			AcDbObjectId att_id = prop_iter->objectId();
			AcDbAttribute* att = NULL;
			if (Acad::eOk != blk_ref->openAttribute(att, att_id, AcDb::kForRead))
				continue;

			ObjectWrap<AcDbAttribute> att_wrap(att);
			if (0 == wcscmp(tag, att_wrap.object->tag()))
			{
				wchar_t* txt = att_wrap.object->textString();
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

AcDbObjectId AttributeFuncs::GetAttributeByName(AcDbObjectId br_id, const wchar_t * att_name)
{
	ObjectWrap<AcDbObject> obj_wrap(DBObject::OpenObjectById<AcDbObject>(br_id));
	if (NULL != obj_wrap.object && obj_wrap.object->isKindOf(AcDbBlockReference::desc()))
	{
		AcDbBlockReference* br = AcDbBlockReference::cast(obj_wrap.object);
		AcDbObjectIterator* att_iter = br->attributeIterator();
		AcDbAttribute *att = NULL;

		while (!att_iter->done())
		{
			br->openAttribute(att, att_iter->objectId(), AcDb::kForRead);
			ObjectWrap<AcDbEntity> ent_wrap(att);
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

void AttributeFuncs::CloneAttributes(AcDbObjectId br_id, AcDbObjectId blk_id)
{
	ObjectWrap<AcDbBlockReference> br_wrap(DBObject::OpenObjectById<AcDbBlockReference>(br_id));
	if (NULL == br_wrap.object)
		return;

	ObjectWrap<AcDbBlockTableRecord> btr_wrap(DBObject::OpenObjectById<AcDbBlockTableRecord>(blk_id));
	if (NULL == btr_wrap.object)
		return;

	// Set up a block table record iterator to iterate over the attribute definitions.
	AcDbBlockTableRecordIterator *blk_tbl_rcd_iter = NULL;
	btr_wrap.object->newIterator(blk_tbl_rcd_iter);

	while (!blk_tbl_rcd_iter->done())
	{
		ObjectWrap<AcDbEntity> ent_wrap(Functions::GetEntityInterTableRecord(blk_tbl_rcd_iter));
		if (ent_wrap.object->isKindOf(AcDbAttributeDefinition::desc()))
		{
			AcDbAttributeDefinition* att_def = AcDbAttributeDefinition::cast(ent_wrap.object);
			if (NULL != att_def)
			{
				ObjectWrap<AcDbAttribute> att_wrap(new AcDbAttribute());
				att_wrap.object->setAttributeFromBlock(att_def, br_wrap.object->blockTransform());
				AcDbObjectId att_id = AcDbObjectId::kNull;
				br_wrap.object->upgradeOpen();
				Acad::ErrorStatus err = br_wrap.object->appendAttribute(att_id, att_wrap.object);
				if (Acad::eOk != err)
				{
					delete att_wrap.object;
					att_wrap.object = NULL;
				}
				br_wrap.object->downgradeOpen();
			}
		}

		blk_tbl_rcd_iter->step();
	}

	delete blk_tbl_rcd_iter;
}

void AttributeFuncs::AssignAttributeVal(const AcDbObjectId& br_id, const wchar_t * tag, const wchar_t * val)
{
	AcDbObjectId id = GetAttributeByName(br_id, tag);
	ObjectWrap<AcDbObject> att_wrap(DBObject::OpenObjectById<AcDbObject>(id));
	AcDbAttribute *att = AcDbAttribute::cast(att_wrap.object);
	if (NULL != att)
	{
		att->upgradeOpen();
		att->setTextString(val);
		att->downgradeOpen();
	}
}

void AttributeFuncs::RotateAttribute(const AcDbObjectId & br_id, const wchar_t * att_name, double rot_val)
{
	AcDbObjectId att_id = GetAttributeByName(br_id, att_name);

	if (AcDbObjectId::kNull != att_id)
	{
		ObjectWrap<AcDbObject> att_wrap(DBObject::OpenObjectById<AcDbObject>(att_id));
		AcDbAttribute *att = AcDbAttribute::cast(att_wrap.object);
		if (NULL != att)
		{
			att->upgradeOpen();
			Acad::ErrorStatus err = att->setRotation(rot_val);
		}
	}
}

AcGePoint3d AttributeFuncs::GetAttributePosition(const AcDbObjectId & br_id, const wchar_t * att_name)
{
	AcDbObjectId att_id = GetAttributeByName(br_id, att_name);
	if (AcDbObjectId::kNull == att_id)
		return AcGePoint3d::kOrigin;


	ObjectWrap<AcDbAttribute> att_wrap(DBObject::OpenObjectById<AcDbAttribute>(att_id));
	return att_wrap.object->position();
}

void AttributeFuncs::SetAttributePosition(AcDbObjectId br_id, const wchar_t * tag, AcGePoint3d pos)
{
		ObjectWrap<AcDbBlockReference> br_wrap(DBObject::OpenObjectById<AcDbBlockReference>(br_id));
		if (NULL == br_wrap.object)
			return;

		AcDbObjectIterator* prop_iter = br_wrap.object->attributeIterator();

		for (; !prop_iter->done(); prop_iter->step())
		{
			AcDbObjectId att_id = prop_iter->objectId();
			AcDbAttribute* att = NULL;
			Acad::ErrorStatus val = br_wrap.object->openAttribute(att, att_id, AcDb::kForRead);
			if (Acad::eOk != val)
				continue;

			ObjectWrap<AcDbAttribute> att_wrap(att);
			if (0 != wcscmp(tag, att->tag()))
				continue;

			att_wrap.object->upgradeOpen();
			att_wrap.object->setPosition(pos);
			att_wrap.object->setAlignmentPoint(pos);
		}

		delete prop_iter;
}

void AttributeFuncs::SetAttributeColor(AcDbObjectId br_id, const wchar_t * tag, int color_index)
{
	ObjectWrap<AcDbBlockReference> br_wrap(DBObject::OpenObjectById<AcDbBlockReference>(br_id));
	if (NULL == br_wrap.object)
		return;

	AcDbObjectIterator* prop_iter = br_wrap.object->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		Acad::ErrorStatus val = br_wrap.object->openAttribute(att, att_id, AcDb::kForRead);
		if (Acad::eOk != val)
			continue;

		ObjectWrap<AcDbAttribute> att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		att_wrap.object->upgradeOpen();
		att_wrap.object->setColorIndex(color_index);
	}

	delete prop_iter;
}

std::wstring AttributeFuncs::GetPropValue(const AcDbObjectId& id, const wchar_t * tag)
{
	AcDbBlockReference* blk_ref = DBObject::OpenObjectById<AcDbBlockReference>(id);
	ObjectWrap<AcDbBlockReference> blk_ref_wrap(blk_ref);
	AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		Acad::ErrorStatus val = blk_ref->openAttribute(att, att_id, AcDb::kForRead);
		if (Acad::eOk != val)
			continue;

		ObjectWrap<AcDbAttribute> att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		delete prop_iter;
		return std::wstring(att->textString());
	}

	return std::wstring(L"");
}

void AttributeFuncs::SetPropValue(const AcDbObjectId& id, const wchar_t * tag, const wchar_t * val)
{
	AcDbBlockReference* blk_ref = DBObject::OpenObjectById<AcDbBlockReference>(id);
	ObjectWrap<AcDbBlockReference> blk_ref_wrap(blk_ref);
	if (NULL == blk_ref)
		return;

	AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		if (Acad::eOk != blk_ref->openAttribute(att, att_id, AcDb::kForRead))
			continue;

		ObjectWrap<AcDbAttribute> att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		att->upgradeOpen();
		att->setTextString(val);
	}

	delete prop_iter;
}

double AttributeFuncs::GetPropRotation(const AcDbObjectId & id, const wchar_t * tag)
{
	AcDbBlockReference* blk_ref = DBObject::OpenObjectById<AcDbBlockReference>(id);
	ObjectWrap<AcDbBlockReference> blk_ref_wrap(blk_ref);
	if (NULL == blk_ref)
		return 0.0;
	AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		Acad::ErrorStatus val = blk_ref->openAttribute(att, att_id, AcDb::kForRead);
		if (Acad::eOk != val)
			continue;

		ObjectWrap<AcDbAttribute> att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		delete prop_iter;
		return att->rotation();
	}

	return 0.0;
}

void AttributeFuncs::SetPropRotation(const AcDbObjectId & id, const wchar_t * tag, double rota)
{
	AcDbBlockReference* blk_ref = DBObject::OpenObjectById<AcDbBlockReference>(id);
	ObjectWrap<AcDbBlockReference> blk_ref_wrap(blk_ref);
	if (NULL == blk_ref)
		return;
	AcDbObjectIterator* prop_iter = blk_ref->attributeIterator();

	for (; !prop_iter->done(); prop_iter->step())
	{
		AcDbObjectId att_id = prop_iter->objectId();
		AcDbAttribute* att = NULL;
		if (Acad::eOk != blk_ref->openAttribute(att, att_id, AcDb::kForRead))
			continue;

		ObjectWrap<AcDbEntity> att_wrap(att);
		if (0 != wcscmp(tag, att->tag()))
			continue;

		att->upgradeOpen();
		att->setRotation(rota);
	}

	delete prop_iter;
}



