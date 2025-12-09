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
#define VERSION_NUM "0.2.0"
#define ERROR "Arpadica::ERROR: "

using namespace std;

const int screenWidth = 1280;
const int screenHeight = 720;

string map_file = "./assets/maps/map.geojson";
string heightmap = "./assets/maps/heightmap.jpg";
string colormap = "./assets/maps/colormap.jpg";

const int mainMapTexWidth = 16384;
const int mainMapTexHeight = 8192;
const string overlayShader_fs = "assets/shaders/map_overlay.fs";
const string overlayShader_vs = "assets/shaders/map_overlay.vs";

int CountrySelectorScrollIndex = 0;
int CountrySelectorActive = 0;

std::string getTitle(float fps = -1);

void renderMapOverlay(MapEngine& mapEngine, RenderTexture2D& targetTex, Camera2D& camera, int screenWidth, int screenHeight);
Vector2 mouseToMap(Ray ray, Vector3 mapPosition, float sizeX, float sizeZ, MapEngine& mapEngine);

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

	GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
	GuiSetFont(baseFont);

	/* CAMERA */
	Camera camera = { 0 };
	camera.position = (Vector3){ 18.0f, 21.0f, 18.0f };     // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                                    // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

	
	const float ZOOM_MIN = 5.0f;
	const float ZOOM_MAX = 200.0f;
	const float ZOOM_SPEED = 0.2f;
	const float ZOOM_SMOOTHNESS = 0.5f;

	float camDistance = Vector3Length(Vector3Subtract(camera.position, camera.target));
	float targetDistance = camDistance;

	vector<Country> countries;
	int selectedCountry;

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

	Country slovakia("SVK", BLUE);
	slovakia.setNames(
		"Slovakia",
		"Slovakia",
		"Slovakia",
		"Slovakia",
		"Slovakia",
		"Slovakia",
		"Slovakia"
	);

	Country czechia("CZE", Color{ 173, 216, 230, 255 });
	czechia.setNames(
		"Czechia",
		"Czechia",
		"Czechia",
		"Czechia",
		"Czechia",
		"Czechia",
		"Czechia"
	);

	Country romania("ROM", GOLD);
	czechia.setNames(
		"Romania",
		"Romania",
		"Romania",
		"Romania",
		"Romania",
		"Romania",
		"Romania"
	);

	countries.push_back(hungary);
	countries.push_back(austria);
	countries.push_back(slovakia);
	countries.push_back(czechia);
	countries.push_back(romania);

	/* MAIN MAP */
	MapEngine mapEngine(mainMapTexWidth, mainMapTexHeight);

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

	RenderTexture2D mainMapTex = LoadRenderTexture(mainMapTexWidth, mainMapTexHeight);
	SetTextureFilter(mainMapTex.texture, TEXTURE_FILTER_BILINEAR);

	Camera2D mapCam = { 0 }; // Camera for main 2D map
	mapCam.target = { 0, 0 };
	mapCam.offset = { 0, 0 };
	mapCam.rotation = 0;
	mapCam.zoom = 1.0f;

	/* HEIGHTMAP */
	BeginDrawing();
	ClearBackground(DARKBLUE);
	DrawTextEx(baseFont, "Generating map model...", {(float)(screenWidth - MeasureText("Generating map model...", 20)) / 2, screenHeight / 2}, 20, 1, WHITE);
	EndDrawing();

	Image image = LoadImage(heightmap.c_str());     // Earth heightmap image (RAM)
	Texture2D heightmap = LoadTextureFromImage(image);        // Convert image to texture (VRAM)
	Texture2D colormapTex = LoadTexture(colormap.c_str());

	float sizeX = 200.0f;
	float sizeZ = 100.0f;
	Mesh mapMesh = GenMeshHeightmap(image, (Vector3){ sizeX, 1.0f, sizeZ }); // Generate heightmap mesh from image
	Model mapModel = LoadModelFromMesh(mapMesh);                  // Load model from generated mesh

	mapModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = colormapTex; // Set map diffuse heightmap
	Vector3 mapPosition = { -sizeX * 0.5f, 0.0f, -sizeZ * 0.5f };


	UnloadImage(image);             // Unload heightmap image from RAM, already uploaded to VRAM


	/* SHADERS */
	DrawTextEx(baseFont, "Setting up shaders...", {(float)(screenWidth - MeasureText("Setting up shaders...", 20)) / 2, screenHeight / 2}, 20, 1, WHITE);

	Shader overlayShader = LoadShader(overlayShader_vs.c_str(), overlayShader_fs.c_str());
	int locOverlay = GetShaderLocation(overlayShader, "politicalMap");
	int locOverlayMix  = GetShaderLocation(overlayShader, "overlayMix");
	int locWorldMinMax = GetShaderLocation(overlayShader, "worldMinMax");

	// Bind the overlay texture to sampler slot 1
	SetShaderValueTexture(overlayShader, locOverlay, mainMapTex.texture);

	// Blend strength
	float overlayMix = 0.75f;
	SetShaderValue(overlayShader, locOverlayMix, &overlayMix, SHADER_UNIFORM_FLOAT);

	// Setting map size
	float worldMinMax[4] = {
		mapPosition.x,          // minX
		mapPosition.x + sizeX,  // maxX
		mapPosition.z,          // minZ
		mapPosition.z + sizeZ   // maxZ
	};
	SetShaderValue(overlayShader, locWorldMinMax, worldMinMax, SHADER_UNIFORM_VEC4);

	// Lighting uniforms
	int locLightDir   = GetShaderLocation(overlayShader, "lightDir");
	int locLightColor = GetShaderLocation(overlayShader, "lightColor");
	int locAmbient    = GetShaderLocation(overlayShader, "ambient");

	// Direction TO light 
	Vector3 lightDir = Vector3Normalize((Vector3){ 0.2f, 1.0f, 0.2f });
	float lightDirV[3] = { lightDir.x, lightDir.y, lightDir.z };
	float lightColorV[3] = { 1.0f, 1.0f, 1.0f };
	float ambient = 0.25f; 

	SetShaderValue(overlayShader, locLightDir,   lightDirV,   SHADER_UNIFORM_VEC3);
	SetShaderValue(overlayShader, locLightColor, lightColorV, SHADER_UNIFORM_VEC3);
	SetShaderValue(overlayShader, locAmbient,    &ambient,    SHADER_UNIFORM_FLOAT);

	mapModel.materials[0].shader = overlayShader;
	

	State selectedState;
	string stateInfo = "";

	vector<State> selectedStates;

	renderMapOverlay(mapEngine, mainMapTex, mapCam, mainMapTexWidth, mainMapTexHeight);
	while (!WindowShouldClose())
	{

		SetWindowTitle(getTitle((float)GetFPS()).c_str());

		float dt = GetFrameTime();

		// Zoom
		float wheel = GetMouseWheelMove();
		float t = 1.0f - powf(0.001f, dt * ZOOM_SMOOTHNESS);
		if (wheel != 0.0f)
		{
			targetDistance *= expf(-wheel * ZOOM_SPEED); 
			targetDistance = Clamp(targetDistance, ZOOM_MIN, ZOOM_MAX);
		}
		// Smoothly approach target distance even when wheel is idle
		camDistance = Lerp(camDistance, targetDistance, t);
		// Recompute camera.position along forward vector toward target
		Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
		camera.position = Vector3Subtract(camera.target, Vector3Scale(forward, camDistance));

		// Pan with mouse
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			Vector2 delta = GetMouseDelta();
			camera.position.x += delta.x * 0.1f;
			camera.position.z += delta.y * 0.1f;
			camera.target.x += delta.x * 0.1f;
			camera.target.z += delta.y * 0.1f;
		}

		// State info
		if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
		{
			Vector2 mouse = GetMousePosition();
			Ray ray = GetMouseRay(mouse, camera);

			Vector2 worldPos = mouseToMap(ray, mapPosition, sizeX, sizeZ, mapEngine);

			selectedState = mapEngine.getStateAt((int)worldPos.x, (int)worldPos.y);
			if (selectedState.id != "") {
				stateInfo = "State ID: " + selectedState.id + " | Name: " + selectedState.name_en;
			}
		}

		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			Vector2 mouse = GetMousePosition();
			Ray ray = GetMouseRay(mouse, camera);

			Vector2 worldPos = mouseToMap(ray, mapPosition, sizeX, sizeZ, mapEngine);

			selectedState = mapEngine.getStateAt((int)worldPos.x, (int)worldPos.y);
			if (selectedState.id != "") 
			{
				// Set color based on selected country
				Color countryColor = countries[selectedCountry].getColor();
				mapEngine.setStateColor(selectedState.id, countryColor);
				renderMapOverlay(mapEngine, mainMapTex, mapCam, mainMapTexWidth, mainMapTexHeight);
			}

			
		}

		BeginDrawing();

		ClearBackground(RAYWHITE);


		BeginMode3D(camera);
			DrawModel(mapModel, mapPosition, 1.0f, WHITE);
		EndMode3D();


		if (!stateInfo.empty()) {
            //DrawText(stateInfo.c_str(), 10, screenHeight - 30, 16, YELLOW);
			DrawTextEx(baseFont, stateInfo.c_str(), {10, (float)(screenHeight - 30)}, 28, 1, YELLOW);
        }


		GuiStatusBar((Rectangle){ 0, 00, screenWidth, 48 }, NULL);
		GuiLabel((Rectangle){ 152, 8, 200, 24 }, "Selected Country");

		// GuiListView that contains all countries
		std::string countryListStr = "";
		for (size_t i = 0; i < countries.size(); i++)
		{
			countryListStr += countries[i].getId();
			if (i < countries.size() - 1)
			{
				countryListStr += ";";
			}
		}

		GuiListView((Rectangle){ 24, 8, 120, 72 }, countryListStr.c_str(), &CountrySelectorScrollIndex, &CountrySelectorActive);

		// Set selected country based on active index
		if (CountrySelectorActive >= 0 && CountrySelectorActive < (int)countries.size())
		{
			selectedCountry = CountrySelectorActive;
		}


		//DrawTexture(heightmap, screenWidth - heightmap.width - 20, 20, WHITE);

		DrawFPS(10, 10);

		EndDrawing();
	}

	UnloadTexture(heightmap);
	UnloadModel(mapModel);
	UnloadShader(overlayShader);
	UnloadRenderTexture(mainMapTex);

	CloseWindow();
	return 0;
}

void renderMapOverlay(MapEngine& mapEngine, RenderTexture2D& targetTex, Camera2D& camera, int screenWidth, int screenHeight)
{
	BeginTextureMode(targetTex);
		ClearBackground(BLANK);
		BeginMode2D(camera);
			mapEngine.render(camera);
			mapEngine.render_outline(camera);
		EndMode2D();
	EndTextureMode();
}

Vector2 mouseToMap(Ray ray, Vector3 mapPosition, float sizeX, float sizeZ, MapEngine& mapEngine)
{
	float denom = ray.direction.y;
	if (fabsf(denom) > 1e-6f) {
		float t = -ray.position.y / denom; // since plane point is (0,0,0) and normal is (0,1,0)
		if (t >= 0.0f) {
			Vector3 hit = Vector3Add(ray.position, Vector3Scale(ray.direction, t)); // world-space hit

			// Convert world hit to map texture/local coordinates
			// Your map spans [mapPosition.x, mapPosition.x + sizeX] in X and [mapPosition.z, mapPosition.z + sizeZ] in Z
			float localX = hit.x - mapPosition.x; // 0..sizeX
			float localZ = hit.z - mapPosition.z; // 0..sizeZ

			// Optionally clamp
			if (localX >= 0 && localX <= sizeX && localZ >= 0 && localZ <= sizeZ) {
				// If your mapEngine expects pixel coords matching heightmap/colormap resolution:
				float u = localX / sizeX;              // 0..1
				float v = localZ / sizeZ;              // 0..1
				int px = (int)(u * mainMapTexWidth);   // or use the geojson pixel space
				int py = (int)(v * mainMapTexHeight);

				return Vector2{ (float)px, (float)py };
			}
		}
	}

	return Vector2{};
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