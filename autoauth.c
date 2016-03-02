/*
 * AutoAuthorization - Auto-handling of request of remote user who wants to appear in your buddy list
 * Copyright (C) 2012
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 */


#define PURPLE_PLUGINS
#define PLUGIN_ID	"core-autoauth"
#define PLUGIN_NAME 	"AutoAuthorization"
#define PLUGIN_SUMMARY "Automatic handling of request of remote user who wants to appear in your buddy list"
#define PLUGIN_DESCRIPTION "To choose the handling see configuration menu of the plugin"
#define PLUGIN_AUTHOR "Bruneau Florian <@gmail.com>"
#define PLUGIN_WEBSITE "http://code.google.com/p/pidgin-autoauthorization-plugin/"
#define PLUGIN_VERSION "1.0"

//GLibs and SDL
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>

//libpurple
#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "account.h"
#include "debug.h"
#include "internal.h"

typedef enum
{
	FT_ACCEPT,
	FT_REJECT,
	FT_IGNORE
} AutoAuthSetting;


// Callback function called when "account-authorization-requested" is catched
static int autoauth(PurpleAccount *account,const char * remote_user)
{

	char info[100];
	//Parameter the user choosed
	int Preference = purple_prefs_get_int("/plugins/core/autoauth/auto");

	int ToReturn=0;

	//Actually contact isn't necessary
	//PurpleContact *contact = purple_contact_new();

	PurpleBuddy * buddy;

	PurpleGroup*group;

	if (purple_prefs_get_bool("/plugins/core/autoauth/group"))
	{
	    //The user choosed to put automatically addded in a part-group
		group=purple_group_new("Automatically Added");
		purple_debug_info("autoauth", "Creating/Opening group 'Automatically Added' \n");
	}
	else
	{
	    //Automatically added will be added in Buddies
		group=purple_group_new(_("Buddies"));
		purple_debug_info("autoauth", "Creating/Opening group 'Buddies' \n");
	}
    //These are controls in debugging
	if (group==NULL) purple_debug_info("autoauth", "Error group=null\n");

	if (account==NULL) purple_debug_info("autoauth", "Error account=null\n");

	purple_blist_add_group(group,NULL);



	switch (Preference)
	{
		//If he choosed to accept all requests
		case FT_ACCEPT:
				sprintf(info,"%s %s",remote_user,_(" had been added to your buddy list"));
                //There is no alias for now but whynot in the future
				buddy=purple_buddy_new(account,remote_user,NULL);

				purple_debug_info("autoauth", "Buddy Created for %s with this account %s (%s)\n",buddy->name,account->username,			 															purple_account_get_protocol_name(buddy->account));

				//contact = purple_buddy_get_contact(buddy);

				//purple_blist_add_contact(contact, group,(PurpleBlistNode*)group);


				if (group!=NULL)
				{
					purple_blist_add_buddy(buddy,NULL,group,(PurpleBlistNode*)group);
					purple_debug_info("autoauth", "Buddy (%s) added to blist in group (%s)\n",buddy->name,group->name);
				}


				purple_account_add_buddy(account,buddy);

				purple_debug_info("autoauth", "%s has been added to your buddy list",buddy->name);



				//-------------------------------------------------------------------


				ToReturn=1;//PURPLE_ACCOUNT_RESPONSE_ACCEPT;//1
				break;
		//If he choosed to reject all requests
		case FT_REJECT:
				sprintf(info,"%s %s",remote_user,_(" adding request had been refused "));
				ToReturn=-1;//PURPLE_ACCOUNT_RESPONSE_DENY;//-1
				break;
		//If he choosed to reject all requests
		case FT_IGNORE:
				sprintf(info,"%s %s",remote_user,_(" adding request had been ignored "));
				ToReturn=0;//PURPLE_ACCOUNT_RESPONSE_IGNORE;//0
				break;

	}

	//If he also choosed to be notified of plugin actions
	if(remote_user!=NULL)
		{
		if (purple_prefs_get_bool("/plugins/core/autoauth/notify"))
			{
				purple_notify_message(NULL, PURPLE_NOTIFY_MSG_INFO, "Information",info, NULL, NULL, NULL);
			}
		}

	return ToReturn;
}



static gboolean
plugin_load(PurplePlugin *plugin) {


	static int handle;

	purple_signal_connect(purple_accounts_get_handle(), "account-authorization-requested", &handle,
				PURPLE_CALLBACK(autoauth),plugin);

	purple_signal_connect(purple_accounts_get_handle(), "account-authorization-requested-with-message", &handle,
				PURPLE_CALLBACK(autoauth),plugin);


    return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
	return TRUE;
}

static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin)
{

	PurplePluginPrefFrame *frame;
	PurplePluginPref *pref;

	frame = purple_plugin_pref_frame_new();

	pref = purple_plugin_pref_new_with_name_and_label(_("/plugins/core/autoauth/auto"),
					_("Automatic action to do when someone adds you"));
	purple_plugin_pref_set_type(pref, PURPLE_PLUGIN_PREF_CHOICE);
	purple_plugin_pref_add_choice(pref, _("Auto Accept"), GINT_TO_POINTER(FT_ACCEPT));
	purple_plugin_pref_add_choice(pref, _("Auto Reject"), GINT_TO_POINTER(FT_REJECT));
	purple_plugin_pref_add_choice(pref, _("Auto Ignore"), GINT_TO_POINTER(FT_IGNORE));
	purple_plugin_pref_frame_add(frame, pref);

	pref = purple_plugin_pref_new_with_name_and_label(
			"/plugins/core/autoauth/notify", _("Notify when someone is added or denied or ignored"));
	purple_plugin_pref_frame_add(frame, pref);

	pref = purple_plugin_pref_new_with_name_and_label(
			"/plugins/core/autoauth/group", _("Add acceted buddies to 'Automatically Added' group (created if necessary)"));
	purple_plugin_pref_frame_add(frame, pref);

	return frame;
}

static PurplePluginUiInfo prefs_info = {
	get_plugin_pref_frame,
	0,
	NULL,


	NULL,
	NULL,
	NULL,
	NULL
};


static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    PLUGIN_ID,
    PLUGIN_NAME,
    PLUGIN_VERSION,

    PLUGIN_SUMMARY,
    PLUGIN_DESCRIPTION,
    PLUGIN_AUTHOR,
    PLUGIN_WEBSITE,

    plugin_load,
    plugin_unload,
    NULL,

    NULL,
    NULL,
    &prefs_info,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};





static void
init_plugin(PurplePlugin *plugin)
{
	purple_prefs_add_none("/plugins/core/autoauth");
	purple_prefs_add_int("/plugins/core/autoauth/auto", FT_ACCEPT);
	purple_prefs_add_bool("/plugins/core/autoauth/notify",FALSE);
	purple_prefs_add_bool("/plugins/core/autoauth/group",FALSE);


}

PURPLE_INIT_PLUGIN(autoauth, init_plugin, info)



