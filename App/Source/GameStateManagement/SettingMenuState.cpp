// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

#include "SettingMenuState.h"

// Include CGameStateManager
#include "GameStateManager.h"

// Include Mesh Builder
#include "Primitives/MeshBuilder.h"
// Include ImageLoader
#include "System\ImageLoader.h"
// Include Shader Manager
#include "RenderControl\ShaderManager.h"

 // Include shader
#include "RenderControl\shader.h"

// Include CSettings
#include "GameControl/Settings.h"

// Include CKeyboardController
#include "Inputs/KeyboardController.h"

// Include SoundController
#include "../SoundController/SoundController.h"


#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CSettingMenuState::CSettingMenuState(void)
	: background(NULL)
{

}

/**
 @brief Destructor
 */
CSettingMenuState::~CSettingMenuState(void)
{

}

/**
 @brief Init this class instance
 */
bool CSettingMenuState::Init(void)
{
	cout << "CSettingMenuState::Init()\n" << endl;

	CShaderManager::GetInstance()->Use("Shader2D");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	//Create Background Entity
	background = new CBackgroundEntity("Image/TitleScreen.png");
	background->SetShader("Shader2D");
	background->Init();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(CSettings::GetInstance()->pWindow, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load the images for buttons
	CImageLoader* il = CImageLoader::GetInstance();
	returnButtonData.fileName = "Image\\GUI\\ReturnButton.png";
	returnButtonData.textureID = il->LoadTextureGetID(returnButtonData.fileName.c_str(), false);

	FullscreenButtonData.fileName = "Image\\GUI\\FullscreenButton.png";
	FullscreenButtonData.textureID = il->LoadTextureGetID(FullscreenButtonData.fileName.c_str(), false);

	muteButtonData.fileName = "Image\\GUI\\MuteButton.png";
	muteButtonData.textureID = il->LoadTextureGetID(muteButtonData.fileName.c_str(), false);

	// Enable the cursor
	if (CSettings::GetInstance()->bDisableMousePointer == true)
		glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	return true;
}

/**
 @brief Update this class instance
 */
bool CSettingMenuState::Update(const double dElapsedTime)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;

	float buttonWidth = 128;
	float buttonHeight = 64;

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		// Create a window called "Hello, world!" and append into it.
		ImGui::Begin("Main Menu", NULL, window_flags);
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth/2.0 - buttonWidth/2.0, 
			CSettings::GetInstance()->iWindowHeight/1.8));				// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, CSettings::GetInstance()->iWindowHeight));

		//Added rounding for nicer effect
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;
		
		// Add codes for Mute button here
		if (ImGui::ImageButton((ImTextureID)muteButtonData.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			// Load the menu state
			cout << "Muting Audio" << endl;
			CSettings::GetInstance()->MuteAudio = !CSettings::GetInstance()->MuteAudio;
		}

		// Add codes for Fullscreen button here
		if (ImGui::ImageButton((ImTextureID)FullscreenButtonData.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			// Load the menu state
			cout << "Toggling Fullscreen" << endl;
			CSettings* cSettings = CSettings::GetInstance();
			cSettings->isFullScreen = !cSettings->isFullScreen;

			if (cSettings->isFullScreen)
			{
				glfwSetWindowMonitor(cSettings->pWindow, glfwGetPrimaryMonitor(), 0, 0, cSettings->iWindowWidth, cSettings->iWindowHeight, cSettings->FPS);
			}
			else 
			{
				glfwSetWindowMonitor(cSettings->pWindow, nullptr, 100, 100, cSettings->iWindowWidth, cSettings->iWindowHeight, cSettings->FPS);
			
			}
			
		}

		// Add codes for Return button here
		if (ImGui::ImageButton((ImTextureID)returnButtonData.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			// Load the menu state
			cout << "Loading MenuState" << endl;
			CGameStateManager::GetInstance()->SetActiveGameState("MenuState");
		}
		ImGui::End();
	}

	//For keyboard controls
	if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_SPACE))
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading PlayGameState" << endl;
		CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		return true;
	}
	else if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE))
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Quitting the game from MenuState" << endl;
		return false;
	}

	return true;
}

/**
 @brief Render this class instance
 */
void CSettingMenuState::Render(void)
{
	// Clear the screen and buffer
	glClearColor(0.0f, 0.55f, 1.00f, 1.00f);

	//Render Background
	background->Render();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//cout << "CSettingMenuState::Render()\n" << endl;
}

/**
 @brief Destroy this class instance
 */
void CSettingMenuState::Destroy(void)
{
	// Disable the cursor
	if (CSettings::GetInstance()->bDisableMousePointer == true)
		glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		// Hide the cursor
		if (CSettings::GetInstance()->bShowMousePointer == false)
			glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// Delete the background
	if (background)
	{
		delete background;
		background = NULL;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	cout << "CSettingMenuState::Destroy()\n" << endl;
}
