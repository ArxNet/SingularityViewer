/** 
 * @file llfloatertopobjects.h
 * @brief Shows top colliders or top scripts
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LL_LLFLOATERTOPOBJECTS_H
#define LL_LLFLOATERTOPOBJECTS_H

#include "llfloater.h"

class LLUICtrl;

// Bits for simulator performance query flags
enum LAND_STAT_FLAGS
{
	STAT_FILTER_BY_PARCEL	= 0x00000001,
	STAT_FILTER_BY_OWNER	= 0x00000002,
	STAT_FILTER_BY_OBJECT	= 0x00000004,
	STAT_FILTER_BY_PARCEL_NAME	= 0x00000008,
	STAT_REQUEST_LAST_ENTRY	= 0x80000000,
};

enum LAND_STAT_REPORT_TYPE
{
	STAT_REPORT_TOP_SCRIPTS = 0,
	STAT_REPORT_TOP_COLLIDERS
};

class LLFloaterTopObjects : public LLFloater, public LLSingleton<LLFloaterTopObjects>
{
	friend class LLSingleton<LLFloaterTopObjects>;
public:
	// Opens the floater on screen.
//	static void show();

	// Opens the floater if it's not on-screen.
	// Juggles the UI based on method = "scripts" or "colliders"
	static void handle_land_reply(LLMessageSystem* msg, void** data);
	void handleReply(LLMessageSystem* msg, void** data);
	
	void clearList();
	void updateSelectionInfo();
	virtual BOOL postBuild();

	void onRefresh();

	static void setMode(U32 mode);

private:
	LLFloaterTopObjects();
	~LLFloaterTopObjects();

	void initColumns(LLCtrlListInterface *list);

	void onCommitObjectsList();
	static void onDoubleClickObjectsList(void* data);
	void onClickShowBeacon();

	void doToObjects(int action, bool all);

	void onReturnAll();
	void onReturnSelected();
	void onDisableAll();
	void onDisableSelected();

	void onTeleportToObject();
	void onKick();
	void onProfile();

	static bool callbackReturnAll(const LLSD& notification, const LLSD& response);
	static bool callbackDisableAll(const LLSD& notification, const LLSD& response);

	void onGetByOwnerName();
	void onGetByObjectName();
	void onGetByParcelName();

	void showBeacon();

private:
	std::string mMethod;

	LLSD mObjectListData;
	uuid_vec_t mObjectListIDs;

	U32 mCurrentMode;
	U32 mFlags;
	std::string mFilter;

	BOOL mInitialized;

	F32 mtotalScore;

	enum
	{
		ACTION_RETURN = 0,
		ACTION_DISABLE
	};

	static LLFloaterTopObjects* sInstance;
};

#endif
