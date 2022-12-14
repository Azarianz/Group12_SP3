/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Player2D.h"
#include "GUI_Scene2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
#include "Primitives/MeshBuilder.h"

//Game Manager
#include "GameManager.h"

#include "Pet2D.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::CPlayer2D(void)
	: cMap2D(NULL)
	, cKeyboardController(NULL)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
{
	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::i32vec2(0);

	// Initialise vecNumMicroSteps
	vec2NumMicroSteps = glm::i32vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::~CPlayer2D(void)
{
	// We won't delete this since it was created elsewhere
	cInventoryManager = NULL;

	// We won't delete this since it was created elsewhere
	cKeyboardController = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CPlayer2D::Init(void)
{
	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	cKeyboardController->Reset();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	itemCollected = flareCollected = cerealCollected = false;

	playerStart = vec2Index;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	 
	// Load the player texture 
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Assets/Player - Copy.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Assets/Player.png" << endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation 
	animatedSprites = CMeshBuilder::GenerateSpriteAnimation(3, 3, cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);
	animatedSprites->AddAnimation("idle_up", 3, 3);
	animatedSprites->AddAnimation("idle_down", 7, 7);
	animatedSprites->AddAnimation("idle_left", 5, 5);
	animatedSprites->AddAnimation("idle_right", 1, 1);
	animatedSprites->AddAnimation("idle", 0, 0);
	animatedSprites->AddAnimation("right", 2, 2);
	animatedSprites->AddAnimation("left", 4, 5);
	animatedSprites->AddAnimation("up", 2, 3);
	animatedSprites->AddAnimation("down", 6, 7);
	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("right", 0, 0.2f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// Get the handler to the CInventoryManager instance
	cInventoryManager = CInventoryManager::GetInstance();
	// Add a Lives icon as one of the inventory items 
	cInventoryItem = cInventoryManager->Add("Lives", "Image/Scene2D_Lives.tga", 1, 1);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	//Add a Health icon as one of the inventory items 
	//cInventoryItem = cInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	//cInventoryItem->vec2Size = glm::vec2(25, 25);

	iJumpCount = 0;

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();

	return true;
}

/**
 @brief Reset this instance
 */
bool CPlayer2D::Reset()
{
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::i32vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	vec2NumMicroSteps = glm::i32vec2(0, 0);

	//Set it to fall upon entering new level
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	//CS: Reset double jump
	iJumpCount = 0;

	//CS: Play the "idle" animation as default
	animatedSprites->PlayAnimation("right", -1, 0.2f);

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);

	return true;
}

/**
 @brief Update this instance
 */
void CPlayer2D::Update(const double dElapsedTime)
{
	// Store the old position
	vec2OldIndex = vec2Index;

	if (cSettings->MuteAudio == false)
	{
		//Play a sound for jump
		cSoundController->PlaySoundByID(4);
	}

	// Get keyboard updates
	if (cKeyboardController->IsKeyDown(GLFW_KEY_W) ||
		cKeyboardController->IsKeyDown(GLFW_KEY_A) ||
		cKeyboardController->IsKeyDown(GLFW_KEY_S) ||
		cKeyboardController->IsKeyDown(GLFW_KEY_D))
	{
		if (cKeyboardController->IsKeyDown(GLFW_KEY_A))
		{
			// Calculate the new position to the left
			if (vec2Index.x >= 0)
			{
				vec2NumMicroSteps.x -= playerSpeed;
				if (vec2NumMicroSteps.x < 0)
				{
					vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
					vec2Index.x--;
				}
			}

			// Constraint the player's position within the screen boundary
			Constraint(LEFT);

			// If the new position is not feasible, then revert to old position
			if (CheckPosition(LEFT) == false)
			{
				vec2Index = vec2OldIndex;
				vec2NumMicroSteps.x = 0;
			}


			//CS: Play the "left" animation
			animatedSprites->PlayAnimation("right", -1, 0.2f);
			dirFacing = 0;

			//CS: Change Color
			runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
		}
		else if (cKeyboardController->IsKeyDown(GLFW_KEY_D))
		{
			// Calculate the new position to the right
			if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS)
			{
				vec2NumMicroSteps.x += playerSpeed;

				if (vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
				{
					vec2NumMicroSteps.x = 0;
					vec2Index.x++;
				}
			}

			// Constraint the player's position within the screen boundary
			Constraint(RIGHT);

			// If the new position is not feasible, then revert to old position
			if (CheckPosition(RIGHT) == false)
			{
				vec2NumMicroSteps.x = 0;
			}

			//CS: Play the "right" animation
			animatedSprites->PlayAnimation("right", -1, 0.2f);
			dirFacing = 1;

			//CS: Change Color
			runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
		}
		else if (cKeyboardController->IsKeyDown(GLFW_KEY_W))
		{
			// Calculate the new position up
			if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS)
			{
				vec2NumMicroSteps.y += playerSpeed
					;
				if (vec2NumMicroSteps.y > cSettings->NUM_STEPS_PER_TILE_YAXIS)
				{
					vec2NumMicroSteps.y = 0;
					vec2Index.y++;
				}
			}

			// Constraint the player's position within the screen boundary
			Constraint(UP);

			// If the new position is not feasible, then revert to old position
			if (CheckPosition(UP) == false)
			{
				vec2NumMicroSteps.y = 0;
			}

			//CS: Play the "idle" animation
			animatedSprites->PlayAnimation("right", -1, 0.2f);
			dirFacing = 2;

			//CS: Change Color
			runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
		}
		else if (cKeyboardController->IsKeyDown(GLFW_KEY_S))
		{
			// Calculate the new position down
			if (vec2Index.y >= 0)
			{
				vec2NumMicroSteps.y -= playerSpeed;
				if (vec2NumMicroSteps.y < 0)
				{
					vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
					vec2Index.y--;
				}
			}

			// Constraint the player's position within the screen boundary
			Constraint(DOWN);

			// If the new position is not feasible, then revert to old position
			if (CheckPosition(DOWN) == false)
			{
				vec2Index = vec2OldIndex;
				vec2NumMicroSteps.y = 0;
			}

			//CS: Play the "idle" animation
			animatedSprites->PlayAnimation("right", -1, 0.2f);
			dirFacing = 3;

			//CS: Change Color
			runtimeColour = glm::vec4(1.0, 1.0, 1.0, 1.0);
		}
	}

	else
	{
		switch (dirFacing)
		{
		case 0:
			animatedSprites->PlayAnimation("idle_left", -1, 1.0f);
			break;
		case 1:
			animatedSprites->PlayAnimation("idle_right", -1, 1.0f);
			break;
		case 2:
			animatedSprites->PlayAnimation("idle_up", -1, 1.0f);
			break;
		case 3:
			animatedSprites->PlayAnimation("idle_down", -1, 1.0f);
			break;
		default:
			break;
		}
	}

	if (cKeyboardController->IsKeyDown(GLFW_KEY_F)) {
		if (flareCollected)
		{
			cInventoryItem = cInventoryManager->GetItem("Item");
			if (cInventoryItem->GetCount() > 0 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 0)
			{
				cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 3);
				flareIndex = vec2Index;
				cInventoryItem->Remove(1);
				itemCollected = false;
				flareCollected = false;
				CGUI_Scene2D::GetInstance()->SetItemType(0);
			}
		}
	}

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_C)) {
		if (cerealCollected)
		{
			cInventoryItem = cInventoryManager->GetItem("Item");
			if (cInventoryItem->GetCount() > 0 && cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 0)
			{
				cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 5);
				cInventoryItem->Remove(1);
				itemCollected = false;
				cerealCollected = false;
				CGUI_Scene2D::GetInstance()->SetItemType(0);
			}
		}
		flareCollected = false;
	}

	if (cKeyboardController->IsKeyPressed(GLFW_KEY_E)) 
	{
		cInventoryItem = cInventoryManager->GetItem("Whistle");

		if (cInventoryItem->GetCount() > 0)
		{
			CPet2D::GetInstance()->HiHzWhistle();
			cInventoryItem->Remove(1);
		}
	}

	// Update Jump or Fall
	//CS: Will cause error when debugging. Set to default elapsed time
	//UpdateJumpFall(dElapsedTime);

	// Interact with the Map
	InteractWithMap();

	//Update Health Lives
	UpdateHealthLives();

	//CS: Update the animated sprite
	animatedSprites->Update(dElapsedTime);

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CPlayer2D::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CPlayer2D::Render(void)
{
	glBindVertexArray(VAO);
	// get matrix's uniform location and set matrix
	unsigned int transformLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "transform");
	unsigned int colorLoc = glGetUniformLocation(CShaderManager::GetInstance()->activeShader->ID, "runtimeColour");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	transform = glm::translate(transform, glm::vec3(vec2UVCoordinate.x,
		vec2UVCoordinate.y,
		0.0f));
	// Update the shaders with the latest transform
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform4fv(colorLoc, 1, glm::value_ptr(runtimeColour));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	//CS: Render the animated sprite
	glBindVertexArray(VAO);
	animatedSprites->Render();
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CPlayer2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

/**
 @brief Constraint the player's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CPlayer2D::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x < 0)
		{
			vec2Index.x = 0;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y < 0)
		{
			vec2Index.y = 0;
			vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CPlayer2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CPlayer2D::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.x >= cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2NumMicroSteps.x = 0;
			return true;
		}

		// If the new position is fully within a row, then check this row only
		if (vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 rows, then check both rows as well
		else if (vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}

	}
	else if (eDirection == UP)
	{
		// If the new position is at the top row, then return true
		if (vec2Index.y >= cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2NumMicroSteps.y = 0;
			return true;
		}

		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == DOWN)
	{
		// If the new position is fully within a column, then check this column only
		if (vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}
	}
	else
	{
		cout << "CPlayer2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

void CPlayer2D::UpdateHealthLives(void)
{
	// Update Health and Lives
	cInventoryItem = cInventoryManager->GetItem("Lives");


	if (cInventoryItem->GetCount() < 1)
	{
		// Check if all life is lost
		if (cInventoryItem->GetCount() <= 0)
		{
			//Player loses the game
			CGameManager::GetInstance()->bPlayerLost = true;

			if (cSettings->MuteAudio == false)
			{
				//Play Sound
				cSoundController->StopPlayByID(4);
			}
		}
	}
}

/**
 @brief Let player interact with the map. You can add collectibles such as powerups and health here.
 */
void CPlayer2D::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 2:
		if (!itemCollected)
		{
			// Erase the flare from this position
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Item");
			cInventoryItem->Add(1);
			flareCollected = true;
			itemCollected = true;
			CGUI_Scene2D::GetInstance()->SetItemType(1);
			if (cSettings->MuteAudio == false)
			{
				cSoundController->StopPlayByID(1);
				// Play a bell sound 
				cSoundController->PlaySoundByID(1);
			}
		}
		break;
	case 4:
		if (!itemCollected)
		{
			// Erase the cereal from this position
			cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
			cInventoryItem = cInventoryManager->GetItem("Item");
			cInventoryItem->Add(1);
			cerealCollected = true;
			itemCollected = true;
			CGUI_Scene2D::GetInstance()->SetItemType(2);
			if (cSettings->MuteAudio == false)
			{
				cSoundController->StopPlayByID(1);
				// Play a bell sound 
				cSoundController->PlaySoundByID(1);
			}
		}
		break;
	case 10:
		// Increase the lives by 1
		cInventoryItem = cInventoryManager->GetItem("Lives");
		cInventoryItem->Add(1);
		// Erase the life from this position
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
		break;
	case 20:
		// Decrease health by 1 (Spikes)
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Remove(1);
		break;
	case 21:
		// Increase health by 1 (Spa)
		cInventoryItem = cInventoryManager->GetItem("Health");
		cInventoryItem->Add(1);
		break;
	default:
		break;
	}
}
