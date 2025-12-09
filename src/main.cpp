#define RAYGUI_IMPLEMENTATION
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
string heightmap = "./assets/heightmap.jpg";

std::string getTitle(float fps = -1);

int main() 
{
	InitWindow(screenWidth, screenHeight, getTitle().c_str());
	SetTargetFPS(165);

	/* FONTS */
	std::vector<int> glyphs;
	for (int cp = 32; cp <= 0x017F; ++cp) glyphs.push_back(cp);

	Font baseFont = LoadFontEx("./assets/fonts/Poppins/normal.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontI = LoadFontEx("./assets/fonts/Poppins/italic.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontB = LoadFontEx("./assets/fonts/Poppins/bold.ttf", 96, glyphs.data(), (int)glyphs.size());
	Font baseFontBI = LoadFontEx("./assets/fonts/Poppins/bold_italic.ttf", 96, glyphs.data(), (int)glyphs.size());

	/* CAMERA */
	Camera camera = { 0 };
    camera.position = (Vector3){ 18.0f, 21.0f, 18.0f };     // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

	/* MAIN MAP */
	MapEngine mapEngine(screenWidth, screenHeight);

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

	/* HEIGHTMAP */
	BeginDrawing();
	ClearBackground(DARKBLUE);
	DrawTextEx(baseFont, "Generating map model...", {(float)(screenWidth - MeasureText("Generating map model...", 20)) / 2, screenHeight / 2}, 20, 1, WHITE);
	EndDrawing();

    Image image = LoadImage(heightmap.c_str());     // Earth heightmap image (RAM)
    Texture2D heightmap = LoadTextureFromImage(image);        // Convert image to texture (VRAM)

    Mesh mapMesh = GenMeshHeightmap(image, (Vector3){ 200.0f, 1.0f, 100.0f }); // Generate heightmap mesh from image
    Model mapModel = LoadModelFromMesh(mapMesh);                  // Load model from generated mesh

    mapModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightmap; // Set map diffuse heightmap
    Vector3 mapPosition = { -8.0f, 0.0f, -8.0f };           // Define model position

    UnloadImage(image);             // Unload heightmap image from RAM, already uploaded to VRAM


	while (!WindowShouldClose())
	{

		SetWindowTitle(getTitle((float)GetFPS()).c_str());

		// Pan with mouse
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			Vector2 delta = GetMouseDelta();
			camera.position.x += delta.x * 0.1f;
			camera.position.z += delta.y * 0.1f;
			camera.target.x += delta.x * 0.1f;
			camera.target.z += delta.y * 0.1f;
		}

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

			DrawModel(mapModel, mapPosition, 1.0f, WHITE);

			DrawGrid(20, 1.0f);

		EndMode3D();

		//DrawTexture(heightmap, screenWidth - heightmap.width - 20, 20, WHITE);
		DrawRectangleLines(screenWidth - heightmap.width - 20, 20, heightmap.width, heightmap.height, GREEN);

		DrawFPS(10, 10);

		EndDrawing();
	}

	UnloadTexture(heightmap);     // Unload heightmap
    UnloadModel(mapModel);         // Unload model

	CloseWindow();
	return 0;
}

std::string getTitle(float fps)
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