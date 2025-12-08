#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <string>
#include <iostream>
#include <cmath>
#include <cfloat>
#include <vector>

#include "map_engine.hpp"
#include "country.hpp"

#define TITLE "Arpadica"
#define VERSION_NUM "0.0.1"
#define ERROR "Arpadica::ERROR: "

using namespace std;

const int screenWidth = 1280;
const int screenHeight = 720;


string map_file = "./assets/map.geojson";

std::string getTitle(float fps = -1)
{
	if (fps >= 0)
	{
		return (string(TITLE) + " " + string(VERSION_NUM) + " - " + to_string(GetFPS()) + " FPS");
	}
	else
	{
		return (string(TITLE) + " " + string(VERSION_NUM));
	}
}

int main() 
{
	InitWindow(screenWidth, screenHeight, getTitle().c_str());
	SetTargetFPS(165);

	bool showMessageBox = false;

	std::vector<int> glyphs;
	for (int cp = 32; cp <= 0x017F; ++cp) glyphs.push_back(cp);

	Font baseFont = LoadFontEx("./assets/fonts/Poppins/normal.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontI = LoadFontEx("./assets/fonts/Poppins/italic.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontB = LoadFontEx("./assets/fonts/Poppins/bold.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontBI = LoadFontEx("./assets/fonts/Poppins/bold_italic.ttf", 96, glyphs.data(), (int)glyphs.size());

	MapEngine mapEngine(screenWidth, screenHeight);

	Country hungary("HUN", LIME);
	hungary.setNames(
		"Magyar Empire",
		"Kingdom of Hungary",
		"Hungarian Republic",
		"Hungary",
		"Hungarian Republic",
		"Hungarian Peoples' Republic",
		"Council Republic of Hungary"
	);

	Country austria("AUS", LIGHTGRAY);
	austria.setNames(
		"Süddeutsches Reich",
		"Austrian Empire",
		"Republic of Austria",
		"Austria",
		"Republic of Austria",
		"Austrian People's Republic",
		"Union of Austrian Soviets"
	);

	// Loading screen
	BeginDrawing();
	ClearBackground(DARKBLUE);
	DrawTextEx(baseFont, "Loading map...", {(float)(screenWidth - MeasureText("Loading map...", 20)) / 2, screenHeight / 2}, 20, 1, WHITE);
	EndDrawing();

	if(!mapEngine.LoadMap(map_file))
	{
		cerr << ERROR << "Failed to load map data! Exiting." << endl;
		CloseWindow();
		return 1;
	}

	Camera2D camera = {0};
	camera.target = (Vector2){ 0, 0 };
	camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	const float ZOOM_MIN = 0.1f;
	const float ZOOM_MAX = 10.0f;
	const float ZOOM_SPEED = 0.2f;
	const float ZOOM_SMOOTHNESS = 0.5f;

	float targetZoom = camera.zoom;

	State selectedState;
	string stateInfo = "";

	while (!WindowShouldClose())
	{
		SetWindowTitle(getTitle((float)GetFPS()).c_str());

		float dt = GetFrameTime();

		// Zoom
		float wheel = GetMouseWheelMove();
		if (wheel != 0.0f)
		{
			// Zoom toward mouse position
			Vector2 mouseScreen = GetMousePosition();
			Vector2 preWorld = GetScreenToWorld2D(mouseScreen, camera);

			// Update target zoom exponentially by wheel input
			targetZoom *= expf(wheel * ZOOM_SPEED);
			targetZoom = Clamp(targetZoom, ZOOM_MIN, ZOOM_MAX);

			// Smoothly approach target zoom
			float t = 1.0f - powf(0.001f, dt * ZOOM_SMOOTHNESS);
			camera.zoom = Lerp(camera.zoom, targetZoom, t);

			Vector2 postWorld = GetScreenToWorld2D(mouseScreen, camera);
			camera.target.x += preWorld.x - postWorld.x;
			camera.target.y += preWorld.y - postWorld.y;
		}
		else
		{
			// Keep smoothing even without new wheel input
			float t = 1.0f - powf(0.001f, dt * ZOOM_SMOOTHNESS);
			camera.zoom = Lerp(camera.zoom, targetZoom, t);
		}

		// Panning
		if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
			Vector2 delta = GetMouseDelta();
			camera.target.x -= delta.x / camera.zoom;
			camera.target.y -= delta.y / camera.zoom;
		}

		// State selection
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			Vector2 mousePos = GetMousePosition();
			Vector2 worldPos = {
				(mousePos.x - camera.offset.x) / camera.zoom + camera.target.x,
				(mousePos.y - camera.offset.y) / camera.zoom + camera.target.y
			};

			selectedState = mapEngine.getStateAt((int)worldPos.x, (int)worldPos.y);
			if (selectedState.id != "") {
				stateInfo = "State ID: " + selectedState.id + " | Name: " + selectedState.name_en;
			}
		}

		BeginDrawing();
		ClearBackground(BLUE);

		BeginMode2D(camera);

		mapEngine.render(camera);

		if(camera.zoom > 5.0f)
		{
			mapEngine.render_outline(camera);
		}

		EndMode2D();

		/*DrawText(("States: " + to_string(mapEngine.getStates().size())).c_str(), 10, 10, 20, WHITE);
        DrawText("Use mouse to explore", 10, 35, 16, LIGHTGRAY);
		DrawText("Left Click: Get province info", 10, 55, 16, LIGHTGRAY);
		DrawText("Control + Left Click: Paint province", 10, 75, 16, LIGHTGRAY);*/

		DrawTextEx(baseFont, ("States: " + to_string(mapEngine.getStates().size())).c_str(), {10, 10}, 28, 1, WHITE);
		DrawTextEx(baseFont, "Use mouse to explore", {10, 35}, 24, 1, LIGHTGRAY);
		DrawTextEx(baseFont, "Left Click: Get province info", {10, 55}, 24, 1, LIGHTGRAY);
		DrawTextEx(baseFont, "Control + Left Click: Paint province", {10, 75}, 24, 1, LIGHTGRAY);

		// Show hovered region info
        if (!stateInfo.empty()) {
            //DrawText(stateInfo.c_str(), 10, screenHeight - 30, 16, YELLOW);
			DrawTextEx(baseFont, stateInfo.c_str(), {10, (float)(screenHeight - 30)}, 28, 1, YELLOW);
        }

		BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            if (GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#Show Message")) showMessageBox = true;

            if (showMessageBox)
            {
                int result = GuiMessageBox((Rectangle){ 85, 70, 250, 100 },
                    "#191#Message Box", "Hi! This is a message!", "Nice;Cool");

                if (result >= 0) showMessageBox = false;
            }

        EndDrawing();

		EndDrawing();

	}
	CloseWindow();
	return 0;
}