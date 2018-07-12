#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#pragma warning(disable: 4018) //'expression' : signed/unsigned mismatch

#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

#include "ImGui/imgui-setup.h"
#include "Version.hpp"
#include "ArchAdminClient.hpp"

#include "Manager.hpp"

String u8ToSfString(const char* u8string) { return String::fromUtf8(u8string, u8string + strlen(u8string)); }
String u8ToSfString(const string& str) { return String::fromUtf8(str.begin(), str.end()); }

int main(int argc, char* argv[]) {
	RenderWindow win;
	win.create(VideoMode(960, 640),
			   u8ToSfString("ArchUserServer Administrative Client " + stageString + versionString + " (Arch " + archVersionString + ")"),
			   Style::Titlebar | Style::Resize | Style::Close);
	win.clear(); win.display();
	win.setVerticalSyncEnabled(true);
	win.resetGLStates();

	imgui::SFML::Init(win);
	imgui::GetIO().IniFilename = nullptr;
	ImGui::StyleColorsClassic();
	auto& style = imgui::GetStyle();
	style.FrameBorderSize = 1.0f;
	style.ScrollbarRounding = 0.0f;
	style.WindowRounding = 0.0f;
	style.ChildBorderSize = 0.0f;

	Clock deltaClock;
	while (win.isOpen()) {
		Event e;
		while (win.pollEvent(e)) {
			imgui::SFML::ProcessEvent(e);
			if (e.type == Event::Closed)
				win.close();
		}
		imgui::SFML::Update(win, deltaClock.restart());

		Manager::runImGui();

		win.clear();

		imgui::SFML::Render(win);

		win.display();
	}

	return 0;
}
