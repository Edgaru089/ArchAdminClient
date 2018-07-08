#pragma once

#include <atomic>
#include <thread>
#include <SFML/Window.hpp>
using namespace std;
using namespace sf;

#include "ImGui/imgui-setup.h"
#include "ArchAdminClient.hpp"
#include "LogSystem.hpp"


namespace Manager {

	ArchAdminClient client;
	atomic_bool connecting = false;
	atomic_bool connectionFailed = false;
	atomic_bool loggedIn = false;

	char ip[128], pass[128];
	int port;

	void runLoginGui() {
		imgui::SetNextWindowPos(imgui::GetIO().DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5f, 0.8f));
		imgui::Begin("Administrative Login", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

		imgui::InputText("Ip Address", ip, sizeof(ip));
		imgui::InputInt("Port", &port, 1, 100);
		imgui::InputText("Password", pass, sizeof(pass), ImGuiInputTextFlags_Password);

		if (imgui::Button("Connect") && !connecting) {
			connecting = true;
			thread([&]() {
				if (client.connect(IpAddress(ip), port, ArchAdminClient::getHashOfPassword(pass))) {
					if (connecting) {
						loggedIn = true;
						ip[0] = '\0'; pass[0] = '\0'; port = 0;
					}
					else
						client.disconnect();
				}
				else {
					if (connecting)
						connectionFailed = true;
				}
				connecting = false;
			}).detach();
		}

		imgui::End();

		if (connecting || connectionFailed) {
			imgui::SetNextWindowPos(imgui::GetIO().DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5, 1.0));
			imgui::OpenPopup("Connect");
			if (imgui::BeginPopupModal("Connect", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {

				if (connecting) {
					imgui::Text("Connecting...");
					if (imgui::Button("Cancel"))
						connecting = false;
				}
				else if (connectionFailed) {
					imgui::Text("Connection Failed.");
					if (imgui::Button("Close"))
						connectionFailed = false;
				}

				imgui::EndPopup();
			}
		}
	}


	vector<pair<string, pair<string, string>>> users;
	bool reloadPending = true;

	//bool firstRun = true;

	vector<FloatRect> rowBoundingRects;

	bool addingUser = false;
	char addUsername[128], addUserPasswordCleartext[128];

	bool changingUser = false;
	int changeId;
	char changePassClear[128];

	bool showSha256Window = false;
	char hashInput[256], hashOutput[65];

	//bool findWinOpen = false;
	char findContent[128];
	bool findCaseInsensitive = false;
	int curFindId = -1;
	bool newFound = true;
	bool showNotFoundWindow = false;

	bool showDemoWindow = false;

	void runManageGui() {

		//if (firstRun) {
		//	client.listUsers(users);

		//	firstRun = false;
		//}

		if (reloadPending) {
			client.listUsers(users);
			reloadPending = false;
		}

		if (addingUser) {
			imgui::SetNextWindowPos(imgui::GetIO().DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5, 1.0));
			imgui::OpenPopup("Add User");
			if (imgui::BeginPopupModal("Add User", &addingUser, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
				imgui::InputText("User Name", addUsername, sizeof(addUsername));
				imgui::InputText("Password(Clear Text)", addUserPasswordCleartext, sizeof(addUserPasswordCleartext));
				if (imgui::Button("Add")) {
					client.addUser(addUsername, ArchAdminClient::getHashOfPassword(addUserPasswordCleartext));
					addUsername[0] = '\0';
					addUserPasswordCleartext[0] = '\0';
					client.listUsers(users);
				}
				imgui::EndPopup();
			}
		}

		if (changingUser) {
			imgui::SetNextWindowPos(imgui::GetIO().DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5, 1.0));
			imgui::OpenPopup("Change User Password");
			if (imgui::BeginPopupModal("Change User Password", &changingUser, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
				imgui::InputText("User Name", const_cast<char*>(users[changeId].first.c_str()), users[changeId].first.size() + 1, ImGuiInputTextFlags_ReadOnly);
				imgui::InputText("Password(Clear Text)", changePassClear, sizeof(changePassClear));
				if (imgui::Button("Change")) {
					client.changeUser(const_cast<char*>(users[changeId].first.c_str()), ArchAdminClient::getHashOfPassword(changePassClear));
					changePassClear[0] = '\0';
					client.listUsers(users);
					changingUser = false;
				}
				imgui::EndPopup();
			}
		}

		if (showSha256Window) {
			imgui::SetNextWindowPos(imgui::GetIO().DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5, 1.0));
			imgui::OpenPopup("SHA-256 Hash");
			if (imgui::BeginPopupModal("SHA-256 Hash", &showSha256Window, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
				imgui::PushItemWidth(500);
				if (imgui::InputText("Input", hashInput, sizeof(hashInput)))
					strcpy(hashOutput, sha256(hashInput).c_str());
				imgui::InputText("Output", hashOutput, sizeof(hashOutput), ImGuiInputTextFlags_ReadOnly);
				imgui::PopItemWidth();
				if (imgui::Button("Copy"))
					imgui::SetClipboardText(hashOutput);
				imgui::EndPopup();
			}
		}

		//if (findWinOpen) {
		//	imgui::SetNextWindowPos(imgui::GetIO().DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5, 1.0));
		//	imgui::OpenPopup("Find User");
		//	if (imgui::BeginPopupModal("Find User", &findWinOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {

		//		imgui::EndPopup();
		//	}
		//}

		if (showNotFoundWindow) {
			imgui::SetNextWindowPos(imgui::GetIO().DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5, 1.0));
			imgui::OpenPopup("Not Found");
			if (imgui::BeginPopupModal("The string was not found in the remaining usernames.", &showNotFoundWindow, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {

				imgui::EndPopup();
			}
		}

		imgui::SetNextWindowSize(imgui::GetIO().DisplaySize + ImVec2(2, 2), ImGuiCond_Always);
		imgui::SetNextWindowPos(ImVec2(-1, -1), ImGuiCond_Always);
		imgui::Begin("MainWindow", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);

		if (imgui::BeginMenuBar()) {
			if (imgui::BeginMenu("Connection")) {
				if (imgui::MenuItem("Disconnect               ")) {
					client.disconnect();
					loggedIn = false;
					//firstRun = true;
					reloadPending = true;
				}
				imgui::EndMenu();
			}
			if (imgui::BeginMenu("Users")) {
				if (imgui::MenuItem("Add User...              ")) {
					addUsername[0] = '\0';
					addUserPasswordCleartext[0] = '\0';
					addingUser = true;
				}
				//imgui::Separator();
				//if (imgui::MenuItem("Find...                  ")) {
				//	findWinOpen = true;
				//}
				imgui::EndMenu();
			}
			if (imgui::BeginMenu("Extras")) {
				if (imgui::MenuItem("SHA-256 Hash...          ")) {
					hashInput[0] = '\0';
					hashOutput[0] = '\0';
					showSha256Window = true;
				}
				imgui::MenuItem("Show Demo Window", nullptr, &showDemoWindow);
				imgui::EndMenu();
			}

			imgui::EndMenuBar();
		}

		if (imgui::Button("Refresh"))
			reloadPending = true;
		imgui::SameLine();
		imgui::VerticalSeparator();
		imgui::SameLine();
		imgui::TextUnformatted("Find");
		imgui::PushItemWidth(150);
		imgui::SameLine();
		imgui::InputText("##FindContent", findContent, sizeof(findContent));
		imgui::PopItemWidth();
		imgui::SameLine();
		if (newFound)
			newFound = false;
		if (imgui::IsMouseClicked(1))
			curFindId = -1;
		if (imgui::Button("Find Next")) {
			string buf = findContent;
			if (findCaseInsensitive)
				for (int i = 0; buf[i] != '\0'; i++)
					buf[i] = toupper(buf[i]);
			int cur0, begin = curFindId + 1;
			for (cur0 = begin; cur0 = (cur0 + 1) % users.size(); cur0 != curFindId && cur0 != begin) {
				string usern = users[cur0].first;
				if (findCaseInsensitive)
					for (int i = 0; usern[i] != '\0'; i++)
						usern[i] = toupper(usern[i]);
				if (usern.find(buf) != string::npos)
					break;
			}
			if (cur0 != curFindId)
				showNotFoundWindow = true;
			else {
				cur0 = curFindId;
				newFound = true;
			}
		}
		imgui::SameLine();
		if (imgui::Selectable("Case Insensitive", &findCaseInsensitive));
		imgui::SameLine();
		imgui::Text("CurFindId: %d", curFindId);

		imgui::Separator();

		imgui::Text("User Count: %d", users.size());

		imgui::BeginChild("##ManageUsersChild", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		rowBoundingRects.resize(users.size());
		imgui::BeginColumns("##UsersColumns", 3);
		//imgui::SetColumnWidth(0, 36);

		//imgui::Text("Id");
		//for (int i = 1; i <= users.size(); i++) {
		//	imgui::Text("%d", i);

		//	ImVec2 leftTop = imgui::GetItemRectMin();
		//	rowBoundingRects[i - 1].left = leftTop.x - imgui::GetStyle().ItemSpacing.x + 2;
		//	rowBoundingRects[i - 1].top = leftTop.y - imgui::GetStyle().ItemSpacing.y / 2;
		//}

		//imgui::NextColumn();
		imgui::Text("UserName");
		for (int i = 0; i < users.size(); i++) {
			imgui::TextUnformatted(users[i].first.c_str());

			ImVec2 leftTop = imgui::GetItemRectMin();
			rowBoundingRects[i].left = leftTop.x - imgui::GetStyle().ItemSpacing.x + 2;
			rowBoundingRects[i].top = leftTop.y - imgui::GetStyle().ItemSpacing.y / 2;

			if (newFound && i == curFindId)
				imgui::SetScrollHere(0.3);
		}

		imgui::NextColumn();
		imgui::Text("Hashed Password");
		for (int i = 0; i < users.size(); i++) {
			imgui::TextUnformatted(users[i].second.first.c_str());
		}

		imgui::NextColumn();
		imgui::Text("Session");
		for (int i = 0; i < users.size(); i++) {
			if (users[i].second.second != "")
				imgui::TextUnformatted(users[i].second.second.c_str());
			else
				imgui::TextUnformatted("N/A");

			ImVec2 leftTop = imgui::GetItemRectMin();
			ImVec2 size = imgui::GetItemRectSize();
			rowBoundingRects[i].width = (leftTop.x + imgui::GetColumnWidth()) - rowBoundingRects[i].left - 4;
			rowBoundingRects[i].height = size.y + imgui::GetStyle().ItemSpacing.y;
		}

		imgui::EndColumns();

		//if (imgui::IsMouseHoveringWindow && imgui::IsMouseClicked(1))
		//	curFindId = -1;

		imgui::EndChild();

		for (int i = 0; i < users.size(); i++) {
			auto& j = rowBoundingRects[i];
			imgui::PushID(i);
			bool rectAdded = false;
			if (j.contains(imgui::GetIO().MousePos)) {
				imgui::GetWindowDrawList()->AddRect(ImVec2(j.left, j.top), ImVec2(j.left + j.width, j.top + j.height),
													imgui::ColorConvertFloat4ToU32(imgui::GetStyleColorVec4(ImGuiCol_Border)));
				rectAdded = true;
				if (imgui::IsMouseClicked(1)) {
					imgui::OpenPopup("itemContextMenu");
				}
			}
			if (newFound && i == curFindId) {
				imgui::GetWindowDrawList()->AddRect(ImVec2(j.left, j.top), ImVec2(j.left + j.width, j.top + j.height),
													imgui::ColorConvertFloat4ToU32(imgui::GetStyleColorVec4(ImGuiCol_Border)));
				rectAdded = true;
			}
			if (imgui::BeginPopup("itemContextMenu")) {
				if (imgui::Selectable("Copy Username"))
					imgui::SetClipboardText(users[i].first.c_str());
				if (imgui::Selectable("Copy Hashed Password"))
					imgui::SetClipboardText(users[i].second.first.c_str());
				if (users[i].second.second == "")
					imgui::Selectable("Copy User Session", false, ImGuiSelectableFlags_Disabled);
				else
					if (imgui::Selectable("Copy User Session"))
						imgui::SetClipboardText(users[i].second.second.c_str());
				imgui::Separator();
				if (imgui::Selectable("Change Password...                 ")) {
					changingUser = true;
					changeId = i;
				}
				if (imgui::Selectable("Delete User                        ")) {
					client.deleteUser(users[i].first);
					reloadPending = true;
				}
				imgui::Separator();
				if (imgui::Selectable("Clear Session                      ")) {
					client.removeSession(users[i].first);
					users[i].second.second = "";
				}
				if (imgui::Selectable("Acquire Session if Not Present     "))
					client.acquireSession(users[i].first, users[i].second.second);
				imgui::EndPopup();
			}
			if (!rectAdded && imgui::IsPopupOpen("itemContextMenu"))
				imgui::GetWindowDrawList()->AddRect(ImVec2(j.left, j.top), ImVec2(j.left + j.width, j.top + j.height),
													imgui::ColorConvertFloat4ToU32(imgui::GetStyleColorVec4(ImGuiCol_Border)));
			imgui::PopID();
		}

		imgui::End();

		if (showDemoWindow)
			imgui::ShowDemoWindow(&showDemoWindow);

	}


	void runImGui() {
		imgui::PushStyleColor(ImGuiCol_ModalWindowDarkening, Color(0, 0, 0, 96));
		GImGui->ModalWindowDarkeningRatio = 1.0f;
		if (!loggedIn)
			runLoginGui();
		else
			runManageGui();
		imgui::PopStyleColor();
	}
}
