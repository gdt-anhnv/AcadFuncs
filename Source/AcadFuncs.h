#ifndef _ACAD_FUNCS_H_
#define _ACAD_FUNCS_H_

#ifdef _USING_ARX_
#include "acad_header.h"
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

class AcadFuncs
{
public:
	static AcDbObjectIdArray GetEntityBelongToLayer(AcDbDatabase* db, const wchar_t* layer_name);
	static AcDbObjectIdArray GetPointsBelongToLayer(AcDbDatabase* db, const wchar_t* entry_name, const wchar_t* layer_name);
	static AcDbBlockTable* GetBlockTable(AcDbDatabase* db);
	static AcDbBlockTableRecord* GetModelSpace(AcDbDatabase* db);
	static AcDbBlockTableRecord* GetBlkTblRcd(AcDbDatabase* db, const wchar_t* blk_name);
	static AcDbEntity* GetEntityById(AcDbObjectId id);
	static AcDbViewportTableRecord* GetActiveViewport(AcDbDatabase* db);
	static AcDbObject* OpenObjectById(AcDbObjectId id);
	static AcDbObjectId GetTextStyleId(AcDbDatabase* db, std::wstring& ts);
	static AcDbLayerTable* GetLayerTable(AcDbDatabase* db);
	static AcDbLayerTableRecord* GetLayerTableRecord(AcDbDatabase* db, const wchar_t* layer_name);
	static AcDbDimStyleTable* GetDimStyleTable(AcDbDatabase* db);
	static AcDbDimStyleTableRecord* GetDimStyleTableRecord(AcDbDatabase* db, const wchar_t* entryName);
	static AcDbExtents* GetBoundary(AcDbDatabase* db);
	static void GetBoundaryDrawing(const wchar_t* fp, double& width, double& height);
	static const std::wstring GetAttValueWithTag(AcDbDatabase*db, const wchar_t* entry_name, const wchar_t* tag);
	static const std::wstring GetAttValueWithTag(AcDbObjectId blk_id, const wchar_t* tag);
	static AcDbObjectId BindingXref(AcDbDatabase* db, const wchar_t* source_data, const wchar_t* name, AcGePoint3d ins_pnt);
	static void ExplodeBlockReference(AcDbDatabase* db, AcDbObjectId id, bool del_obj);
	static AcDbObjectId GetAttributeByName(AcDbObjectId br_id, const wchar_t* att_name);
	static AcDbObjectId FindBlockByName(AcDbDatabase * db, const wchar_t * bn);
	static AcDbObjectIdArray FindBlockRefsByName(AcDbDatabase* db, const wchar_t* name);
	static AcDbObjectId CloneBlock(AcDbDatabase* db, const wchar_t* fn_in, const wchar_t* _blk_name);
	static AcDbObjectIdArray GetDimsByXData(AcDbDatabase* db, const wchar_t* app_name);
	static void ScaleModelSpace(AcDbDatabase* db, double val);
	static double GetDimLength(AcDbObjectId id);
	static void EraserDimOnLayer(AcDbDatabase* db, const wchar_t* layer);
	static void CloneAttributes(AcDbObjectId br_id, AcDbObjectId blk_id);
	static void SetAttributePosition(AcDbObjectId br_id, const wchar_t* tag, AcGePoint3d pos);
	static void SetAttributeColor(AcDbObjectId br_id, const wchar_t* tag, int color_index);
	static void UpdateDimForBlock(AcDbDatabase* db, const wchar_t* blk_name); /*Let the dimention show up before clone or binding block*/
	static void ChangeLayer(AcDbDatabase* db, wchar_t* base, wchar_t* des, bool del_layer = false);
	static void DeleteLayer(AcDbDatabase*db, wchar_t* layer);
	static void DeleteLayerByRegex(AcDbDatabase* db, wchar_t* regex);
	static void DeleteLayerFromFile(const wchar_t* file_name, const wchar_t* layer);
	static void EraseAllEntityOnLayer(AcDbDatabase* db, wchar_t* layer);
	static void MoveAllEntityOnLayer(AcDbDatabase* db, wchar_t* layer, wchar_t* des);
	static bool CheckExistedLayer(AcDbDatabase* db, const wchar_t* layer);
	static bool CheckExistedDimByText(AcDbDatabase* db, const wchar_t* text);
	static void DeleteDimByTextOverride(AcDbDatabase* db, const wchar_t* text);
	static AcGePoint3dArray GetListVertexOfPolyline(AcDbPolyline* pline);
	static AcGePoint3dArray GetListVertexOfPolyline(const AcDbObjectId& id);
	static double GetLengthPolyline(const AcDbObjectId& id);
	static double GetLengthPolyline(const AcDbObjectId & id, const AcGePoint3d &vertex1, const AcGePoint3d &vertex2);
	static double GetDisToStartOfPolyline(const AcDbObjectId& id, const AcGePoint3d& pnt);
	static double GetDisTwoPointOfPolyline(const AcDbObjectId& id, const AcGePoint3d& start, const AcGePoint3d& end);
	static AcDbObjectId GetDimStyleByName(AcDbDatabase* db, const wchar_t* name);
	static AcDbLinetypeTableRecord* GetLineType(AcDbDatabase* db, const wchar_t* name);
	static const AcDbEvalVariant GetDynBlkValue(const AcDbObjectId& br_id, const wchar_t* name);
	static bool DynBlkHas(AcDbObjectId& br_id, const wchar_t* name);
	static void SetDoubleDynBlk(const AcDbObjectId& br_id, const wchar_t* name, double val);
	static std::wstring GetPropValue(const AcDbObjectId& id, const wchar_t* tag);
	static void SetPropValue(const AcDbObjectId& id, const wchar_t* tag, const wchar_t* val);
	static double GetPropRotation(const AcDbObjectId& id, const wchar_t* tag);
	static void SetPropRotation(const AcDbObjectId& id, const wchar_t* tag, double rota);
	static AcDbRegAppTable* GetRegAppTbl(AcDbDatabase* db);
	static bool HasRegAppName(AcDbDatabase* db, const wchar_t* app_name);
	static void AddRegAppName(AcDbDatabase* db, const wchar_t* app_name);
	static void AssignAttributeVal(const AcDbObjectId& br_id, const wchar_t* tag, const wchar_t* val);
	static bool GetCentroidClosedPolyline(AcDbDatabase* db, const AcDbObjectId& id, AcGePoint2d& centroid);
	static AcDbObjectId GetLayerId(AcDbDatabase* db, const wchar_t* layer);
	static void RemoveAllPersistentReactors(const AcDbObjectId& id);
	static AcDbObjectIdArray GetAllPersistentReactors(const AcDbObjectId& id);
	static void RotateAttribute(const AcDbObjectId& br_id, const wchar_t* att_name, double rot_val);
	static void RemoveBlkRef(AcDbDatabase* db, const wchar_t* br_name);
	static void CreateBlkTblRcd(AcDbDatabase* db, const wchar_t* btr_name);
	static void ChangeLayer(AcDbDatabase* db, AcDbObjectIdArray ids, const wchar_t* layer);
	static AcGePoint3d GetAttPos(const AcDbObjectId& br_id, const wchar_t* att_name);
	static AcGePoint3d GetAllignmentPoint(const AcDbObjectId& br_id, const wchar_t* att_name);
	//datax function
	static void SetXData(AcDbDatabase* db, const AcDbObjectId& id, const resbuf* xdata);

	static void GetIdsToAds(const std::list<AcDbObjectId> & ids, ads_name &ss);


	//only use for arx
#ifdef _USING_ARX_
	static AcDbObjectIdArray GetEntsInsidePolyline(const AcDbObjectId & id);
	static void ZoomIntoZone(const AcDbObjectId & id);
	static void ZoomIntoZoneExtent(const AcDbObjectId & id, int exten_val);
	static AcDbObjectIdArray GetIdsFromSelectionSet(const ads_name&);
	static AcDbObjectIdArray GeLastCreatedObjId();
	static AcDbObjectIdArray GetObjIdsByPicking();
	static AcDbObjectIdArray GetObjIdsByPicking(wchar_t* prompt);
	static AcDbObjectIdArray GetObjIdsInSelected();
	static AcDbObjectIdArray GetObjIdsInSelected(wchar_t* prompt);
	static AcDbObjectIdArray GetObjIdsInWindow(const AcGePoint3d& pnt1, const AcGePoint3d& pnt2);
	static void AcadFuncs::ConvertToAdsPoint(AcGePoint3d pnt, ads_point);
	static AcGePoint3d ConvertAdsToPoint(const ads_point);
	static AcDbObjectIdArray UserGetEnts();
	static AcGePoint3d UserGetPoint(std::wstring prompt);
	static AcGePoint3d GetMinPerpendicularPoint(AcDbPolyline * pl, AcGePoint3d check_pnt);
	static AcDbObjectIdArray GetEntsFromSS(const ads_name& ss);
	static void MergeIntoPolyline(const AcDbObjectIdArray& ids);
	static AcGePoint3dArray GetVerticesBetweenTwoPoint(const AcDbObjectId& pl_id, const AcGePoint3d& pnt_1, const AcGePoint3d & pnt_2);
	static bool IsPointOnStraightline(const AcGePoint3d& sp, const AcGePoint3d& ep, const AcGePoint3d& pnt);
	static int GetInt(std::wstring promp);
	static void DrawOrderFront(AcDbObjectIdArray ids);
	static void DrawOrderBot(AcDbObjectIdArray ids);
	static void EraseObjects(AcDbObjectIdArray ids);

#endif
};
#endif // !_ACAD_FUNCS_H_ 
