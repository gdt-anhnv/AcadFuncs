#ifndef _LAYER_FUNCS_H_
#define _LAYER_FUNCS_H_

#include "../acad_header.h"

class LayerFuncs
{
public:
	static AcDbLayerTable* GetLayerTable(AcDbDatabase* db);
	static AcDbLayerTableRecord* GetLayerTableRecord(AcDbDatabase* db, const wchar_t* layer_name);
	static bool CheckExistedLayer(AcDbDatabase* db, const wchar_t* layer);
	static void ChangeLayer(AcDbDatabase* db, wchar_t* base, wchar_t* des, bool del_layer = false);
	static void ChangeLayer(AcDbDatabase* db, AcDbObjectIdArray ids, const wchar_t* layer);
	static void DeleteLayer(AcDbDatabase*db, wchar_t* layer);
	static void DeleteLayerByRegex(AcDbDatabase* db, wchar_t* regex);
	static void DeleteLayerFromFile(const wchar_t* file_name, const wchar_t* layer);
	static void EraseAllEntityOnLayer(AcDbDatabase* db, wchar_t* layer);
	static void MoveAllEntityOnLayer(AcDbDatabase* db, wchar_t* layer, wchar_t* des);
	static AcDbObjectId GetLayerId(AcDbDatabase* db, const wchar_t* layer);

	static AcDbObjectIdArray GetEntityBelongToLayer(AcDbDatabase* db, const wchar_t* layer_name);
	static AcDbObjectIdArray GetPointsBelongToLayer(AcDbDatabase* db, const wchar_t* entry_name, const wchar_t* layer_name);
};


#endif // _LAYER_FUNCS_H_


