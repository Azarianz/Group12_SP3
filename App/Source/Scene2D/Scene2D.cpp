/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Scene2D.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScene2D::CScene2D(void)
	: cMap2D(NULL)
	, cPlayer2D(NULL)
	, cKeyboardController(NULL)
	, cGUI_Scene2D(NULL)
	, cGameManager(NULL)
	, cSoundController(NULL)
{
}

/**
 @brief Destructor
 */
CScene2D::~CScene2D(void)
{
	if (cGUI_Scene2D)
	{
		cGUI_Scene2D->Destroy();
		cGUI_Scene2D = NULL;
	}

	if (cKeyboardController)
	{
		// We won't delete this since it was created elsewhere
		cKeyboardController = NULL;
	}

	if (cPlayer2D)
	{
		cPlayer2D->Destroy();
		cPlayer2D = NULL;
	}

	// Destroy the enemies
	if (cEnemy2D)
	{
		cEnemy2D->Destroy();
		cEnemy2D = NULL;
	}

	if (cMap2D)
	{
		cMap2D->Destroy();
		cMap2D = NULL;
	}

	//if (cGameManager)
	//{
	//	cGameManager->Destroy();
	//	cGameManager = NULL;
	//}

	if (cSoundController)
	{
		// We won't delete this since it was created elsewhere
		cSoundController = NULL;
	}
}

/**
@brief Init Initialise this instance
*/
bool CScene2D::Init(void)
{
	// Include Shader Manager
	CShaderManager::GetInstance()->Use("Shader2D");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	// Create and initialise the Map 2D
	cMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	cMap2D->SetShader("Shader2D");
	// Load the map into an array
	// Initialise the instance
	if (cMap2D->Init(2, 30, 40) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}
	// Load the map into an array
	if (cMap2D->LoadMap("Maps/DM2213_Map_Level_01.csv") == false)
	{
		// The loading of a map has failed. Return false
		return false;
	}

	srand(time(NULL));
	RandomSpawns();

	// Load Scene2DColour into ShaderManager
	CShaderManager::GetInstance()->Use("Shader2D_Colour");
	//CShaderManager::GetInstance()->activeShader->setInt("texture1", 0);

	// Create and initialise the CPlayer2D
	cPlayer2D = CPlayer2D::GetInstance();
	// Pass shader to cPlayer2D
	cPlayer2D->SetShader("Shader2D_Colour");
	// Initialise the instance
	if (cPlayer2D->Init() == false)
	{
		cout << "Failed to load CPlayer2D" << endl;
		return false;
	}

	// Create and initialise the CEnemy2D
	cEnemy2D = CEnemy2D::GetInstance();
	// Pass shader to cEnemy2D
	cEnemy2D->SetShader("Shader2D_Colour");
	if (cEnemy2D->Init() == false)
	{
		cout << "Failed to load CEnemy2D" << endl;
		return false;
	}
	cEnemy2D->SetPlayer2D(cPlayer2D);

	// Create and initialise the CPet2D
	cPet2D = CPet2D::GetInstance();
	// Pass shader to cPlayer2D
	cPet2D->SetShader("Shader2D_Colour");
	// Initialise the instance
	if (cPet2D->Init() == false)
	{
		cout << "Failed to load CPet2D" << endl;
		return false;
	}
	cPet2D->SetPlayer2D(cPlayer2D);

	// Store the keyboard controller singleton instance here
	cKeyboardController = CKeyboardController::GetInstance();

	// Store the cGUI_Scene2D singleton instance here
	cGUI_Scene2D = CGUI_Scene2D::GetInstance();
	cGUI_Scene2D->Init();

	// Game Manager
	cGameManager = CGameManager::GetInstance();
	cGameManager->Init();

	// Load the sounds into CSoundController
	cSoundController = CSoundController::GetInstance();
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\coin.ogg"), 1, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\death.ogg"), 2, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\done.ogg"), 3, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\BGM.ogg"), 4, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Ghast attk.ogg"), 5, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Ghast1.ogg"), 6, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\ghast2.ogg"), 7, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\GhastCry.ogg"), 8, true);
	cSoundController->LoadSound(FileSystem::getPath("Sounds\\Whristle.ogg"), 9, true);


	return true;
}

/**
@brief Update Update this instance
*/
bool CScene2D::Update(const double dElapsedTime)
{
	// Call the cPlayer2D's update method before Map2D as we want to capture the inputs before map2D update
	cPlayer2D->Update(dElapsedTime);

	// Call all the cEnemy2D's update method before Map2D
	cEnemy2D->Update(dElapsedTime);

	// Call all the cPet2D's updated method
	cPet2D->Update(dElapsedTime);

	// Call the Map2D's update method
	cMap2D->Update(dElapsedTime);

	// Get keyboard updates
	if (cKeyboardController->IsKeyReleased(GLFW_KEY_F6))
	{
		// Save the current game to a save file
		// Make sure the file is open
		try {
			if (cMap2D->SaveMap("Maps/DM2213_Map_Level_01_SAVEGAMEtest.csv") == false)
			{
				throw runtime_error("Unable to save the current game to a file");
			}
		}
		catch (runtime_error e)
		{
			cout << "Runtime error: " << e.what();
			return false;
		}
	}

	//Check if the game should go to the next level
	if (cGameManager->bLevelCompleted == true)
	{
		cMap2D->SetCurrentLevel(cMap2D->GetCurrentLevel() + 1);
		cPlayer2D->Reset();
		cGameManager->bLevelCompleted = false;
	}

	//Check if the game has been won by the player
	if (cGameManager->bPlayerWon == true)
	{
		//End the game and switch to Win screen		
	}

	//Check if the game should be ended
	if (cGameManager->bPlayerLost == true)
	{
		//cSoundController->PlaySoundByID(2);
		//return false;
	}

	// Call the cGUI_Scene2D's update method
	cGUI_Scene2D->Update(dElapsedTime);

	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CScene2D::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);
}

/**
 @brief Render Render this instance
 */
void CScene2D::Render(void)
{
	// Call the CEnemy2D's PreRender()
	cEnemy2D->PreRender();
	// Call the CEnemy2D's Render()
	cEnemy2D->Render();
	// Call the CEnemy2D's PostRender()
	cEnemy2D->PostRender();

	// Call the CPet2D's PreRender()
	cPet2D->PreRender();
	// Call the CPet2D's Render()
	cPet2D->Render();
	// Call the CPet2D's PostRender()
	cPet2D->PostRender();

	// Call the CPlayer2D's PreRender()
	cPlayer2D->PreRender();
	// Call the CPlayer2D's Render()
	cPlayer2D->Render();
	// Call the CPlayer2D's PostRender()
	cPlayer2D->PostRender();

	// Call the Map2D's PreRender()
	cMap2D->PreRender();
	// Call the Map2D's Render()
	cMap2D->Render();
	// Call the Map2D's PostRender()
	cMap2D->PostRender();

	// Call the cGUI_Scene2D's PreRender()
	cGUI_Scene2D->PreRender();
	// Call the cGUI_Scene2D's Render()
	cGUI_Scene2D->Render();
	// Call the cGUI_Scene2D's PostRender()
	cGUI_Scene2D->PostRender();
}


void CScene2D::RandomSpawns(void)
{
	unsigned int PlayerSpawn;	// 4 different locations
	unsigned int WinZone;	// 4 different locations
	unsigned int DogSpawn;	// 6 different locations
	unsigned int FlareSpawn;	// 2 different locations
	//unsigned int HiFreqWhistleSpawn;	// 2 different locations
	unsigned int CerealSpawn[2] = {};	// 4 different locations


	// set player's spawn at 1 of 4 random locations
	PlayerSpawn = rand() % 4;

	// set win-zone at 1 of 4 random locations
	WinZone = rand() % 4;
	// if the win-zone and player spawn are in similar
	// location, randomise again until they are diff
	while (WinZone == PlayerSpawn)
	{
		WinZone = rand() % 4;
		//cout << "winzone changed" << endl;
	}

	// set dog's spawn at 1 of 4 random locations
	DogSpawn = rand() % 6;
	// make sure the dog's spawn does not overlap
	// with the other 2 spawns
	while ((DogSpawn == PlayerSpawn) || (DogSpawn == WinZone))
	{
		DogSpawn = rand() % 4;
		//cout << "dog changed" << endl;
	}

	// set flare's spawn at 1 of 2 random locations
	FlareSpawn = rand() % 2;

	// set hi-freq whistle's spawn at 1 of 2 random locations
	//HiFreqWhistle = rand() % 2;

	CerealSpawn[0] = rand() % 4;
	CerealSpawn[1] = rand() % 4;
	while (CerealSpawn[0] == CerealSpawn[1])
	{
		CerealSpawn[1] = rand() % 4;
		//cout << "CS1 changed" << endl;
	}

	// Spawn player
	switch (PlayerSpawn)
	{
	case 0:
		cMap2D->SetMapInfo(26, 4, 200);
		break;
	case 1:
		cMap2D->SetMapInfo(4, 4, 200);
		break;
	case 2:
		cMap2D->SetMapInfo(4, 36, 200);
		break;
	case 3:
		cMap2D->SetMapInfo(26, 36, 200);
		break;
	default:
		cout << "Player is not spawned" << endl;
		break;
	}

	// Spawn win-zone
	switch (WinZone)
	{
	case 0:
		cMap2D->SetMapInfo(23, 13, 99);
		cMap2D->SetMapInfo(22, 13, 99);
		cMap2D->SetMapInfo(22, 12, 99);
		cMap2D->SetMapInfo(23, 12, 99);
		break;
	case 1:
		cMap2D->SetMapInfo(7, 4, 99);
		cMap2D->SetMapInfo(6, 4, 99);
		cMap2D->SetMapInfo(6, 3, 99);
		cMap2D->SetMapInfo(7, 3, 99);
		break;
	case 2:
		cMap2D->SetMapInfo(4, 30, 99);
		cMap2D->SetMapInfo(3, 30, 99);
		cMap2D->SetMapInfo(3, 29, 99);
		cMap2D->SetMapInfo(4, 29, 99);
		break;
	case 3:
		cMap2D->SetMapInfo(19, 33, 99);
		cMap2D->SetMapInfo(20, 33, 99);
		cMap2D->SetMapInfo(20, 32, 99);
		cMap2D->SetMapInfo(19, 32, 99);
		break;
	default:
		cout << "Win-Zone is not spawned" << endl;
		break;
	}

	// Spawn dog
	switch (DogSpawn)
	{
	case 0:
		cMap2D->SetMapInfo(29, 13, 400);
		break;
	case 1:
		cMap2D->SetMapInfo(7, 13, 400);
		break;
	case 2:
		cMap2D->SetMapInfo(1, 28, 400);
		break;
	case 3:
		cMap2D->SetMapInfo(26, 28, 400);
		break;
	case 4:
		cMap2D->SetMapInfo(18, 30, 400);
		break;
	case 5:
		cMap2D->SetMapInfo(17, 1, 400);
		break;
	default:
		cout << "Player is not spawned" << endl;
		break;
	}

	// Spawn flare
	switch (FlareSpawn)
	{
	case 0:
		cMap2D->SetMapInfo(26, 18, 2);
		break;
	case 1:
		cMap2D->SetMapInfo(3, 21, 2);
		break;
	default:
		cout << "Flare is not spawned" << endl;
		break;
	}

	// Spawn cereal 1
	switch (CerealSpawn[0])
	{
	case 0:
		cMap2D->SetMapInfo(19, 12, 4);
		break;
	case 1:
		cMap2D->SetMapInfo(10, 8, 4);
		break;
	case 2:
		cMap2D->SetMapInfo(17, 30, 4);
		break;
	case 3:
		cMap2D->SetMapInfo(9, 26, 4);
		break;
	default:
		cout << "Cereal1 is not spawned" << endl;
		break;
	}

	// Spawn cereal 2
	switch (CerealSpawn[1])
	{
	case 0:
		cMap2D->SetMapInfo(19, 12, 4);
		break;
	case 1:
		cMap2D->SetMapInfo(10, 8, 4);
		break;
	case 2:
		cMap2D->SetMapInfo(17, 30, 4);
		break;
	case 3:
		cMap2D->SetMapInfo(9, 26, 4);
		break;
	default:
		cout << "Cereal2 is not spawned" << endl;
		break;
	}

	/*cout << endl
		<< "PS: " << PlayerSpawn << endl
		<< "WZ: " << WinZone << endl
		<< "DS: " << DogSpawn << endl
		<< "FS: " << FlareSpawn << endl
		<< "CS0: " << CerealSpawn[0] << endl
		<< "CS1: " << CerealSpawn[1] << endl
		<< endl;*/

	return;
}


/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CScene2D::PostRender(void)
{
}