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

#include "LoseWinState.h"

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
CLoseWinState::CLoseWinState(void)
	//: background(NULL)
{

}

/**
 @brief Destructor
 */
CLoseWinState::~CLoseWinState(void)
{

}

/**
 @brief Init this class instance
 */
bool CLoseWinState::Init(void)
{
	cout << "CLoseWinState::Init()\n" << endl;

	CShaderManager::GetInstance()->Use("Shader2D");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	//Create Background Entity
	/*background = new CBackgroundEntity("Image/Assets/LoseScrn.png");
	background->SetShader("Shader2D");
	background->Init();*/

	//Game Manager
	cGameManager = CGameManager::GetInstance();

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
	startButtonData.fileName = "Image\\GUI\\RetryButton.png";
	startButtonData.textureID = il->LoadTextureGetID(startButtonData.fileName.c_str(), false);

	exitButtonData.fileName = "Image\\GUI\\ReturnButton.png";
	exitButtonData.textureID = il->LoadTextureGetID(exitButtonData.fileName.c_str(), false);

	// Enable the cursor
	if (CSettings::GetInstance()->bDisableMousePointer == true)
		glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (CSettings::GetInstance()->MuteAudio == false)
	{
		//Play Sound
		CSoundController::GetInstance()->PlaySoundByID(3);
	}

	return true;
}

/**
 @brief Update this class instance
 */
bool CLoseWinState::Update(const double dElapsedTime)
{
	if (CSettings::GetInstance()->MuteAudio == false)
	{
		CSoundController::GetInstance()->StopPlayByID(4);
	}

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
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - buttonWidth / 2,
			CSettings::GetInstance()->iWindowHeight / 1.85f));				// Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->iWindowWidth, 
			CSettings::GetInstance()->iWindowHeight));
		ImGui::SetWindowFontScale(2.5f);

		//ImGui::Begin("Invisible window", NULL, window_flags);
		//ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		//ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
		//ImGui::SetWindowFontScale(1.5f * relativeScale_y);

		cout << "bPlayerLost: " << cGameManager->bPlayerLost << endl;

		if (cGameManager->bPlayerWon == true)
		{
			background = new CBackgroundEntity("Image/Assets/WinScrn.png");
			background->SetShader("Shader2D");
			background->Init();
		}
		else if (cGameManager->bPlayerLost == true)
		{
			background = new CBackgroundEntity("Image/Assets/LoseScrn.png");
			background->SetShader("Shader2D");
			background->Init();
		}

		else 
		{
			ImGui::TextColored(ImVec4(1, 1, 1, 1), "GameOver!", NULL);
		}
		
		ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->iWindowWidth / 2.0 - buttonWidth / 2.0,
			CSettings::GetInstance()->iWindowHeight / 1.8f));				// Set the top-left of the window at (10,10)


		//Added rounding for nicer effect
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 200.0f;

		// Add codes for Start button here
		if (ImGui::ImageButton((ImTextureID)startButtonData.textureID,
			ImVec2(buttonWidth, buttonHeight), ImVec2(0.0, 0.0), ImVec2(1.0, 1.0)))
		{
			// Reset the CKeyboardController
			CKeyboardController::GetInstance()->Reset();

			// Load the menu state
			cout << "Loading PlayGameState" << endl;
			CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		}

		// Add codes for Exit button here
		if (ImGui::ImageButton((ImTextureID)exitButtonData.textureID,
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

	return true;
}

/**
 @brief Render this class instance
 */
void CLoseWinState::Render(void)
{
	// Clear the screen and buffer
	glClearColor(0.0f, 0.55f, 1.00f, 1.00f);

	//Render Background
	background->Render();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief Destroy this class instance
 */
void CLoseWinState::Destroy(void)
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

	cout << "CMenuState::Destroy()\n" << endl;
}
