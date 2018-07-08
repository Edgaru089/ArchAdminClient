#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#pragma warning(disable: 4018) //'expression' : signed/unsigned mismatch

#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

#include "ImGui/imgui-setup.h"
#include "LogSystem.hpp"
#include "ArchAdminClient.hpp"

#include "Manager.hpp"

int main(int argc, char* argv[]) {
	ofstream logout("latest.log");
	dlog.addOutputStream(clog);
	dlog.addOutputStream(logout);

	RenderWindow win;
	win.create(VideoMode(800, 600), "ArchUserServer Administrative Client");
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
