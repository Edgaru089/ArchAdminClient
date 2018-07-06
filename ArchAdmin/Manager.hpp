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
		imgui::SetNextWindowPos(imgui::GetIO().DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5, 0.8));
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
				imgui::EndMenu();
			}

			imgui::MenuItem("Show Demo Window", nullptr, &showDemoWindow);

			imgui::EndMenuBar();
		}

		if (imgui::Button("Refresh")) {
			reloadPending = true;
		}

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


		for (int i = 0; i < users.size(); i++) {
			auto& j = rowBoundingRects[i];
			imgui::PushID(i);
			if (j.contains(imgui::GetIO().MousePos)) {
				imgui::GetOverlayDrawList()->AddRect(ImVec2(j.left, j.top), ImVec2(j.left + j.width, j.top + j.height),
													 imgui::ColorConvertFloat4ToU32(imgui::GetStyleColorVec4(ImGuiCol_Border)));
				if (imgui::IsMouseClicked(1)) {
					imgui::OpenPopup("itemContextMenu");
				}
			}
			if (imgui::BeginPopup("itemContextMenu")) {
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
			imgui::PopID();
		}

		imgui::EndChild();

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
