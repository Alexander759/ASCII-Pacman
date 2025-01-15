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

char PLAYERHASWONMESSAGE[] = "You won!";
char PLAYERHASLOSTMESSAGE[] = "You lost";

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

const int MILISECONDSINCYCLE = 10;
const int CYCLESPERMOVE = 10;
const int MILISECONDSPERMOVE = MILISECONDSINCYCLE * CYCLESPERMOVE;

const int OFFSETX = 0;
const int OFFSETY = 0;

const char UPKEYSYMBOL = 'W';
const char LEFTKEYSYMBOL = 'A';
const char DOWNKEYSYMBOL = 'S';
const char RIGHTKEYSYMBOL = 'D';
const int MASKKEYPRESSED = 0x8000;

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

void setCursorPosition(const Coordinates& coordinates) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD position = { coordinates.x , coordinates.y };
	SetConsoleCursorPosition(hStdout, position);
}

void setCursorPositionWithOffset(const Coordinates& coordinates) {
	Coordinates offsetCoordinates = {
		coordinates.x + OFFSETX,
		coordinates.y + OFFSETY
	};
	setCursorPosition(offsetCoordinates);
}

void printAtCoordinatesOfConsole(const Coordinates& coordinates, char symbol) {
	setCursorPosition(coordinates);
	cout << symbol;
}

#pragma endregion

#pragma region CoordinatesMethods

void copyCoordinates(Coordinates& destination, const Coordinates& source) {
	destination.x = source.x;
	destination.y = source.y;
}

#pragma endregion


#pragma region MapFunctions

void reduceCoordinates(Map& map, Coordinates& coordinates) {
	if (map.horizontalSize <= coordinates.x) {
		coordinates.x %= map.horizontalSize;
	}
	else if (coordinates.x < 0) {
		coordinates.x %= map.horizontalSize;
		coordinates.x += map.horizontalSize;
	}

	if (map.verticalSize <= coordinates.y) {
		coordinates.y %= map.verticalSize;
	}
	else if (coordinates.y < 0) {
		coordinates.y %= map.verticalSize;
		coordinates.y += map.verticalSize;
	}
}

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

bool areCoordinatesReachableForPacman(Map& map, Coordinates& coordinates) {
	return areCoordinatesInMapRange(map, coordinates)
		&& getAtPosition(map, coordinates) != '#';
}



void printAtMap(Map& map, const Coordinates& coordinates, char symbol) {
	if (areCoordinatesInMapRange(map, coordinates)) {
		setAtPosition(map, coordinates, symbol);
		printAtCoordinatesOfConsole(coordinates, symbol);
	}
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

void printMapOnConsole(Map& map) {
	if (map.content == nullptr || map.horizontalSize == 0 || map.verticalSize == 0) {
		return;
	}

	for (size_t i = 0; i < map.verticalSize; i++)
	{
		for (size_t j = 0; j < map.horizontalSize; j++)
		{
			Coordinates coordinates = { j, i };
			printAtCoordinatesOfConsole(coordinates, getAtPosition(map, coordinates));
		}
	}
};

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

#pragma region GameFunctions

void eatFood(Game& game, Coordinates& coordinates) {
	char symbol = getAtPosition(game.map, coordinates);

	if (!isFood(symbol)) {
		return;
	}

	game.score++;
}

void updatePacmanPosition(Game& game, Coordinates& newPosition) {
	char symbol = getAtPosition(game.map, newPosition);

	if (isFood(symbol)) {
		eatFood(game, newPosition);
	}

	printAtMap(game.map, game.pacman.position, FREESPACESYMBOL);

	copyCoordinates(game.pacman.position, newPosition);

	printAtMap(game.map, game.pacman.position, game.pacman.symbol);
}

void movePacman(Game& game) {
	Coordinates newPosition = {
		game.pacman.position.x + game.pacman.currentDirection->x,
		game.pacman.position.y + game.pacman.currentDirection->y
	};

	reduceCoordinates(game.map, newPosition);

	if (areCoordinatesReachableForPacman(game.map, newPosition)) {
		updatePacmanPosition(game, newPosition);
	}
}

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

#pragma region MainGameLogic

const VectorMovement* listenForInput() {
	if (GetAsyncKeyState(UPKEYSYMBOL) & MASKKEYPRESSED) {
		return &UPVECTOR;
	}

	if (GetAsyncKeyState(LEFTKEYSYMBOL) & MASKKEYPRESSED) {
		return &LEFTVECTOR;
	}

	if (GetAsyncKeyState(DOWNKEYSYMBOL) & MASKKEYPRESSED) {
		return &DOWNVECTOR;
	}

	if (GetAsyncKeyState(RIGHTKEYSYMBOL) & MASKKEYPRESSED) {
		return &RIGHTVECTOR;
	}

	return nullptr;
};

bool playerWon(Game& game) {
	return game.foodCount == game.score;
};

void checkForPacmanNewDirection(Game& game) {
	const VectorMovement* newDirection = listenForInput();
	if (newDirection != nullptr) {
		Coordinates possibleCoordinates = { 
			game.pacman.position.x + newDirection->x,
			game.pacman.position.y + newDirection->y 
		};

		if (areCoordinatesReachableForPacman(game.map, possibleCoordinates)) {
			game.pacman.currentDirection = newDirection;
		}
	}
}

void handleEndOfGame(Game& game, bool playerHasWon) {
	Coordinates end = { 0, game.map.verticalSize + 1 };
	setCursorPositionWithOffset(end);
	if (playerHasWon) {
		cout << PLAYERHASWONMESSAGE << endl;
	}
	else {
		cout << PLAYERHASLOSTMESSAGE << endl;
	}
}

void disposeGame(Game& game) {
	disposeMap(game.map);
};

void startGame(Game& game) {
	printMapOnConsole(game.map);

	int cycles = 0;
	bool playerHasWon = false;
	while (!playerHasWon) {
		checkForPacmanNewDirection(game);

		if (cycles >= CYCLESPERMOVE) {
			movePacman(game);

			cycles = 0;
			playerHasWon = playerWon(game);
		}

		Sleep(MILISECONDSINCYCLE);
		cycles++;
	}

	handleEndOfGame(game, playerHasWon);
	disposeGame(game);
}


#pragma endregion


int main()
{
	Game game;
	if (setUpGame(game)) {
		startGame(game);
	}	
}