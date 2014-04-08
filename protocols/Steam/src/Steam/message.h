﻿#ifndef _STEAM_MESSAGE_H_
#define _STEAM_MESSAGE_H_

namespace SteamWebApi
{
	class MessageApi : public BaseApi
	{
	public:
		class SendResult : public Result
		{
			friend MessageApi;

		private:
			DWORD timestamp;

		public:
			SendResult() : timestamp(0) { }

			const DWORD GetTimestamp() const { return timestamp; }
		};

		static void SendStatus(HANDLE hConnection, const char *token, const char *sessionId, int state, SendResult *sendResult)
		{
			sendResult->success = false;

			CMStringA data;
			data.AppendFormat("access_token=%s", token);
			data.AppendFormat("&umqid=%s", sessionId);
			data.AppendFormat("&persona_state=%u", state);
			data.Append("&type=personastate");

			HttpRequest request(hConnection, REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;

			sendResult->success = true;
		}

		static void SendMessage(HANDLE hConnection, const char *token, const char *sessionId, const char *steamId, const char *text, SendResult *sendResult)
		{
			sendResult->success = false;

			CMStringA data;
			data.AppendFormat("access_token=%s", token);
			data.AppendFormat("&umqid=%s", sessionId);
			data.AppendFormat("&steamid_dst=%s", steamId);
			data.Append("&type=saytext");
			data.AppendFormat("&text=%s", ptrA(mir_urlEncode(text)));

			HttpRequest request(hConnection, REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node;

			node = json_get(root, "error");
			ptrW error(json_as_string(node));

			if (lstrcmp(error, L"OK"))
				return;

			node = json_get(root, "utc_timestamp");
			sendResult->timestamp = atol(ptrA(mir_u2a(json_as_string(node))));

			sendResult->success = true;
		}

		static void SendTyping(HANDLE hConnection, const char *token, const char *sessionId, const char *steamId, SendResult *sendResult)
		{
			sendResult->success = false;

			CMStringA data;
			data.AppendFormat("access_token=%s", token);
			data.AppendFormat("&umqid=%s", sessionId);
			data.AppendFormat("&steamid_dst=%s", steamId);
			data.Append("&type=typing");

			HttpRequest request(hConnection, REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;

			sendResult->success = true;
		}
	};
}


#endif //_STEAM_MESSAGE_H_