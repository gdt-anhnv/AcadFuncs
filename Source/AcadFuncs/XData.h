#ifndef _XDATA_FUNCS_H_
#define _XDATA_FUNCS_H_

#include "../acad_header.h"

#include <iostream>
#include <string>

class XDataFuncs
{
public:
	static void AddXDataIntVal(const AcDbObjectId & id, const std::wstring & app_name, const int & value);
	static void AddXDataWstringVal(const AcDbObjectId & id, const std::wstring & app_name, const ACHAR * value);
	static bool CheckXDataIntVal(const AcDbObjectId &id, const std::wstring & app_name, const int & value);
	static bool CheckXDataStringVal(const AcDbObjectId &id, const std::wstring & app_name, const std::wstring & value);
	static void SetXData(AcDbDatabase* db, const AcDbObjectId& id, const resbuf* xdata);
};

#endif