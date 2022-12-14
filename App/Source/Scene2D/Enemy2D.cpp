/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Enemy2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include math.h
#include <math.h>

// Include soundcontroller
#include "../SoundController/SoundController.h"

 // Include CGameStateManager
#include "../GameStateManagement/GameStateManager.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CEnemy2D::CEnemy2D(void)
	: bIsActive(false)
	, cMap2D(NULL)
	, cSettings(NULL)
	, cPlayer2D(NULL)
	, sCurrentFSM(FSM::IDLE)
	, iFSMCounter(0)
	, quadMesh(NULL)
{
	transform = glm::mat4(1.0f);	// make sure to initialize matrix to identity matrix first

	// Initialise vecIndex
	vec2Index = glm::vec2(0);

	// Initialise vecNumMicroSteps
	i32vec2NumMicroSteps = glm::vec2(0);

	// Initialise vec2UVCoordinate
	vec2UVCoordinate = glm::vec2(0.0f);

	i32vec2Destination = glm::vec2(0, 0);	// Initialise the iDestination
	i32vec2Direction = glm::vec2(0, 0);		// Initialise the iDirection
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CEnemy2D::~CEnemy2D(void)
{
	// Delete the quadMesh
	if (quadMesh)
	{
		delete quadMesh;
		quadMesh = NULL;
	}

	// We won't delete this since it was created elsewhere
	cPlayer2D = NULL;

	// We won't delete this since it was created elsewhere
	cMap2D = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

/**
  @brief Initialise this instance
  */
bool CEnemy2D::Init(void)
{
	// Keyboard Controller singleton instance
	cKeyboardController = CKeyboardController::GetInstance();

	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	cSoundController = CSoundController::GetInstance();

	cPet2D = CPet2D::GetInstance();

	// Get the handler to the CMap2D instance
	cMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to cPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (cMap2D->FindValue(300, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	cMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Index = glm::vec2(uiCol, uiRow);
	// By default, microsteps should be zero
	i32vec2NumMicroSteps = glm::vec2(0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Create the Quad Mesh using the mesh builder
	quadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), cSettings->TILE_WIDTH, cSettings->TILE_HEIGHT);

	// Load the enemy2D texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Assets/Demon.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Assets/Demon.tga" << endl;
		return false;
	}

	//CS: Init the color to white
	runtimeColour = glm::vec4(1.0, 0.0, .0, 1.0);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetStatus(CPhysics2D::STATUS::FALL);

	// Get the handler to the CInventoryManager instance
	cInventoryManager = CInventoryManager::GetInstance();

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	return true;
}

/**
 @brief Update this instance
 */
void CEnemy2D::Update(const double dElapsedTime)
{
	v1 = rand() % 100;
	cout << v1 << endl;
	if (sCurrentFSM != FLAREFLLW)
	{
		unsigned int uiRow = -1;
		unsigned int uiCol = -1;
		if (cMap2D->FindValue(3, uiRow, uiCol)) {
			sCurrentFSM = FLAREFLLW;
			flareOldIndex = cPlayer2D->flareIndex;
			//cout << "Fllwing Flare :3" << endl;
		}
	}
	
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_SPACE))
	{
		// Ignore whistle if flare is up
		if (sCurrentFSM != FLAREFLLW)
		{
			//Find player last location when they use the whistle
			playerLast = cPlayer2D->vec2Index;
			path = cMap2D->PathFind(vec2Index,
				playerLast,
				heuristic::manhattan,
				10);
			iFSMCounter = 0;
			sCurrentFSM = HUNTING;
			cMap2D->PrintSelf();
		}
	}

	if (!bIsActive)
		return;

	switch (sCurrentFSM)
	{
	case IDLE:
		
		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = PATROL;
			if (v1 == 99) {
				cSoundController->PlaySoundByID(6);
			}
			iFSMCounter = 0;			;
		}

		iFSMCounter++;
		break;
	case PATROL:
		if (v1 == 3) {
			cSoundController->PlaySoundByID(7);
		}

		if (iFSMCounter > iMaxFSMCounter)
		{
			sCurrentFSM = IDLE;
			iFSMCounter = 0;
			break;
		}

		//If player is close to the enemy
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 7.0f ||
			cPhysics2D.CalculateDistance(vec2Index, cPet2D->vec2Index) < 7.0f)
		{
			sCurrentFSM = CHASE;
			iFSMCounter = 0;
			break;
		}
		else
		{
			// Patrol around
			// Update the Enemy2D's position for patrol
			UpdatePosition();
			cout << "DEMON PATROL" << endl;
		}
		iFSMCounter++;
		break;
	case CHASE:
		//If Player & Pet is within Range OR Player is within range, chase the player
		if ((cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 7.0f &&
			cPhysics2D.CalculateDistance(vec2Index, cPet2D->vec2Index) < 7.0f) || 
			cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 7.0f)
		{
			cout << "DEMON CHASE: PLAYER" << endl;

			path = cMap2D->PathFind(vec2Index,
				cPlayer2D->vec2Index,
				heuristic::euclidean,
				10);

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				if (bFirstPosition == true)
				{
					// Set a destination
					i32vec2Destination = coord;
					// Calculate the direction between enemy2D and player2D
					i32vec2Direction = i32vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - i32vec2Destination) == i32vec2Direction)
					{
						// Set a destination
						i32vec2Destination = coord;
					}
					else 
					{
						break;
					}
				}
			}

			// Update the Enemy2D's position for attack
			UpdatePosition();

			cSoundController->PlaySoundByID(5);
		}
		//If pet is within range but nothing else is, chase pet
		else if (cPhysics2D.CalculateDistance(vec2Index, cPet2D->vec2Index) < 7.0f)
		{
			cout << "DEMON CHASE: PLAYER" << endl;

			path = cMap2D->PathFind(vec2Index,
				cPet2D->vec2Index,
				heuristic::euclidean,
				10);

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				if (bFirstPosition == true)
				{
					// Set a destination
					i32vec2Destination = coord;
					// Calculate the direction between enemy2D and player2D
					i32vec2Direction = i32vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - i32vec2Destination) == i32vec2Direction)
					{
						// Set a destination
						i32vec2Destination = coord;
					}
					else
					{
						break;
					}
				}
			}

			// Update the Enemy2D's position for attack
			UpdatePosition();

			cSoundController->PlaySoundByID(5);
		}
		else
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cout << "CHASE : Reset counter: " << iFSMCounter << endl;
			break;
		}

	case HUNTING:
		cout << "DEMON HUNTING" << endl;

		// follow last player location
		if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) > 7.0f)
		{
			path = cMap2D->PathFind(vec2Index,
				playerLast,
				heuristic::euclidean,
				10);

			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				//std::cout << coord.x << "," << coord.y << "\n";

				if (bFirstPosition == true)
				{
					// Set a destination
					i32vec2Destination = coord;
					// Calculate the direction between enemy2D and player2D
					i32vec2Direction = i32vec2Destination - vec2Index;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - i32vec2Destination) == i32vec2Direction)
					{
						// Set a destination
						i32vec2Destination = coord;
					}
					else
					{
						break;
					}
				}
			}

			UpdatePosition();
		}
		
		// if the player is spotted, switch to chase
		else if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->vec2Index) < 7.0f ||
			cPhysics2D.CalculateDistance(vec2Index, cPet2D->vec2Index) < 7.0f)
		{
			sCurrentFSM = CHASE;
			iFSMCounter = 0;
			cout << "Switching to Enemy::CHASE State" << endl;
			break;
		}

	case STUNNED:
		if (iFSMCounter > stunnedCounter)
		{
			sCurrentFSM = PATROL;
			iFSMCounter = 0;
			cSoundController->PlaySoundByID(8);
			cout << "Switching to Enemy::PATROL State" << endl;

		}
		iFSMCounter++;
		break;
	case FLAREFLLW:
		if (iFSMCounter > fllwCounter)
		{
			cMap2D->SetMapInfo(cPlayer2D->flareIndex.y, cPlayer2D->flareIndex.x, 0);
			sCurrentFSM = PATROL;
			iFSMCounter = 0;

			cout << "Switching to Enemy::PATROL State" << endl;
		}
		else if (cPhysics2D.CalculateDistance(vec2Index, cPlayer2D->flareIndex) > 1.0f)
		{
			path = cMap2D->PathFind(vec2Index,
				flareOldIndex,
				heuristic::euclidean,
				10);

			// Calculate new destination
			bool bFirstPosition = true;
			for (const auto& coord : path)
			{
				if (bFirstPosition == true)
				{
					// Set a destination
					flareOldIndex = coord;
					bFirstPosition = false;
				}
				else
				{
					if ((coord - flareOldIndex) == i32vec2Direction)
					{
						// Set a destination
						flareOldIndex = coord;
					}
					else
						break;
				}
			}
			UpdatePosition();
		}

		iFSMCounter++;
		cout << iFSMCounter << endl;
		break;
	default:
		break;
	}

	InteractWithMap();

	// Update the UV Coordinates
	vec2UVCoordinate.x = cSettings->ConvertIndexToUVSpace(cSettings->x, vec2Index.x, false, i32vec2NumMicroSteps.x * cSettings->MICRO_STEP_XAXIS);
	vec2UVCoordinate.y = cSettings->ConvertIndexToUVSpace(cSettings->y, vec2Index.y, false, i32vec2NumMicroSteps.y * cSettings->MICRO_STEP_YAXIS);
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEnemy2D::PreRender(void)
{
	if (!bIsActive)
		return;

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CEnemy2D::Render(void)
{
	if (!bIsActive)
		return;

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

	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);

	// Render the tile
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	quadMesh->Render();

	glBindVertexArray(0);

}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CEnemy2D::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
@brief Set the indices of the enemy2D
@param iIndex_XAxis A const int variable which stores the index in the x-axis
@param iIndex_YAxis A const int variable which stores the index in the y-axis
*/
void CEnemy2D::Seti32vec2Index(const int iIndex_XAxis, const int iIndex_YAxis)
{
	this->vec2Index.x = iIndex_XAxis;
	this->vec2Index.y = iIndex_YAxis;
}

/**
@brief Set the number of microsteps of the enemy2D
@param iNumMicroSteps_XAxis A const int variable storing the current microsteps in the X-axis
@param iNumMicroSteps_YAxis A const int variable storing the current microsteps in the Y-axis
*/
void CEnemy2D::Seti32vec2NumMicroSteps(const int iNumMicroSteps_XAxis, const int iNumMicroSteps_YAxis)
{
	this->i32vec2NumMicroSteps.x = iNumMicroSteps_XAxis;
	this->i32vec2NumMicroSteps.y = iNumMicroSteps_YAxis;
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param cPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void CEnemy2D::SetPlayer2D(CPlayer2D* cPlayer2D)
{
	this->cPlayer2D = cPlayer2D;

	// Update the enemy's direction
	//UpdateDirection();
}


/**
 @brief Constraint the enemy2D's position within a boundary
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
void CEnemy2D::Constraint(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		if (vec2Index.x <= 0)
		{
			vec2Index.x = 0;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == RIGHT)
	{
		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			vec2Index.x = ((int)cSettings->NUM_TILES_XAXIS) - 1;
			i32vec2NumMicroSteps.x = 0;
		}
	}
	else if (eDirection == UP)
	{
		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			vec2Index.y = ((int)cSettings->NUM_TILES_YAXIS) - 1;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else if (eDirection == DOWN)
	{
		if (vec2Index.y <= 0)
		{
			vec2Index.y = 0;
			i32vec2NumMicroSteps.y = 0;
		}
	}
	else
	{
		cout << "CEnemy2D::Constraint: Unknown direction." << endl;
	}
}

/**
 @brief Check if a position is possible to move into
 @param eDirection A DIRECTION enumerated data type which indicates the direction to check
 */
bool CEnemy2D::CheckPosition(DIRECTION eDirection)
{
	if (eDirection == LEFT)
	{
		cout << "direction: LEFT" << endl;
		//cout << "current location valid?: " << (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) == 100) << endl;

		if (vec2Index.x <= 0)
		{
			cout << "FALSE LEFT" << endl;
			return false;
		}

		// If the new position is fully within a row, then check this row only
		else if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}

		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x - 1) >= 100))
			{
				return false;
			}
		}
	}
	else if (eDirection == RIGHT)
	{
		cout << "direction: RIGHT" << endl;

		if (vec2Index.x >= (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			cout << "FALSE RIGHT" << endl;
			return false;
		}

		// If the new position is fully within a row, then check this row only
		else if (i32vec2NumMicroSteps.y == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}

		}
		// If the new position is between 2 rows, then check both rows as well
		else if (i32vec2NumMicroSteps.y != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y, vec2Index.x + 1) >= 100))
			{
				return false;
			}
		}

	}
	else if (eDirection == UP)
	{
		cout << "direction: UP" << endl;

		if (vec2Index.y >= (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			cout << "FALSE UP" << endl;
			return false;
		}

		// If the new position is fully within a column, then check this column only
		else if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}
		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			// If the 2 grids are not accessible, then return false
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y + 1, vec2Index.x) >= 100))
			{
				return false;
			}
		}

	}
	else if (eDirection == DOWN)
	{
		cout << "direction: DOWN" << endl;

		if (vec2Index.y <= 0)
		{
			cout << "FALSE DOWN" << endl;
			return false;
		}

		// If the new position is fully within a column, then check this column only
		else if (i32vec2NumMicroSteps.x == 0)
		{
			// If the grid is not accessible, then return false
			if (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100)
			{
				return false;
			}
		}

		// If the new position is between 2 columns, then check both columns as well
		else if (i32vec2NumMicroSteps.x != 0)
		{
			if ((cMap2D->GetMapInfo(vec2Index.y, vec2Index.x) >= 100) ||
				(cMap2D->GetMapInfo(vec2Index.y - 1, vec2Index.x) >= 100))
			{
				return false;
			}
		}

	}
	else
	{
		cout << "CEnemy2D::CheckPosition: Unknown direction." << endl;
	}

	return true;
}

/**
 @brief Let enemy2D interact with the Player
 */
bool CEnemy2D::InteractWithPlayer(void)
{
	glm::vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;

	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= i32vec2PlayerPos.x - 0.5) &&
		(vec2Index.x <= i32vec2PlayerPos.x + 0.5))
		&&
		((vec2Index.y >= i32vec2PlayerPos.y - 0.5) &&
			(vec2Index.y <= i32vec2PlayerPos.y + 0.5)))
	{

		//Jumpscare

		// Reset the CKeyboardController
		CKeyboardController::GetInstance()->Reset();
		cout << "Loading Jumpscare" << endl;
		CSoundController::GetInstance()->PlaySoundByID(10);
		CGameStateManager::GetInstance()->SetActiveGameState("JumpscareState");

		return true;
	}

	return false;
}

/**
 @brief Let enemy2D interact with the Pet
 */
bool CEnemy2D::InteractWithPet(void)
{
	glm::vec2 i32vec2PlayerPos = cPlayer2D->vec2Index;

	// Check if the enemy2D is within 1.5 indices of the player2D
	if (((vec2Index.x >= cPet2D->vec2Index.x - 0.5) &&
		(vec2Index.x <= cPet2D->vec2Index.x + 0.5))
		&&
		((vec2Index.y >= cPet2D->vec2Index.y - 0.5) &&
			(vec2Index.y <= cPet2D->vec2Index.y + 0.5)))
	{
		//cout << "Gotcha!" << endl;
		// Remove health by 1 (Spa)
		CInventoryItem* cInventoryItem = cInventoryManager->GetItem("Lives");
		cInventoryItem->Remove(1);

		// Since the player has been caught, then reset the FSM
		sCurrentFSM = IDLE;
		iFSMCounter = 0;

		if (cSettings->MuteAudio == false)
		{
			//Play Sound
			CSoundController::GetInstance()->PlaySoundByID(2);
		}

		return true;
	}

	return false;
}

/**
 @brief Update the enemy's direction.
 */
void CEnemy2D::UpdateDirection(void)
{
	// Set the destination to the player
	i32vec2Destination = cPlayer2D->vec2Index;

	// Calculate the direction between enemy2D and player2D
	i32vec2Direction = i32vec2Destination - i32vec2Index;

	// Calculate the distance between enemy2D and player2D
	float fDistance = cPhysics2D.CalculateDistance(vec2Index, i32vec2Destination);
	if (fDistance >= 0.05f)
	{
		// Calculate direction vector.
		// We need to round the numbers as it is easier to work with whole numbers for movements
		i32vec2Direction.x = (int)round(i32vec2Direction.x / fDistance);
		i32vec2Direction.y = (int)round(i32vec2Direction.y / fDistance);
	}
	else
	{
		// Since we are not going anywhere, set this to 0.
		i32vec2Direction = glm::vec2(0);
	}
}

/**x
 @brief Flip horizontal direction. For patrol use only
 */
void CEnemy2D::FlipHorizontalDirection(void)
{
	i32vec2Direction.x *= -1;
}

void CEnemy2D::FlipVerticalDirection(void)
{
	i32vec2Direction.y *= -1;
}

void CEnemy2D::FlipRandomDirection(void)
{
	//Reset Direction
	i32vec2Direction.x = 0;
	i32vec2Direction.y = 0;

	int temp = rand() % 4;

	if (temp == UP)
	{
		cout << "CheckPosition(UP): " << CheckPosition(UP) << endl;
		i32vec2Direction.y = 1;
	}

	else if (temp == DOWN)
	{
		cout << "CheckPosition(DOWN): " << CheckPosition(DOWN) << endl;
		i32vec2Direction.y = -1;
	}

	else if (temp == LEFT)
	{
		cout << "CheckPosition(LEFT): " << CheckPosition(LEFT) << endl;
		i32vec2Direction.x = -1;
	}

	else if (temp == RIGHT)
	{
		cout << "CheckPosition(RIGHT): " << CheckPosition(RIGHT) << endl;
		i32vec2Direction.x = 1;
	}

	temp = rand() % 4;
	cout << "temp: " << temp << endl;
}

void CEnemy2D::GenerateRandomPoint(void)
{
	int randx;
	int randy;

	while (true)
	{		
		randx = rand() % ((int)cSettings->NUM_TILES_YAXIS - 1) + 1;
		randy = rand() % ((int)cSettings->NUM_TILES_YAXIS - 1) + 1;

		if (cMap2D->GetMapInfo(randy, randx) != 100)
		{
			glm::vec2 temp = glm::vec2(randx, randy);

			//Reset Direction
			i32vec2Direction.x = 0;
			i32vec2Direction.y = 0;

			// Find player last location when hey use the whistle
			path = cMap2D->PathFind(vec2Index,
				temp,
				heuristic::manhattan,
				20);
			cout << "new random patrol point: " << temp.x << " , " << temp.y << endl;
			break;
		}

		cout << "random point not valid" << endl;
	}

}

void CEnemy2D::InteractWithMap(void)
{
	switch (cMap2D->GetMapInfo(vec2Index.y, vec2Index.x))
	{
	case 3: // Flare Dropped
		// Erase the flare from this position
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
		break;
	case 5: // Cereal Dropped
		// Erase the cereal from this position
		cMap2D->SetMapInfo(vec2Index.y, vec2Index.x, 0);
		// Insert Stun Code Here
		sCurrentFSM = STUNNED;
		//cout << "Switching to Enemy::STUNNED State" << endl;

		if (cSettings->MuteAudio == false)
		{
			cSoundController->StopPlayByID(1);
			// Play a bell sound 
			cSoundController->PlaySoundByID(1);
		}
		break;
	default:
		break;
	}
}

/**
@brief Update position.
*/
void CEnemy2D::UpdatePosition(void)
{
	// Store the old position
	i32vec2OldIndex = vec2Index;

	// if the player is to the left or right of the enemy2D, then jump to attack
	if (i32vec2Direction.x < 0)
	{
		// Move left
		const int iOldIndex = vec2Index.x;	
 		if (vec2Index.x > 0)
		{
			i32vec2NumMicroSteps.x -= enemySpeed;
   			if (i32vec2NumMicroSteps.x < 0)
			{
				i32vec2NumMicroSteps.x = ((int)cSettings->NUM_STEPS_PER_TILE_XAXIS) - 1;
				vec2Index.x--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(LEFT);

		// Find a feasible position for the enemy2D's current position
   		if (CheckPosition(LEFT) == false)
		{
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;

			//if patrol state
			if (sCurrentFSM == PATROL)
			{
				FlipRandomDirection();
				//GenerateRandomPoint();
			}
		}

		cout << "CheckPosition(LEFT): " << CheckPosition(LEFT) << endl;

		// Interact with the Player
		InteractWithPlayer();

		// Interact with the Cat
		InteractWithPet();
	}
	else if (i32vec2Direction.x > 0)
	{		
		// Move right
		const int iOldIndex = vec2Index.x;
		if (vec2Index.x < (int)cSettings->NUM_TILES_XAXIS - 1)
		{
			i32vec2NumMicroSteps.x += enemySpeed;
			if (i32vec2NumMicroSteps.x >= cSettings->NUM_STEPS_PER_TILE_XAXIS)
			{
				i32vec2NumMicroSteps.x = 0;
				vec2Index.x++;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(RIGHT);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(RIGHT) == false)
		{
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.x = 0;

			//if patrol state
			if (sCurrentFSM == PATROL)
			{
				FlipRandomDirection();
				//GenerateRandomPoint();
			}
		}

		cout << "CheckPosition(RIGHT): " << CheckPosition(RIGHT) << endl;
		//system("pause");

		// Interact with the Player
		InteractWithPlayer();

		// Interact with the Cat
		InteractWithPet();
	}
	else if (i32vec2Direction.y < 0)
	{
		// Move down
		const int iOldIndex = vec2Index.y;

		if (vec2Index.y >= 0)
		{
			i32vec2NumMicroSteps.y -= enemySpeed;
			if (i32vec2NumMicroSteps.y < 0)
			{
				i32vec2NumMicroSteps.y = ((int)cSettings->NUM_STEPS_PER_TILE_YAXIS) - 1;
				vec2Index.y--;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(DOWN);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(DOWN) == false)
		{
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.y = 0;

			//if patrol state
			if (sCurrentFSM == PATROL)
			{
				FlipRandomDirection();
				//GenerateRandomPoint();
			}
		}

		cout << "CheckPosition(DOWN): " << CheckPosition(DOWN) << endl;

		// Interact with the Player
		InteractWithPlayer();

		// Interact with the Cat
		InteractWithPet();
	}
	else if (i32vec2Direction.y > 0)
	{
		// Move up
		const int iOldIndex = vec2Index.y;
		if (vec2Index.y < (int)cSettings->NUM_TILES_YAXIS - 1)
		{
			i32vec2NumMicroSteps.y += enemySpeed;
			if (i32vec2NumMicroSteps.y >= cSettings->NUM_STEPS_PER_TILE_YAXIS)
			{
				i32vec2NumMicroSteps.y = 0;
				vec2Index.y++;
			}
		}

		// Constraint the enemy2D's position within the screen boundary
		Constraint(UP);

		// Find a feasible position for the enemy2D's current position
		if (CheckPosition(UP) == false)
		{
			vec2Index = i32vec2OldIndex;
			i32vec2NumMicroSteps.y = 0;

			//if patrol state
			if (sCurrentFSM == PATROL)
			{
				FlipRandomDirection();
				//GenerateRandomPoint();
			}
		}
		
		cout << "CheckPosition(UP): " << CheckPosition(UP) << endl;

		// Interact with the Player
		InteractWithPlayer();

		// Interact with the Cat
		InteractWithPet();
	}
}