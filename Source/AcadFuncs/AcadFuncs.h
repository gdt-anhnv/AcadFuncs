#ifndef _ACAD_FUNCS_H_
#define _ACAD_FUNCS_H_

#ifdef _USING_ARX_
#include "../acad_header.h"
#include "../wrap_header.h"

#include "Geometry.h"
#include "XData.h"

#include <iostream>
#include <string>
#include <list>
#else
class AcDbDatabase;
class AcDbBlockTable;
class AcDbBlockTableRecord;
class AcDbEntity;
class AcDbViewportTableRecord;
class AcDbObject;
class AcDbLayerTable;
class AcDbLayerTableRecord;
class AcDbExtents;
class AcGePoint3d;
class AcDbBlockReference;
class AcGeMatrix3d;
class AcDbObjectIdArray;
#endif

class Functions
{
public:
	static resbuf *AppendToResbuf(resbuf *head, // Head of list
		resbuf *tail);  // New tail of list
	static bool IsMatchRegex(wchar_t* regex, wchar_t* source);
	static AcDbEntity* GetEntityInterTableRecord(AcDbBlockTableRecordIterator* iter);
	static AcDbViewportTableRecord* GetActiveViewport(AcDbDatabase* db);
	static AcDbObjectId GetTextStyleId(AcDbDatabase* db, std::wstring& ts);
	static AcDbDimStyleTable* GetDimStyleTable(AcDbDatabase* db);
	static AcDbDimStyleTableRecord* GetDimStyleTableRecord(AcDbDatabase* db, const wchar_t* entryName);
	static AcDbExtents* GetBoundary(AcDbDatabase* db);
	static void GetBoundaryDrawing(const wchar_t* fp, double& width, double& height);
	static AcDbObjectId BindingXref(AcDbDatabase* db, const wchar_t* source_data, const wchar_t* name, AcGePoint3d ins_pnt);
	static void ExplodeBlockReference(AcDbDatabase* db, AcDbObjectId id, bool del_obj);
	static AcDbObjectId CloneBlock(AcDbDatabase* db, const wchar_t* fn_in, const wchar_t* _blk_name);
	static AcDbObjectIdArray GetDimsByXData(AcDbDatabase* db, const wchar_t* app_name);
	static void ScaleModelSpace(AcDbDatabase* db, double val);
	static double GetDimLength(AcDbObjectId id);
	static void EraserDimOnLayer(AcDbDatabase* db, const wchar_t* layer);
	static void UpdateDimForBlock(AcDbDatabase* db, const wchar_t* blk_name); /*Let the dimention show up before clone or binding block*/
	static bool CheckExistedDimByText(AcDbDatabase* db, const wchar_t* text);
	static void DeleteDimByTextOverride(AcDbDatabase* db, const wchar_t* text);
	static AcDbObjectId GetDimStyleByName(AcDbDatabase* db, const wchar_t* name);
	static AcDbLinetypeTableRecord* GetLineType(AcDbDatabase* db, const wchar_t* name);
	static const AcDbEvalVariant GetDynBlkValue(const AcDbObjectId& br_id, const wchar_t* name);
	static bool DynBlkHas(AcDbObjectId& br_id, const wchar_t* name);
	static void SetDoubleDynBlk(const AcDbObjectId& br_id, const wchar_t* name, double val);
	static AcDbRegAppTable* GetRegAppTbl(AcDbDatabase* db);
	static bool HasRegAppName(AcDbDatabase* db, const wchar_t* app_name);
	static void AddRegAppName(AcDbDatabase* db, const wchar_t* app_name);
	static bool GetCentroidClosedPolyline(AcDbDatabase* db, const AcDbObjectId& id, AcGePoint2d& centroid);
	static void RemoveAllPersistentReactors(const AcDbObjectId& id);
	static AcDbObjectIdArray GetAllPersistentReactors(const AcDbObjectId& id);
	static void RemoveBlkRef(AcDbDatabase* db, const wchar_t* br_name);
	static void CreateBlkTblRcd(AcDbDatabase* db, const wchar_t* btr_name);
	static AcGePoint3d GetAllignmentPoint(const AcDbObjectId& br_id, const wchar_t* att_name);
	//datax function
	static void GetIdsToAds(const std::list<AcDbObjectId> & ids, ads_name &ss);
	static void ScaleBlockReference(const AcDbObjectId & id, const int & val);
};

#endif // !_ACAD_FUNCS_H_ 
