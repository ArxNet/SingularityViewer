/** 
 * @file llhudmanager.cpp
 * @brief LLHUDManager class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llhudmanager.h"

#include "message.h"
#include "object_flags.h"

#include "llagent.h"
#include "llhudeffect.h"
#include "pipeline.h"
#include "llviewercontrol.h"
#include "llviewerobjectlist.h"

// These are loaded from saved settings.
LLColor4 LLHUDManager::sParentColor;
LLColor4 LLHUDManager::sChildColor;

LLHUDManager::LLHUDManager()
{

	LLHUDManager::sParentColor = gColors.getColor("FocusColor");
	// rdw commented out since it's not used.  Also removed from colors_base.xml
	//LLHUDManager::sChildColor = gColors.getColor("FocusSecondaryColor");
}

LLHUDManager::~LLHUDManager()
{
}

static LLFastTimer::DeclareTimer FTM_HUD_EFFECTS("Hud Effects");

void LLHUDManager::updateEffects()
{
	LLFastTimer ftm(FTM_HUD_EFFECTS);
	std::size_t i;
	for (i = 0; i < mHUDEffects.size(); i++)
	{
		LLHUDEffect *hep = mHUDEffects[i];
		if (hep->isDead())
		{
			continue;
		}
		hep->update();
	}
}

void LLHUDManager::sendEffects()
{

	if(!gSavedSettings.getBOOL("BroadcastViewerEffects"))
		return;
	
	std::size_t i;
	for (i = 0; i < mHUDEffects.size(); i++)
	{
		LLHUDEffect *hep = mHUDEffects[i];
		if (hep->isDead())
		{
			llwarns << "Trying to send dead effect!" << llendl;
			continue;
		}
		if (hep->mType < LLHUDObject::LL_HUD_EFFECT_BEAM)
		{
			llwarns << "Trying to send effect of type " << hep->mType << " which isn't really an effect and shouldn't be in this list!" << llendl;
			continue;
		}
		if (hep->getNeedsSendToSim() && hep->getOriginatedHere())
		{
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_ViewerEffect);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_Effect);
			hep->packData(msg);
			hep->setNeedsSendToSim(FALSE);
			if (!hep->isDead())
				gAgent.sendMessage();
		}
	}
}

//static
void LLHUDManager::shutdownClass()
{
	getInstance()->mHUDEffects.clear();
}

void LLHUDManager::cleanupEffects()
{
	std::size_t i = 0;

	while (i < mHUDEffects.size())
	{
		if (mHUDEffects[i]->isDead())
		{
			mHUDEffects.erase(mHUDEffects.begin() + i);
		}
		else
		{
			i++;
		}
	}
}

LLHUDEffect *LLHUDManager::createViewerEffect(const U8 type, BOOL send_to_sim, BOOL originated_here)
{
	// SJB: DO NOT USE addHUDObject!!! Not all LLHUDObjects are LLHUDEffects!
	LLHUDEffect *hep = LLHUDObject::addHUDEffect(type);
	if (!hep)
	{
		return NULL;
	}

	LLUUID tmp;
	tmp.generate();
	hep->setID(tmp);
	hep->setNeedsSendToSim(send_to_sim);
	hep->setOriginatedHere(originated_here);

	mHUDEffects.push_back(hep);
	return hep;
}


//static
void LLHUDManager::processViewerEffect(LLMessageSystem *mesgsys, void **user_data)
{
	LLHUDEffect *effectp = NULL;
	LLUUID effect_id;
	U8 effect_type = 0;
	S32 number_blocks = mesgsys->getNumberOfBlocksFast(_PREHASH_Effect);
	S32 k;

	for (k = 0; k < number_blocks; k++)
	{
		effectp = NULL;
		LLHUDEffect::getIDType(mesgsys, k, effect_id, effect_type);
		S32 i;
		for (i = 0; i < (S32)LLHUDManager::getInstance()->mHUDEffects.size(); i++)
		{
			LLHUDEffect *cur_effectp = LLHUDManager::getInstance()->mHUDEffects[i];
			if (!cur_effectp)
			{
				llwarns << "Null effect in effect manager, skipping" << llendl;
				LLHUDManager::getInstance()->mHUDEffects.erase(LLHUDManager::getInstance()->mHUDEffects.begin() + i);
				i--;
				continue;
			}
			if (cur_effectp->isDead())
			{
	//			llwarns << "Dead effect in effect manager, removing" << llendl;
				LLHUDManager::getInstance()->mHUDEffects.erase(LLHUDManager::getInstance()->mHUDEffects.begin() + i);
				i--;
				continue;
			}
			if (cur_effectp->getID() == effect_id)
			{
				if (cur_effectp->getType() != effect_type)
				{
					llwarns << "Viewer effect update doesn't match old type!" << llendl;
				}
				effectp = cur_effectp;
				break;
			}
		}

		if (effect_type)
		{
			if (!effectp)
			{
				effectp = LLHUDManager::getInstance()->createViewerEffect(effect_type, FALSE, FALSE);
			}

			if (effectp)
			{
				effectp->unpackData(mesgsys, k);
			}
		}
		else
		{
			llwarns << "Received viewer effect of type " << effect_type << " which isn't really an effect!" << llendl;
		}
	}

	//llinfos << "Got viewer effect: " << effect_id << llendl;
}

