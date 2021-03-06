/** 
 * @file slfloatermediafilter.cpp
 * @brief The SLFloaterMediaFilter class definitions
 *
 * $LicenseInfo:firstyear=2011&license=viewergpl$
 * 
 * Copyright (c) 2011, Sione Lomu
 * with debugging and improvements by Henri Beauchamp
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

#include "slfloatermediafilter.h"

#include "llscrolllistctrl.h"
#include "lluictrlfactory.h" 
#include "llviewerparcelmedia.h"

SLFloaterMediaFilter::SLFloaterMediaFilter(const LLSD& key) : LLFloater(std::string("media filter")), mIsDirty(false)
{
    LLUICtrlFactory::getInstance()->buildFloater(this, "floater_media_filter.xml");
}

SLFloaterMediaFilter::~SLFloaterMediaFilter()
{
}

BOOL SLFloaterMediaFilter::postBuild()
{
	mWhitelistSLC = getChild<LLScrollListCtrl>("whitelist_list");
	mBlacklistSLC = getChild<LLScrollListCtrl>("blacklist_list");

	if (mWhitelistSLC && mBlacklistSLC)
	{
		getChild<LLUICtrl>("clear_lists")->setCommitCallback(boost::bind(LLViewerParcelMedia::clearDomainFilterList));
		getChild<LLUICtrl>("show_ips")->setCommitCallback(boost::bind(&SLFloaterMediaFilter::onShowIPs, this));
		getChild<LLUICtrl>("add_whitelist")->setCommitCallback(boost::bind(&SLFloaterMediaFilter::onWhitelistAdd, this));
		getChild<LLUICtrl>("remove_whitelist")->setCommitCallback(boost::bind(&SLFloaterMediaFilter::onWhitelistRemove, this));
		getChild<LLUICtrl>("add_blacklist")->setCommitCallback(boost::bind(&SLFloaterMediaFilter::onBlacklistAdd, this));
		getChild<LLUICtrl>("remove_blacklist")->setCommitCallback(boost::bind(&SLFloaterMediaFilter::onBlacklistRemove, this));
		getChild<LLUICtrl>("commit_domain")->setCommitCallback(boost::bind(&SLFloaterMediaFilter::onCommitDomain, this));
		mIsDirty = true;
	}

	return TRUE;
}

void SLFloaterMediaFilter::draw()
{
	if (mIsDirty && mWhitelistSLC && mBlacklistSLC)
	{
		S32 whitescrollpos = mWhitelistSLC->getScrollPos();
		S32 blackscrollpos = mBlacklistSLC->getScrollPos();
		mWhitelistSLC->deleteAllItems();
		mBlacklistSLC->deleteAllItems();
		std::set<std::string> listed;
		LLHost host;
		std::string ip;
		std::string domain;
		std::string action;
		LLSD element;
		element["columns"][0]["font"] = "SANSSERIF";
		element["columns"][0]["font-style"] = "BOLD";
		for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
		{
			domain = LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString();
			if (mShowIPs)
			{
				host.setHostByName(domain);
				ip = host.getIPString();
				if (ip != domain && domain.find('/') == std::string::npos)
				{
					domain += " (" + ip + ")";
				}
			}

			action = LLViewerParcelMedia::sMediaFilterList[i]["action"].asString();
			if (!domain.empty() && action == "allow")
			{	
				element["columns"][0]["column"] = "whitelist_col";
				element["columns"][0]["value"] = domain;
				//element["columns"][0]["color"] = LLColor4::green3.getValue();
				mWhitelistSLC->addElement(element, ADD_BOTTOM);
				listed.insert(domain);
			}
			else if (!domain.empty() && action == "deny")
			{
				element["columns"][0]["column"] = "blacklist_col";
				element["columns"][0]["value"] = domain;
				//element["columns"][0]["color"] = LLColor4::red2.getValue();
				mBlacklistSLC->addElement(element, ADD_BOTTOM);
				listed.insert(domain);
			}
			else
			{
				LL_WARNS("MediaFilter") << "Bad media filter list: removing corrupted entry for \"" << domain << "\"" << LL_ENDL;
				LLViewerParcelMedia::sMediaFilterList.erase(i--);
			}
		}
		std::set<std::string>::iterator it;
		element["columns"][0]["font"] = "SANSSERIF";
		element["columns"][0]["font-style"] = "ITALIC";
		//element["columns"][0]["color"] = LLColor4::green3.getValue();
		element["columns"][0]["column"] = "whitelist_col";
		for (it = LLViewerParcelMedia::sAllowedMedia.begin(); it != LLViewerParcelMedia::sAllowedMedia.end(); it++)
		{
			domain = *it;
			if (mShowIPs)
			{
				host.setHostByName(domain);
				ip = host.getIPString();
				if (ip != domain && domain.find('/') == std::string::npos)
				{
					domain += " (" + ip + ")";
				}
			}
			if (listed.count(domain) == 0)
			{
				element["columns"][0]["value"] = domain;
				mWhitelistSLC->addElement(element, ADD_BOTTOM);
			}
		}
		element["columns"][0]["column"] = "blacklist_col";
		for (it = LLViewerParcelMedia::sDeniedMedia.begin(); it != LLViewerParcelMedia::sDeniedMedia.end(); it++)
		{
			domain = *it;
			if (mShowIPs)
			{
				host.setHostByName(domain);
				ip = host.getIPString();
				if (ip != domain && domain.find('/') == std::string::npos)
				{
					domain += " (" + ip + ")";
				}
			}
			if (listed.count(domain) == 0)
			{
				element["columns"][0]["value"] = domain;
				mBlacklistSLC->addElement(element, ADD_BOTTOM);
			}
		}
		mWhitelistSLC->setScrollPos(whitescrollpos);
		mBlacklistSLC->setScrollPos(blackscrollpos);

		if (!gSavedSettings.getBOOL("MediaEnableFilter"))
		{
			childDisable("clear_lists");
			childDisable("show_ips");
			childDisable("blacklist_list");
			childDisable("whitelist_list");
			childDisable("remove_whitelist");
			childDisable("add_whitelist");
			childDisable("remove_blacklist");
			childDisable("add_blacklist");
			childDisable("match_ip");
			childDisable("input_domain");
			childDisable("commit_domain");
			childSetText("add_text", std::string("****** WARNING: media filtering is currently DISABLED ******"));
		}

		mIsDirty = false;
		mShowIPs = false;
	}

	LLFloater::draw();
}

void SLFloaterMediaFilter::setDirty()
{
	mIsDirty = true;
}

void SLFloaterMediaFilter::onShowIPs()
{
	mShowIPs = true;
	mIsDirty = true;
}

void SLFloaterMediaFilter::onWhitelistAdd()
{
	childDisable("clear_lists");
	childDisable("show_ips");
	childDisable("blacklist_list");
	childDisable("whitelist_list");
	childDisable("remove_whitelist");
	childDisable("add_whitelist");
	childDisable("remove_blacklist");
	childDisable("add_blacklist");
	childEnable("input_domain");
	childEnable("commit_domain");
	childSetText("add_text", std::string("Enter the domain/url to add to the white list:"));
	mIsWhitelist = true;
}

void SLFloaterMediaFilter::onWhitelistRemove()
{
	LLScrollListItem* selected = mWhitelistSLC->getFirstSelected();

	if (selected)
	{
		std::string domain = mWhitelistSLC->getSelectedItemLabel();
		size_t pos = domain.find(' ');
		if (pos != std::string::npos)
		{
			domain = domain.substr(0, pos);
		}

		LLViewerParcelMedia::sAllowedMedia.erase(domain);

		for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
		{
			if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == domain)
			{
				LLViewerParcelMedia::sMediaFilterList.erase(i);
				break;
			}
		}

		if (childGetValue("match_ip") && domain.find('/') == std::string::npos)
		{
			LLHost host;
			host.setHostByName(domain);
			std::string ip = host.getIPString();

			if (ip != domain)
			{
				LLViewerParcelMedia::sAllowedMedia.erase(ip);

				for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
				{
					if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == ip)
					{
						LLViewerParcelMedia::sMediaFilterList.erase(i);
						break;
					}
				}
			}
		}

		LLViewerParcelMedia::saveDomainFilterList();
		setDirty();
	}
}

void SLFloaterMediaFilter::onBlacklistAdd()
{
	childDisable("clear_lists");
	childDisable("show_ips");
	childDisable("blacklist_list");
	childDisable("whitelist_list");
	childDisable("remove_whitelist");
	childDisable("add_whitelist");
	childDisable("remove_blacklist");
	childDisable("add_blacklist");
	childEnable("input_domain");
	childEnable("commit_domain");
	childSetText("add_text", std::string("Enter the domain/url to add to the black list:"));
	mIsWhitelist = false;
}

void SLFloaterMediaFilter::onBlacklistRemove()
{	
	LLScrollListItem* selected = mBlacklistSLC->getFirstSelected();

	if (selected)
	{
		std::string domain = mBlacklistSLC->getSelectedItemLabel();
		size_t pos = domain.find(' ');
		if (pos != std::string::npos)
		{
			domain = domain.substr(0, pos);
		}

		LLViewerParcelMedia::sDeniedMedia.erase(domain);

		for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
		{
			if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == domain)
			{
				LLViewerParcelMedia::sMediaFilterList.erase(i);
				break;
			}
		}

		if (childGetValue("match_ip") && domain.find('/') == std::string::npos)
		{
			LLHost host;
			host.setHostByName(domain);
			std::string ip = host.getIPString();

			if (ip != domain)
			{
				LLViewerParcelMedia::sDeniedMedia.erase(ip);

				for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
				{
					if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == ip)
					{
						LLViewerParcelMedia::sMediaFilterList.erase(i);
						break;
					}
				}
			}
		}

		LLViewerParcelMedia::saveDomainFilterList();
		setDirty();
	}
}	

void SLFloaterMediaFilter::onCommitDomain()
{
	std::string domain = childGetText("input_domain");
	domain = LLViewerParcelMedia::extractDomain(domain);
	LLHost host;
	host.setHostByName(domain);
	std::string ip = host.getIPString();
	bool match_ip = (childGetValue("match_ip") && ip != domain && domain.find('/') == std::string::npos);

	if (!domain.empty())
	{
		LLViewerParcelMedia::sDeniedMedia.erase(domain);
		LLViewerParcelMedia::sAllowedMedia.erase(domain);
		for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
		{
			if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == domain)
			{
				LLViewerParcelMedia::sMediaFilterList.erase(i);
			}
		}
		if (match_ip)
		{
			LLViewerParcelMedia::sDeniedMedia.erase(ip);
			LLViewerParcelMedia::sAllowedMedia.erase(ip);
			for (S32 i = 0; i < (S32)LLViewerParcelMedia::sMediaFilterList.size(); i++)
			{
				if (LLViewerParcelMedia::sMediaFilterList[i]["domain"].asString() == ip)
				{
					LLViewerParcelMedia::sMediaFilterList.erase(i);
				}
			}
		}
		LLSD newmedia;
		newmedia["domain"] = domain;
		if (mIsWhitelist)
		{
			newmedia["action"] = "allow";
		}
		else
		{
			newmedia["action"] = "deny";
		}
		LLViewerParcelMedia::sMediaFilterList.append(newmedia);
		if (match_ip)
		{
			newmedia["domain"] = ip;
			LLViewerParcelMedia::sMediaFilterList.append(newmedia);
		}
		LLViewerParcelMedia::saveDomainFilterList();
	}

	childEnable("clear_lists");
	childEnable("show_ips");
	childEnable("blacklist_list");
	childEnable("whitelist_list");
	childEnable("remove_whitelist");
	childEnable("add_whitelist");
	childEnable("remove_blacklist");
	childEnable("add_blacklist");
	childDisable("input_domain");
	childDisable("commit_domain");
	childSetText("add_text", std::string("New domain:"));
	childSetText("input_domain", std::string(""));
	setDirty();
}
