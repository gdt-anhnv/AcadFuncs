#ifndef _ATTRIBUTE_FUNCS_H_
#define _ATTRIBUTE_FUNCS_H_
#include "../acad_header.h"


class AttributeFuncs
{
public:
	static const std::wstring GetAttValueWithTag(AcDbDatabase*db, const wchar_t* entry_name, const wchar_t* tag);
	static const std::wstring GetAttValueWithTag(AcDbObjectId blk_id, const wchar_t* tag);
	static AcDbObjectId GetAttributeByName(AcDbObjectId br_id, const wchar_t* att_name);
	static void CloneAttributes(AcDbObjectId br_id, AcDbObjectId blk_id);
	static void AssignAttributeVal(const AcDbObjectId& br_id, const wchar_t* tag, const wchar_t* val);
	static void RotateAttribute(const AcDbObjectId& br_id, const wchar_t* att_name, double rot_val);
	static AcGePoint3d GetAttributePosition(const AcDbObjectId& br_id, const wchar_t* att_name);
	static void SetAttributePosition(AcDbObjectId br_id, const wchar_t * tag, AcGePoint3d pos);
	static void SetAttributeColor(AcDbObjectId br_id, const wchar_t * tag, int color_index);

	static std::wstring GetPropValue(const AcDbObjectId& id, const wchar_t* tag);
	static void SetPropValue(const AcDbObjectId& id, const wchar_t* tag, const wchar_t* val);
	static double GetPropRotation(const AcDbObjectId& id, const wchar_t* tag);
	static void SetPropRotation(const AcDbObjectId& id, const wchar_t* tag, double rota);
};

#endif // _ATTRIBUTE_FUNCS_H_
