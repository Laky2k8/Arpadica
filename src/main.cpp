#include "raylib.h"
#include "raymath.h"

#include <string>
#include <iostream>
#include <cmath>
#include <cfloat>

#include "map_engine.hpp"

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


	std::vector<int> glyphs;
	for (int cp = 32; cp <= 0x017F; ++cp) glyphs.push_back(cp);

	Font baseFont = LoadFontEx("./assets/fonts/Poppins/normal.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontI = LoadFontEx("./assets/fonts/Poppins/italic.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontB = LoadFontEx("./assets/fonts/Poppins/bold.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontBI = LoadFontEx("./assets/fonts/Poppins/bold_italic.ttf", 96, glyphs.data(), (int)glyphs.size());

	MapEngine mapEngine(screenWidth, screenHeight);

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

	State selectedState;
	string stateInfo = "";

	while (!WindowShouldClose())
	{
		SetWindowTitle(getTitle((float)GetFPS()).c_str());

		// Zoom
		camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
		if (camera.zoom > 10.0f) camera.zoom = 10.0f;
		else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

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

		EndDrawing();

	}
	CloseWindow();
	return 0;
}