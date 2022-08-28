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

#include "JumpscareState.h"

// Include CGameStateManager
#include "GameStateManager.h"

#include "../Scene2D/GameManager.h"

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

#include "../SoundController/SoundController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CJumpscareState::CJumpscareState(void)
	//: background(NULL)
{

}

/**
 @brief Destructor
 */
CJumpscareState::~CJumpscareState(void)
{

}

/**
 @brief Init this class instance
 */
bool CJumpscareState::Init(void)
{
	cout << "CJumpscareState::Init()\n" << endl;

	CShaderManager::GetInstance()->Use("Shader2D");
	background = new CBackgroundEntity("Image/Assets/jumpscare.png");
	background->SetShader("Shader2D");
	background->Init();
	endTime = 60.0f;
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);
	return true;
}

/**
 @brief Update this class instance
 */
bool CJumpscareState::Update(const double dElapsedTime)
{
	if (endTime != 0)
	{
		endTime--;
	}
	else if(endTime == 0)
	{
		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();

		// Load the menu state
		cout << "Loading WinLoseState" << endl;
		CGameManager::GetInstance()->bPlayerLost = true;
		CGameStateManager::GetInstance()->SetActiveGameState("LoseWinState");

		return true;
	}

	return true;
}

/**
 @brief Render this class instance
 */
void CJumpscareState::Render(void)
{
	// Clear the screen and buffer
	glClearColor(0.0f, 0.55f, 1.00f, 1.00f);

	//Draw the background
 	background->Render();
}

/**
 @brief Destroy this class instance
 */
void CJumpscareState::Destroy(void)
{
	// Delete the background
	if (background)
	{
		delete background;
		background = NULL;
	}

	cout << "CIntroState::Destroy()\n" << endl;
}
