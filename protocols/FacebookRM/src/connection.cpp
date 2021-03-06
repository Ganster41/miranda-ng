/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-13 Robert P�sel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"

void FacebookProto::ChangeStatus(void*)
{
	ScopedLock s(signon_lock_);
	ScopedLock b(facy.buddies_lock_);
	
	int new_status = m_iDesiredStatus;
	int old_status = m_iStatus;

	if (new_status == ID_STATUS_OFFLINE)
	{ // Logout	
		debugLogA("##### Beginning SignOff process");

		m_iStatus = facy.self_.status_id = ID_STATUS_OFFLINE;
		SetEvent(update_loop_lock_);
		Netlib_Shutdown(facy.hMsgCon);

		OnLeaveChat(NULL, NULL);
		SetAllContactStatuses(ID_STATUS_OFFLINE);
		ToggleStatusMenuItems(false);
		delSetting("LogonTS");

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		if (getByte(FACEBOOK_KEY_DISCONNECT_CHAT, DEFAULT_DISCONNECT_CHAT))
			facy.chat_state(false);

		facy.logout();

		facy.clear_cookies();
		facy.clear_notifications();
		facy.buddies.clear();
		facy.messages_ignore.clear();
		facy.pages.clear();
		facy.typers.clear();
		facy.readers.clear();

		if (facy.hMsgCon)
			Netlib_CloseHandle(facy.hMsgCon);
		facy.hMsgCon = NULL;

		debugLogA("##### SignOff complete");

		return;
	}
	else if (old_status == ID_STATUS_OFFLINE)
	{ // Login
		SYSTEMTIME t;
		GetLocalTime(&t);
		debugLogA("[%d.%d.%d] Using Facebook Protocol RM %s", t.wDay, t.wMonth, t.wYear, __VERSION_STRING_DOTS);
		
		debugLogA("***** Beginning SignOn process");

		m_iStatus = facy.self_.status_id = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		ResetEvent(update_loop_lock_);

		if (NegotiateConnection() && facy.home() && facy.reconnect())
		{		
			// Load all friends
			ProcessFriendList(NULL);

			// Process friendship requests
			ForkThread(&FacebookProto::ProcessFriendRequests, NULL);

			// Get unread messages
			ForkThread(&FacebookProto::ProcessUnreadMessages, NULL);

			// Get notifications
			ForkThread(&FacebookProto::ProcessNotifications, NULL);

			// Load pages for post status dialog
			ForkThread(&FacebookProto::ProcessPages, NULL);

			setDword("LogonTS", (DWORD)time(NULL));
			ForkThread(&FacebookProto::UpdateLoop,  NULL);
			ForkThread(&FacebookProto::MessageLoop, NULL);

			if (getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS))
				ForkThread(&FacebookProto::SetAwayMsgWorker, NULL);
		}
		else {
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)old_status, m_iStatus);

			if (facy.hFcbCon)
				Netlib_CloseHandle(facy.hFcbCon);
			facy.hFcbCon = NULL;

			facy.clear_cookies();

			// Set to offline
			m_iStatus = m_iDesiredStatus = facy.self_.status_id = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

			debugLogA("***** SignOn failed");

			return;
		}

		ToggleStatusMenuItems(true);
		debugLogA("***** SignOn complete");
	}

	m_invisible = (new_status == ID_STATUS_INVISIBLE);

	facy.chat_state(!m_invisible);

	ForkThread(&FacebookProto::ProcessBuddyList, NULL);

	m_iStatus = facy.self_.status_id = new_status;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

	debugLogA("***** ChangeStatus complete");
}

/** Return true on success, false on error. */
bool FacebookProto::NegotiateConnection()
{
	debugLogA("***** Negotiating connection with Facebook");

	DBVARIANT dbv = {0};

	ptrA username( getStringA(FACEBOOK_KEY_LOGIN));
	if (!username || !strlen(username)) {
		NotifyEvent(m_tszUserName,TranslateT("Please enter a username."),NULL,FACEBOOK_EVENT_CLIENT);
		return false;
	}

	ptrA password( getStringA(FACEBOOK_KEY_PASS));
	if (!password || !*password) {
		NotifyEvent(m_tszUserName,TranslateT("Please enter a password."),NULL,FACEBOOK_EVENT_CLIENT);
		return false;
	}

	password = mir_utf8encode(password);

	// Refresh last time of feeds update
	facy.last_feeds_update_ = ::time(NULL);

	// Get info about secured connection
	facy.https_ = getByte(FACEBOOK_KEY_FORCE_HTTPS, DEFAULT_FORCE_HTTPS) != 0;

	// Generate random clientid for this connection
	facy.chat_clientid_ = utils::text::rand_string(8, "0123456789abcdef");

	// Create default group for new contacts
	ptrT groupName( getTStringA(FACEBOOK_KEY_DEF_GROUP));
	if (groupName != NULL)
		Clist_CreateGroup(0, groupName);
	
	return facy.login(username, password);
}

void FacebookProto::UpdateLoop(void *)
{
	time_t tim = ::time(NULL);
	debugLogA(">>>>> Entering Facebook::UpdateLoop[%d]", tim);

	for (int i = -1; !isOffline(); i = ++i % 50)
	{
		if (i != -1) {
			ProcessBuddyList(NULL);

			if (getByte(FACEBOOK_KEY_EVENT_FEEDS_ENABLE, DEFAULT_EVENT_FEEDS_ENABLE))
				ProcessFeeds(NULL);
		}

		if (i == 49)
			ProcessFriendRequests(NULL);

		debugLogA("***** FacebookProto::UpdateLoop[%d] going to sleep...", tim);
		if (WaitForSingleObjectEx(update_loop_lock_, GetPollRate() * 1000, true) != WAIT_TIMEOUT)
			break;
		debugLogA("***** FacebookProto::UpdateLoop[%d] waking up...", tim);
	}

	ResetEvent(update_loop_lock_);
	debugLogA("<<<<< Exiting FacebookProto::UpdateLoop[%d]", tim);
}

void FacebookProto::MessageLoop(void *)
{
	time_t tim = ::time(NULL);
	debugLogA(">>>>> Entering Facebook::MessageLoop[%d]", tim);

	while (facy.channel())
	{
		if (isOffline())
			break;
		debugLogA("***** FacebookProto::MessageLoop[%d] refreshing...", tim);
	}

	debugLogA("<<<<< Exiting FacebookProto::MessageLoop[%d]", tim);
}

BYTE FacebookProto::GetPollRate()
{
	BYTE poll_rate = getByte(FACEBOOK_KEY_POLL_RATE, FACEBOOK_DEFAULT_POLL_RATE);

	return (
	    (poll_rate >= FACEBOOK_MINIMAL_POLL_RATE &&
	      poll_rate <= FACEBOOK_MAXIMAL_POLL_RATE)
	    ? poll_rate : FACEBOOK_DEFAULT_POLL_RATE);
}
