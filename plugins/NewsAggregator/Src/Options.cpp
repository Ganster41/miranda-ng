/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "common.h"

INT_PTR CALLBACK DlgProcAddFeedOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SetWindowText(hwndDlg, TranslateT("Add Feed"));
		SetDlgItemText(hwndDlg, IDC_FEEDURL, _T("http://"));
		SetDlgItemText(hwndDlg, IDC_TAGSEDIT, _T(TAGSDEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_CHECKTIME, EM_LIMITTEXT, 3, 0);
		SetDlgItemInt(hwndDlg, IDC_CHECKTIME, DEFAULT_UPDATE_TIME, TRUE);
		SendDlgItemMessage(hwndDlg, IDC_TIMEOUT_VALUE_SPIN, UDM_SETRANGE32, 0, 999);	
		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE, "AddDlg");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				TCHAR str[MAX_PATH];
				char passw[MAX_PATH];
				if (!GetDlgItemText(hwndDlg, IDC_FEEDTITLE, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter Feed name"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_FEEDURL, str, SIZEOF(str)) || lstrcmp(str, _T("http://")) == 0) {
					MessageBox(hwndDlg, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
					break;
				}
				if (GetDlgItemInt(hwndDlg, IDC_CHECKTIME, false, false) < 0) {
					MessageBox(hwndDlg, TranslateT("Enter checking interval"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_TAGSEDIT, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter message format"), TranslateT("Error"), MB_OK);
					break;
				}
				
				MCONTACT hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
				CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)MODULE);
				GetDlgItemText(hwndDlg, IDC_FEEDTITLE, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "Nick", str);
				HWND hwndList = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				GetDlgItemText(hwndDlg, IDC_FEEDURL, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "URL", str);
				db_set_b(hContact, MODULE, "CheckState", 1);
				db_set_dw(hContact, MODULE, "UpdateTime", GetDlgItemInt(hwndDlg, IDC_CHECKTIME, false, false));
				GetDlgItemText(hwndDlg, IDC_TAGSEDIT, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "MsgFormat", str);
				db_set_w(hContact, MODULE, "Status", CallProtoService(MODULE, PS_GETSTATUS, 0, 0));
				if (IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
					db_set_b(hContact, MODULE, "UseAuth", 1);
					GetDlgItemText(hwndDlg, IDC_LOGIN, str, SIZEOF(str));
					db_set_ts(hContact, MODULE, "Login", str);
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, passw, SIZEOF(passw));
					db_set_s(hContact, MODULE, "Password", passw);
				}
				DeleteAllItems(hwndList);
				UpdateList(hwndList);
			}

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_USEAUTH:
			if (IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE);
			}
			break;

		case IDC_TAGHELP:
			{
				TCHAR tszTagHelp[1024];
				mir_sntprintf(tszTagHelp, SIZEOF(tszTagHelp), _T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s"),
					_T("#<title>#"),		TranslateT("The title of the item."),
					_T("#<description>#"),	TranslateT("The item synopsis."),
					_T("#<link>#"),			TranslateT("The URL of the item."),
					_T("#<author>#"),		TranslateT("Email address of the author of the item."),
					_T("#<comments>#"),		TranslateT("URL of a page for comments relating to the item."),
					_T("#<guid>#"),			TranslateT("A string that uniquely identifies the item."),
					_T("#<category>#"),		TranslateT("Specify one or more categories that the item belongs to.")
					);
				MessageBox(hwndDlg, tszTagHelp, TranslateT("Feed Tag Help"), MB_OK);
			}
			break;

		case IDC_RESET:
			if (MessageBox(hwndDlg, TranslateT("Are you sure?"), TranslateT("Tags Mask Reset"), MB_YESNO | MB_ICONWARNING) == IDYES)
				SetDlgItemText(hwndDlg, IDC_TAGSEDIT, _T(TAGSDEFAULT));
			break;

		case IDC_DISCOVERY:
			EnableWindow(GetDlgItem(hwndDlg, IDC_DISCOVERY), FALSE);
			SetDlgItemText(hwndDlg, IDC_DISCOVERY, TranslateT("Wait..."));
			TCHAR tszURL[MAX_PATH] = {0}, *tszTitle = NULL;
			if (GetDlgItemText(hwndDlg, IDC_FEEDURL, tszURL, SIZEOF(tszURL)) || lstrcmp(tszURL, _T("http://")) != 0)
				tszTitle = CheckFeed(tszURL, hwndDlg);
			else
				MessageBox(hwndDlg, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
			SetDlgItemText(hwndDlg, IDC_FEEDTITLE, tszTitle);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DISCOVERY), TRUE);
			SetDlgItemText(hwndDlg, IDC_DISCOVERY, TranslateT("Check Feed"));
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "AddDlg");
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcChangeFeedOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			ItemInfo &SelItem = *(ItemInfo*)lParam;
			ItemInfo *nSelItem = new ItemInfo(SelItem);
			SetWindowText(hwndDlg, TranslateT("Change Feed"));
			SendDlgItemMessage(hwndDlg, IDC_CHECKTIME, EM_LIMITTEXT, 3, 0);
			SendDlgItemMessage(hwndDlg, IDC_TIMEOUT_VALUE_SPIN, UDM_SETRANGE32, 0, 999);

			MCONTACT hContact;
			for (hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
				ptrT dbNick( db_get_tsa(hContact, MODULE, "Nick"));
				if (dbNick == NULL)
					continue;

				if (lstrcmp(dbNick, SelItem.nick) != 0)
					continue;

				ptrT dbURL( db_get_tsa(hContact, MODULE, "URL"));
				if (dbURL == NULL)
					continue;

				if (lstrcmp(dbURL, SelItem.url) != 0)
					continue;

				nSelItem->hContact = hContact;
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)nSelItem);
				SetDlgItemText(hwndDlg, IDC_FEEDURL, SelItem.url);
				SetDlgItemText(hwndDlg, IDC_FEEDTITLE, SelItem.nick);
				SetDlgItemInt(hwndDlg, IDC_CHECKTIME, db_get_dw(hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME), TRUE);
				
				DBVARIANT dbMsg = {0};
				if (!db_get_ts(hContact, MODULE, "MsgFormat", &dbMsg)) {
					SetDlgItemText(hwndDlg, IDC_TAGSEDIT, dbMsg.ptszVal);
					db_free(&dbMsg);
				}
				if (db_get_b(hContact, MODULE, "UseAuth", 0)) {
					CheckDlgButton(hwndDlg, IDC_USEAUTH, BST_CHECKED);
					EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), TRUE);
					DBVARIANT dbLogin = {0};
					if (!db_get_ts(hContact, MODULE, "Login", &dbLogin)) {
						SetDlgItemText(hwndDlg, IDC_LOGIN, dbLogin.ptszVal);
						db_free(&dbLogin);
					}
					ptrA pwd(db_get_sa(hContact, MODULE, "Password"));
					SetDlgItemTextA(hwndDlg, IDC_PASSWORD, pwd);
				}
				break;
			}
			WindowList_Add(hChangeFeedDlgList, hwndDlg, hContact);
			Utils_RestoreWindowPositionNoSize(hwndDlg, hContact, MODULE, "ChangeDlg");
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				ItemInfo *SelItem = (ItemInfo*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				TCHAR str[MAX_PATH];
				char passw[MAX_PATH];
				if (!GetDlgItemText(hwndDlg, IDC_FEEDTITLE, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter Feed name"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_FEEDURL, str, SIZEOF(str)) || lstrcmp(str, _T("http://")) == 0) {
					MessageBox(hwndDlg, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
					break;
				}
				if (GetDlgItemInt(hwndDlg, IDC_CHECKTIME, false, false) < 0) {
					MessageBox(hwndDlg, TranslateT("Enter checking interval"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_TAGSEDIT, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter message format"), TranslateT("Error"), MB_OK);
					break;
				}

				GetDlgItemText(hwndDlg, IDC_FEEDURL, str, SIZEOF(str));
				db_set_ts(SelItem->hContact, MODULE, "URL", str);
				GetDlgItemText(hwndDlg, IDC_FEEDTITLE, str, SIZEOF(str));
				db_set_ts(SelItem->hContact, MODULE, "Nick", str);
				db_set_dw(SelItem->hContact, MODULE, "UpdateTime", GetDlgItemInt(hwndDlg, IDC_CHECKTIME, false, false));
				GetDlgItemText(hwndDlg, IDC_TAGSEDIT, str, SIZEOF(str));
				db_set_ts(SelItem->hContact, MODULE, "MsgFormat", str);
				if (IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
					db_set_b(SelItem->hContact, MODULE, "UseAuth", 1);

					GetDlgItemText(hwndDlg, IDC_LOGIN, str, SIZEOF(str));
					db_set_ts(SelItem->hContact, MODULE, "Login", str);

					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, passw, SIZEOF(passw));
					db_set_s(SelItem->hContact, MODULE, "Password", passw);
				}
				else
				{
					db_unset(SelItem->hContact, MODULE, "UseAuth");
					db_unset(SelItem->hContact, MODULE, "Login");
					db_unset(SelItem->hContact, MODULE, "Password");
				}
				DeleteAllItems(SelItem->hwndList);
				UpdateList(SelItem->hwndList);
			}

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_USEAUTH:
			if (IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE);
			}
			break;

		case IDC_TAGHELP:
			{
				TCHAR tszTagHelp[1024];
				mir_sntprintf(tszTagHelp, SIZEOF(tszTagHelp), _T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s"),
					_T("#<title>#"),		TranslateT("The title of the item."),
					_T("#<description>#"),	TranslateT("The item synopsis."),
					_T("#<link>#"),			TranslateT("The URL of the item."),
					_T("#<author>#"),		TranslateT("Email address of the author of the item."),
					_T("#<comments>#"),		TranslateT("URL of a page for comments relating to the item."),
					_T("#<guid>#"),			TranslateT("A string that uniquely identifies the item."),
					_T("#<category>#"),		TranslateT("Specify one or more categories that the item belongs to.")
					);
				MessageBox(hwndDlg, tszTagHelp, TranslateT("Feed Tag Help"), MB_OK);
			}
			break;

		case IDC_RESET:
			if (MessageBox(hwndDlg, TranslateT("Are you sure?"), TranslateT("Tags Mask Reset"), MB_YESNO | MB_ICONWARNING) == IDYES)
				SetDlgItemText(hwndDlg, IDC_TAGSEDIT, _T(TAGSDEFAULT));
			break;

		case IDC_DISCOVERY:
			TCHAR tszURL[MAX_PATH] = {0};
			if (GetDlgItemText(hwndDlg, IDC_FEEDURL, tszURL, SIZEOF(tszURL)) || lstrcmp(tszURL, _T("http://")) != 0) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_DISCOVERY), FALSE);
				SetDlgItemText(hwndDlg, IDC_DISCOVERY, TranslateT("Wait..."));
				TCHAR *tszTitle = CheckFeed(tszURL, hwndDlg);
				SetDlgItemText(hwndDlg, IDC_FEEDTITLE, tszTitle);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DISCOVERY), TRUE);
				SetDlgItemText(hwndDlg, IDC_DISCOVERY, TranslateT("Check Feed"));
			}
			else MessageBox(hwndDlg, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		Utils_SaveWindowPosition(hwndDlg, hContact, MODULE, "ChangeDlg");
		WindowList_Remove(hChangeFeedDlgList, hwndDlg);
		ItemInfo *SelItem = (ItemInfo *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		delete SelItem;
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcChangeFeedMenu(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SetWindowText(hwndDlg, TranslateT("Change Feed"));
			SendDlgItemMessage(hwndDlg, IDC_CHECKTIME, UDM_SETRANGE32, 0, 999);

			MCONTACT hContact = lParam;
			WindowList_Add(hChangeFeedDlgList, hwndDlg, hContact);
			Utils_RestoreWindowPositionNoSize(hwndDlg, hContact, MODULE, "ChangeDlg");
			DBVARIANT dbv;
			if (!db_get_ts(hContact, MODULE, "Nick", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_FEEDTITLE, dbv.ptszVal);
				db_free(&dbv);
			}
			if (!db_get_ts(hContact, MODULE, "URL", &dbv)) {
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
				SetDlgItemText(hwndDlg, IDC_FEEDURL, dbv.ptszVal);
				db_free(&dbv);
			}
			SetDlgItemInt(hwndDlg, IDC_CHECKTIME, db_get_dw(hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME), TRUE);
			if (!db_get_ts(hContact, MODULE, "MsgFormat", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_TAGSEDIT, dbv.ptszVal);
				db_free(&dbv);
			}
			if (db_get_b(hContact, MODULE, "UseAuth", 0)) {
				CheckDlgButton(hwndDlg, IDC_USEAUTH, BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), TRUE);
				if (!db_get_ts(hContact, MODULE, "Login", &dbv)) {
					SetDlgItemText(hwndDlg, IDC_LOGIN, dbv.ptszVal);
					db_free(&dbv);
				}
				ptrA pwd(db_get_sa(hContact, MODULE, "Password"));
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, pwd);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				TCHAR str[MAX_PATH];
				char passw[MAX_PATH];
				if (!GetDlgItemText(hwndDlg, IDC_FEEDTITLE, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter Feed name"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_FEEDURL, str, SIZEOF(str)) || lstrcmp(str, _T("http://")) == 0) {
					MessageBox(hwndDlg, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
					break;
				}
				if (GetDlgItemInt(hwndDlg, IDC_CHECKTIME, false, false) < 0) {
					MessageBox(hwndDlg, TranslateT("Enter checking interval"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_TAGSEDIT, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter message format"), TranslateT("Error"), MB_OK);
					break;
				}

				GetDlgItemText(hwndDlg, IDC_FEEDURL, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "URL", str);
				GetDlgItemText(hwndDlg, IDC_FEEDTITLE, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "Nick", str);
				db_set_dw(hContact, MODULE, "UpdateTime", GetDlgItemInt(hwndDlg, IDC_CHECKTIME, false, false));
				GetDlgItemText(hwndDlg, IDC_TAGSEDIT, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "MsgFormat", str);
				if (IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
					db_set_b(hContact, MODULE, "UseAuth", 1);

					GetDlgItemText(hwndDlg, IDC_LOGIN, str, SIZEOF(str));
					db_set_ts(hContact, MODULE, "Login", str);

					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, passw, SIZEOF(passw));
					db_set_s(hContact, MODULE, "Password", passw);
				}
				else
				{
					db_unset(hContact, MODULE, "UseAuth");
					db_unset(hContact, MODULE, "Login");
					db_unset(hContact, MODULE, "Password");
				}
			}

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_USEAUTH:
			if (IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE);
			}
			break;

		case IDC_TAGHELP:
			{
				TCHAR tszTagHelp[1024];
				mir_sntprintf(tszTagHelp, SIZEOF(tszTagHelp), _T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s"),
					_T("#<title>#"),		TranslateT("The title of the item."),
					_T("#<description>#"),	TranslateT("The item synopsis."),
					_T("#<link>#"),			TranslateT("The URL of the item."),
					_T("#<author>#"),		TranslateT("Email address of the author of the item."),
					_T("#<comments>#"),		TranslateT("URL of a page for comments relating to the item."),
					_T("#<guid>#"),			TranslateT("A string that uniquely identifies the item."),
					_T("#<category>#"),		TranslateT("Specify one or more categories that the item belongs to.")
					);
				MessageBox(hwndDlg, tszTagHelp, TranslateT("Feed Tag Help"), MB_OK);
			}
			break;

		case IDC_RESET:
			if (MessageBox(hwndDlg, TranslateT("Are you sure?"), TranslateT("Tags Mask Reset"), MB_YESNO | MB_ICONWARNING) == IDYES)
				SetDlgItemText(hwndDlg, IDC_TAGSEDIT, _T(TAGSDEFAULT));
			break;

		case IDC_DISCOVERY:
			TCHAR tszURL[MAX_PATH] = {0};
			if (GetDlgItemText(hwndDlg, IDC_FEEDURL, tszURL, SIZEOF(tszURL)) || lstrcmp(tszURL, _T("http://")) != 0) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_DISCOVERY), FALSE);
				SetDlgItemText(hwndDlg, IDC_DISCOVERY, TranslateT("Wait..."));
				TCHAR *tszTitle = CheckFeed(tszURL, hwndDlg);
				SetDlgItemText(hwndDlg, IDC_FEEDTITLE, tszTitle);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DISCOVERY), TRUE);
				SetDlgItemText(hwndDlg, IDC_DISCOVERY, TranslateT("Check Feed"));
			}
			else MessageBox(hwndDlg, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		Utils_SaveWindowPosition(hwndDlg, hContact, MODULE, "ChangeDlg");
		WindowList_Remove(hChangeFeedDlgList, hwndDlg);
	}

	return FALSE;
}

INT_PTR CALLBACK UpdateNotifyOptsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_FEEDLIST);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		CreateList(hwndList);
		UpdateList(hwndList);
		CheckDlgButton(hwndDlg, IDC_STARTUPRETRIEVE, db_get_b(NULL, MODULE, "StartupRetrieve", 1));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADD:
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDFEED), hwndDlg, DlgProcAddFeedOpts, (LPARAM)hwndList);
			return FALSE;

		case IDC_CHANGE:
			{
				ItemInfo SelItem = {0};
				int sel = ListView_GetSelectionMark(hwndList);
				ListView_GetItemText(hwndList, sel, 0, SelItem.nick, MAX_PATH);
				ListView_GetItemText(hwndList, sel, 1, SelItem.url, MAX_PATH);
				SelItem.hwndList = hwndList;
				SelItem.SelNumber = sel;
				CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDFEED), hwndDlg, DlgProcChangeFeedOpts, (LPARAM)&SelItem);
			}
			return FALSE;

		case IDC_REMOVE:
			if (MessageBox(hwndDlg, TranslateT("Are you sure?"), TranslateT("Contact deleting"), MB_YESNO | MB_ICONWARNING) == IDYES) {
				TCHAR nick[MAX_PATH], url[MAX_PATH];
				int sel = ListView_GetSelectionMark(hwndList);
				ListView_GetItemText(hwndList, sel, 0, nick, MAX_PATH);
				ListView_GetItemText(hwndList, sel, 1, url, MAX_PATH);

				for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
					ptrT dbNick( db_get_tsa(hContact, MODULE, "Nick"));
					if (dbNick == NULL)
						break;
					if ( lstrcmp(dbNick, nick))
						continue;
						
					ptrT dbURL( db_get_tsa(hContact, MODULE, "URL"));
					if (dbURL == NULL)
						break;
					if ( lstrcmp(dbURL, url))
						continue;

					CallService(MS_DB_CONTACT_DELETE, hContact, 0);
					ListView_DeleteItem(hwndList, sel);
					break;
				}
			}
			return FALSE;

		case IDC_IMPORT:
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FEEDIMPORT), hwndDlg, DlgProcImportOpts, (LPARAM)hwndList);
			return FALSE;

		case IDC_EXPORT:
			CreateDialog(hInst, MAKEINTRESOURCE(IDD_FEEDEXPORT), hwndDlg, DlgProcExportOpts);
			return FALSE;

		case IDC_STARTUPRETRIEVE:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			switch (hdr->code) {
			case PSN_APPLY:
				{
					db_set_b(NULL, MODULE, "StartupRetrieve", IsDlgButtonChecked(hwndDlg, IDC_STARTUPRETRIEVE));
					int i = 0;
					for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
						db_set_b(hContact, MODULE, "CheckState", ListView_GetCheckState(hwndList, i));
						if (!ListView_GetCheckState(hwndList, i))
							db_set_b(hContact, "CList", "Hidden", 1);
						else
							db_unset(hContact,"CList","Hidden");
						i += 1;
					}
				}
				break;

			case NM_DBLCLK:
				{
					ItemInfo SelItem = {0};
					int sel = ListView_GetHotItem(hwndList);
					if (sel != -1) {
						ListView_GetItemText(hwndList, sel, 0, SelItem.nick, MAX_PATH);
						ListView_GetItemText(hwndList, sel, 1, SelItem.url, MAX_PATH);
						SelItem.hwndList = hwndList;
						SelItem.SelNumber = sel;
						CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDFEED), hwndDlg, DlgProcChangeFeedOpts, (LPARAM)&SelItem);
					}
					break;
				}

			case LVN_ITEMCHANGED:
				{
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
					if (((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) && !UpdateListFlag)
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				}
			}
		}
	}//end* switch (msg)
	return FALSE;
}

INT OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszGroup = LPGEN("Network");
	odp.pszTitle = LPGEN("News Aggregator");
	odp.pfnDlgProc = UpdateNotifyOptsProc;
	Options_AddPage(wParam, &odp);
	return 0;
}
