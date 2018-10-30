#ifndef _ARX_FUNCS_H_
#define _ARX_FUNCS_H_

#include "../acad_header.h"

class ARXFuncs
{
public:
	//only use for arx
#ifdef _USING_ARX_
	static AcDbObjectIdArray GetEntsInsidePolyline(const AcDbObjectId & id);
	static void ZoomIntoZone(const AcDbObjectId & id);
	static void ZoomIntoZoneExtent(const AcDbObjectId & id, int exten_val);
	static AcDbObjectIdArray GeLastCreatedObjId();
	static AcDbObjectIdArray GetObjIdsByPicking();
	static AcDbObjectIdArray GetObjIdsByPicking(wchar_t* prompt);
	static AcDbObjectIdArray GetObjIdsInSelected();
	static AcDbObjectIdArray GetObjIdsInSelected(wchar_t* prompt);
	static AcDbObjectIdArray GetObjIdsInWindow(const AcGePoint3d& pnt1, const AcGePoint3d& pnt2);
	static void ConvertToAdsPoint(AcGePoint3d pnt, ads_point);
	static AcGePoint3d ConvertAdsToPoint(const ads_point);
	static AcGePoint3d GetMinPerpendicularPoint(AcDbPolyline * pl, AcGePoint3d check_pnt);
	static void MergeIntoPolyline(const AcDbObjectIdArray& ids);
	static AcGePoint3dArray GetVerticesBetweenTwoPoint(const AcDbObjectId& pl_id, const AcGePoint3d& pnt_1, const AcGePoint3d & pnt_2);
	static void DrawOrderFront(AcDbObjectIdArray ids);
	static void DrawOrderBot(AcDbObjectIdArray ids);
	static void EraseObjects(AcDbObjectIdArray ids);

#endif
};


#endif // !_ARX_FUNCS_H_


