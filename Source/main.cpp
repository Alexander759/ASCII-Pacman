#include <iostream>
#include <fstream>
#include <windows.h>

using namespace std;

#pragma region GameDataStructures

enum GameMode {
	ChaseMode,
	FrightenedMode
};

struct VectorMovement;
struct Coordinates;
struct Map;
struct Pacman;
struct Ghost;
struct Game;

struct VectorMovement {
	int x;
	int y;
};

struct Coordinates {
	int x;
	int y;
};

struct Map {
	char** content;
	int verticalSize;
	int horizontalSize;
};

struct Ghost {
	Coordinates position;
	Coordinates reappearCoordinates;
	Coordinates target;
	void (*setTarget) (Game& game);
	const VectorMovement* currentVector;

	int chaseModeColorCode;
	char symbol;
	char previousSymbolOnCurrentPosition;
};

struct Pacman {
	Coordinates position;
	const VectorMovement* currentDirection;
	int pacmanColor;
	char symbol;
};

struct Game {
	Ghost blinky;
	Ghost pinky;
	Ghost inky;
	Ghost clyde;
	Pacman pacman;
	Map map;
	GameMode currentGameMode;

	int foodCount;
	int leftInFrightenedMode;
	int score;
};

#pragma endregion

#pragma region Constants

const int MAZEMAXSIZE = 50;

const char PACMANSYMBOL = 'Y';
const int PACMANCOLORCODE = 14;

const char FOODSYMBOL = '-';
const int FOODCOLORCODE = 14;

const char PELLETSYMBOL = '@';
const int PELLETCOLOR = 14;

const char FREESPACESYMBOL = ' ';

const int NUMBERSOFSYMBOLSFORFOOD = 2;
char SYMBOLSFORFOOD[NUMBERSOFSYMBOLSFORFOOD] = { FOODSYMBOL, PELLETSYMBOL };

char MAPFILENAME[] = "map.txt";

char UNABLETOOPENFILEMESSAGE[] = "Couldn't open file!";
char FILETOOSHORTMESSAGE[] = "File shorter than expected!";

const int VECTORSLENGTH = 5;

const VectorMovement ZEROVECTOR = { 0,0 };
const VectorMovement UPVECTOR = { 0,-1 };
const VectorMovement LEFTVECTOR = { -1,0 };
const VectorMovement DOWNVECTOR = { 0,1 };
const VectorMovement RIGHTVECTOR = { 1,0 };

const VectorMovement* vectors[VECTORSLENGTH] = {
	&ZEROVECTOR,
	&UPVECTOR,
	&LEFTVECTOR,
	&DOWNVECTOR,
	&RIGHTVECTOR
};

#pragma endregion

#pragma region GenericMethods

int getStringLength(const char* str) {
	if (str == nullptr) {
		return 0;
	}

	int count = 0;

	while (str[count] != '\0') {
		count++;
	}

	return count;
}
bool isPacman(char symbol) {
	return symbol == PACMANSYMBOL;
}

bool isRegularFood(char symbol) {
	return symbol == FOODSYMBOL;
}

bool isPellet(char symbol) {
	return symbol == PELLETSYMBOL;
}

bool isFood(char symbol) {
	for (size_t i = 0; i < NUMBERSOFSYMBOLSFORFOOD; i++)
	{
		if (SYMBOLSFORFOOD[i] == symbol) {
			return true;
		}
	}

	return false;
}


void showConsoleCursor(bool showCursor) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hStdout, &cursorInfo);
	cursorInfo.bVisible = showCursor;
	SetConsoleCursorInfo(hStdout, &cursorInfo);
}
#pragma endregion

#pragma region MapFunctions

bool areCoordinatesInMapRange(Map& map, const Coordinates& coordinates) {
	return coordinates.x >= 0 && coordinates.y >= 0
		&& map.horizontalSize > coordinates.x && map.verticalSize > coordinates.y;
}

char getAtPosition(Map& map, const Coordinates& coordinates) {
	if (!areCoordinatesInMapRange(map, coordinates)) {
		return 0;
	}

	return map.content[coordinates.y][coordinates.x];
}


void setAtPosition(Map& map, const Coordinates& coordinates, char symbol) {
	if (!areCoordinatesInMapRange(map, coordinates)) {
		return;
	}

	map.content[coordinates.y][coordinates.x] = symbol;
}

void getPositionOfSymbol(Map& map, Coordinates& result, char symbol) {
	for (size_t i = 0; i < map.verticalSize; i++)
	{
		for (size_t j = 0; j < map.horizontalSize; j++)
		{
			if (map.content[i][j] == symbol) {
				result.x = j;
				result.y = i;
				return;
			}
		}
	}

	result.x = -1;
	result.y = -1;
}


void disposeMap(Map& map) {
	for (size_t i = 0; i < map.verticalSize; i++)
	{
		delete[] map.content[i];
	}

	delete[] map.content;

	map.horizontalSize = 0;
	map.verticalSize = 0;
};

#pragma endregion


#pragma region ReadFile

void intializeMap(Map& map) {
	map.content = new char* [map.verticalSize];

	for (size_t i = 0; i < map.verticalSize; i++)
	{
		map.content[i] = new char[map.horizontalSize];
	}
}

void handleFileTooShort(Map& map) {
	cout << FILETOOSHORTMESSAGE << endl;
	disposeMap(map);
}

void readFileAndSaveOnMap(Map& map, ifstream& mapFile) {
	char line[MAZEMAXSIZE + 1];

	for (int i = 0; i < map.verticalSize; i++)
	{
		if (mapFile.eof()) {
			handleFileTooShort(map);
			return;
		}

		mapFile.getline(line, MAZEMAXSIZE + 1);

		if (getStringLength(line) < map.horizontalSize) {
			handleFileTooShort(map);
			return;
		}

		for (int j = 0; j < map.horizontalSize; j++)
		{
			setAtPosition(map, { j, i }, line[j]);
		}
	}
}


void readMap(Map& map, const char* fileName) {
	ifstream mapFile(fileName);

	if (!mapFile.is_open()) {
		cout << UNABLETOOPENFILEMESSAGE << endl;
		map.verticalSize = 0;
		map.horizontalSize = 0;
		return;
	}

	mapFile >> map.verticalSize;
	mapFile >> map.horizontalSize;

	while (mapFile.get() != '\n');

	intializeMap(map);

	readFileAndSaveOnMap(map, mapFile);

	mapFile.close();
}


#pragma endregion

#pragma region SetUpGame

int countFood(Map& map) {
	int count = 0;
	for (int i = 0; i < map.verticalSize; i++)
	{
		for (int j = 0; j < map.horizontalSize; j++)
		{
			if (isFood(getAtPosition(map, { j, i }))) {
				count++;
			}
		}
	}

	return count;
}

bool setUpPacman(Game& game) {
	game.pacman.symbol = PACMANSYMBOL;
	getPositionOfSymbol(game.map, game.pacman.position, game.pacman.symbol);
	game.pacman.currentDirection = &ZEROVECTOR;
	game.pacman.pacmanColor = PACMANCOLORCODE;
	game.foodCount = countFood(game.map);
	game.currentGameMode = ChaseMode;
	return true;
}


bool setUpGame(Game& game) {
	showConsoleCursor(false);

	game.leftInFrightenedMode = 0;
	game.score = 0;
	readMap(game.map, MAPFILENAME);

	if (game.map.horizontalSize == 0 || game.map.verticalSize == 0) {
		return false;
	}

	return setUpPacman(game);
}

#pragma endregion

int main()
{
	Game game;
	setUpGame(game);
}