/**
*
* Solution to course project #5
* Introduction to programming course
* Faculty of Mathematics and Informatics of Sofia University
* Winter semester 2024/2025
*
* @author Alexander Asenov
* @idnumber 2MI0600422
* @compiler VCC
*
* <Main program file. Start to run pacman.>
*
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

using namespace std;

/*
===================================
Game custom types
===================================
*/

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
	int startScoreValue;
	char symbol;
	char previousSymbolOnCurrentPosition;
	bool hasStarted;
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
	int movesLeftInFrightenedMode;
	int score;
	bool ghostsMustMoveBack;
};

/*
===================================
Constants
===================================
*/

const int MILISECONDSINCYCLE = 10;
const int CYCLESPERMOVE = 20;
const int CYCLESPERFIRSTPACMANMOVEINFRIGHTENEDMODE = CYCLESPERMOVE / 2;
const int MILISECONDSPERMOVE = MILISECONDSINCYCLE * CYCLESPERMOVE;

const int MOVESINFRIGHTENEDMODE = 10;

const int OFFSETX = 0;
const int OFFSETY = 0;

const int MAZEMAXSIZE = 50;

const char PACMANSYMBOL = 'Y';
const int PACMANCOLORCODE = 14;

const char BLINKYSYMBOL = 'B';
const int BLINKYCOLORCODE = 12;
const int BLINKYSTARTSCORE = 0;

const char PINKYSYMBOL = 'P';
const int PINKYCOLORCODE = 13;
const int PINKYSTARTSCORE = 20;

const char INKYSYMBOL = 'I';
const int INKYCOLORCODE = 11;
const int INKYSTARTSCORE = 40;

const char CLYDESYMBOL = 'C';
const int CLYDECOLORCODE = 10;
const int CLYDESTARTSCORE = 60;

const char FOODSYMBOL = '-';
const int FOODCOLORCODE = 14;

const char PELLETSYMBOL = '@';
const int PELLETCOLOR = 14;

const char WALLSYMBOL = '#';
const int WALLCOLOR = 1;

const int DEFAULTCOLOR = 7;

const char FREESPACESYMBOL = ' ';

const int NUMBERSOFSYMBOLSFORFOOD = 2;
char SYMBOLSFORFOOD[NUMBERSOFSYMBOLSFORFOOD] = { FOODSYMBOL, PELLETSYMBOL };

char MAPFILENAME[] = "map.txt";

char UNABLETOOPENFILEMESSAGE[] = "Couldn't open file!";
char FILETOOSHORTMESSAGE[] = "File shorter than expected!";
char ERRORDURINGSETUPMESSAGE[] = "Error during set up!";

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

const char UPKEYSYMBOL = 'W';
const char LEFTKEYSYMBOL = 'A';
const char DOWNKEYSYMBOL = 'S';
const char RIGHTKEYSYMBOL = 'D';
const int MASKKEYDOWN = 0x8000;

const int PINKYERROR = 4;
const int INKYERROR = 2;
const int CLYDEMAXDISTANCE = 8;

const int FRIGHTENEDMODEGHOSTSCOLOR = 1;

const double DEFAULTPRECISION = 1e-7;

/*
===================================
Generic Methods
===================================
*/

double getAbsoluteValue(double num) {
	return (num >= 0) ? num : (-1) * num;
}

// Uses Newton–Raphson method to find square root
double squareRoot(double num, double eps = DEFAULTPRECISION) {
	if (num < 0) {
		return -1; // There is no square root of negative number
	}

	if (num == 0) {
		return 0; // The square root of 0 is 0
	}

	double prev = num;
	double next = (num + 1) / 2.0; // Initial guess

	while (getAbsoluteValue(prev - next) > eps) {
		prev = next;
		next = (prev + num / prev) / 2.0; // Next guess
	}
	
	return next;
}

double exponentiate(double base, unsigned int power) {
	double result = 1;
	for (size_t i = 0; i < power; i++)
	{
		result *= base;
	}

	return result;
}

int getRandom(unsigned int start, unsigned int end) {
	return rand() % (end - start) + start;
}

unsigned int getNumberOfTruesInBoolArray(bool* bools, int length) {
	unsigned int result = 0;
	for (size_t i = 0; i < length; i++)
	{
		if (bools[i]) {
			result++;
		}
	}

	return result;
}

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

bool isGhost(char symbol) {
	return symbol == PINKYSYMBOL || symbol == INKYSYMBOL || symbol == BLINKYSYMBOL || symbol == CLYDESYMBOL;
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

bool isWall(char symbol) {
	return symbol == WALLSYMBOL;
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

void setConsoleColor(int colorCode) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, colorCode);
}

void setColorToSymbolStatic(char symbol) {
	switch (symbol) {
	case FOODSYMBOL:
		setConsoleColor(FOODCOLORCODE);
		break;
	case PELLETSYMBOL:
		setConsoleColor(PELLETCOLOR);
		break;
	case WALLSYMBOL:
		setConsoleColor(WALLCOLOR);
		break;
	case PACMANSYMBOL:
		setConsoleColor(PACMANCOLORCODE);
		break;
	case BLINKYSYMBOL:
		setConsoleColor(BLINKYCOLORCODE);
		break;
	case PINKYSYMBOL:
		setConsoleColor(PINKYCOLORCODE);
		break;
	case INKYSYMBOL:
		setConsoleColor(INKYCOLORCODE);
		break;
	case CLYDESYMBOL:
		setConsoleColor(CLYDECOLORCODE);
		break;
	default:
		setConsoleColor(DEFAULTCOLOR);
		break;
	}
}

/*
===================================
Vector Methods
===================================
*/

bool areEqualVectors(VectorMovement firstVector, VectorMovement secondVector) {
	return firstVector.x == secondVector.x && firstVector.y == secondVector.y;
}

void copyVector(VectorMovement& destination, const VectorMovement source) {
	destination.x = source.x;
	destination.y = source.y;
}

void multiplyVectorByScalar(VectorMovement& vector, int scalar) {
	vector.x *= scalar;
	vector.y *= scalar;
}

const VectorMovement* getOppositeVector(const VectorMovement* vector) {
	if (vector == nullptr) {
		return nullptr;
	}

	VectorMovement opposite;
	copyVector(opposite, *vector);
	multiplyVectorByScalar(opposite, -1);

	for (size_t i = 0; i < VECTORSLENGTH; i++)
	{
		if (areEqualVectors(opposite, *vectors[i])) {
			return vectors[i];
		}
	}

	return nullptr;
}

/*
===================================
Coordinates Methods
===================================
*/

bool areCoordinatesEqual(const Coordinates& firstCoordinates, const Coordinates& secondCoordinates) {
	return firstCoordinates.x == secondCoordinates.x && firstCoordinates.y == secondCoordinates.y;
}

double getDistance(Coordinates& point1, Coordinates& point2) {
	return squareRoot(exponentiate(point1.x - point2.x, 2) + exponentiate(point1.y - point2.y, 2));
}

void copyCoordinates(Coordinates& destination, const Coordinates& source) {
	destination.x = source.x;
	destination.y = source.y;
}

void printAtCoordinatesOfConsole(const Coordinates& coordinates, char symbol) {
	setCursorPosition(coordinates);
	cout << symbol;
}

void printAtCoordinatesWithOffset(const Coordinates& coordinates, char symbol) {
	setCursorPositionWithOffset(coordinates);
	cout << symbol;
}

void printAtCoordinatesWithOffsetAndStaticColor(const Coordinates& coordinates, char symbol) {
	setColorToSymbolStatic(symbol);
	printAtCoordinatesWithOffset(coordinates, symbol);
}

/*
===================================
Map Methods
===================================
*/

// Gets Coordinates in range from 0 to map sizes
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

void printAtMap(Map& map, const Coordinates& coordinates, char symbol) {
	if (areCoordinatesInMapRange(map, coordinates)) {
		setAtPosition(map, coordinates, symbol);
		printAtCoordinatesWithOffset(coordinates, symbol);
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
			printAtCoordinatesWithOffsetAndStaticColor(coordinates, getAtPosition(map, coordinates));
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

bool areCoordinatesValidPacmanPosition(Map& map, Coordinates& coordinates) {
	return areCoordinatesInMapRange(map, coordinates)
		&& !isWall(getAtPosition(map, coordinates));
}

bool areCoordinatesValidGhostPosition(Map& map, Coordinates& coordinates) {
	return areCoordinatesInMapRange(map, coordinates)
		&& !isWall(getAtPosition(map, coordinates))
		&& !isGhost(getAtPosition(map, coordinates));
}

/*
===================================
Ghost Methods
===================================
*/

int getGhostColorCode(GameMode& gameMode, Ghost& ghost) {
	if (gameMode == ChaseMode) {
		return ghost.chaseModeColorCode;
	}

	return FRIGHTENEDMODEGHOSTSCOLOR;
};

void startGhostIfNeeded(Ghost& ghost, int score) {
	if (!ghost.hasStarted && score >= ghost.startScoreValue) {
		ghost.hasStarted = true;
	}
}

/*
===================================
Game Methods
===================================
*/

Ghost* getGhostBySymbol(Game& game, char symbol) {
	if (game.blinky.symbol == symbol) {
		return &game.blinky;
	}

	if (game.pinky.symbol == symbol) {
		return &game.pinky;
	}

	if (game.inky.symbol == symbol) {
		return &game.inky;
	}

	if (game.clyde.symbol == symbol) {
		return &game.clyde;
	}

	return nullptr;
}

void eatFood(Game& game, Coordinates& coordinates) {
	char symbol = getAtPosition(game.map, coordinates);

	if (isGhost(symbol)) {
		symbol = getGhostBySymbol(game, symbol)->previousSymbolOnCurrentPosition;
	}

	if (!isFood(symbol)) {
		return;
	}

	game.score++;

	if (isPellet(symbol) && (!isGhost(getAtPosition(game.map, coordinates)) || game.currentGameMode == FrightenedMode)) {
		if (game.currentGameMode != FrightenedMode) {
			game.currentGameMode = FrightenedMode;
			game.ghostsMustMoveBack = true;
		}

		game.movesLeftInFrightenedMode = MOVESINFRIGHTENEDMODE;
	}
}

void setColorToSymbolInGame(Game& game, char symbol) {
	if (isGhost(symbol)) {
		setConsoleColor(getGhostColorCode(game.currentGameMode, *getGhostBySymbol(game, symbol)));
		return;
	}

	setColorToSymbolStatic(symbol);
}

void printAtMapWithGameColor(Game& game, Coordinates& position, char symbol) {
	setColorToSymbolInGame(game, symbol);
	printAtMap(game.map, position, symbol);
}

void updatePacmanPosition(Game& game, Coordinates& newPosition) {
	char symbol = getAtPosition(game.map, newPosition);

	if (isGhost(symbol)) {
		symbol = getGhostBySymbol(game, symbol)->previousSymbolOnCurrentPosition;
	}

	if (isFood(symbol)) {
		eatFood(game, newPosition);
	}

	printAtMapWithGameColor(game, game.pacman.position, FREESPACESYMBOL);

	copyCoordinates(game.pacman.position, newPosition);

	printAtMapWithGameColor(game, game.pacman.position, game.pacman.symbol);
}

void movePacman(Game& game) {
	Coordinates newPosition = {
		game.pacman.position.x + game.pacman.currentDirection->x,
		game.pacman.position.y + game.pacman.currentDirection->y
	};

	reduceCoordinates(game.map, newPosition);

	if (areCoordinatesValidPacmanPosition(game.map, newPosition)) {
		updatePacmanPosition(game, newPosition);
	}
}

void setBlinkyTarget(Game& game) {
	copyCoordinates(game.blinky.target, game.pacman.position);
};

void setPinkyTarget(Game& game) {
	if (game.pacman.currentDirection == nullptr) {
		return;
	}

	// Set target to 4 in front of pacman
	game.pinky.target.x = PINKYERROR * game.pacman.currentDirection->x + game.pacman.position.x;
	game.pinky.target.y = PINKYERROR * game.pacman.currentDirection->y + game.pacman.position.y;

	// When pacman is moving up the target is also 4 to the left 
	if (game.pacman.currentDirection == &UPVECTOR) {
		game.pinky.target.x += PINKYERROR * game.pacman.currentDirection->y;
	}
}

void setInkyTarget(Game& game) {
	if (game.pacman.currentDirection == nullptr) {
		return;
	}

	Coordinates infrontPacman;
	copyCoordinates(infrontPacman, game.pacman.position);

	// Get coordinates of 2 in front of pacman
	infrontPacman.x += INKYERROR * game.pacman.currentDirection->x;
	infrontPacman.y += INKYERROR * game.pacman.currentDirection->y;

	// When pacman is moving up the coordinates are also 2 to the left 
	if (game.pacman.currentDirection == &UPVECTOR) {
		infrontPacman.x += INKYERROR * game.pacman.currentDirection->y;
	}

	// The target is the reverse vector of infrontPacman and blinky's position
	game.inky.target.x = infrontPacman.x + (infrontPacman.x - game.blinky.position.x);
	game.inky.target.y = infrontPacman.y + (infrontPacman.y - game.blinky.position.y);
}

void setClydeTarget(Game& game) {
	if (getDistance(game.pacman.position, game.clyde.position) < CLYDEMAXDISTANCE) {
		//When distance is less than 8 go to the left down corner (reappearCoordinates)
		copyCoordinates(game.clyde.target, game.clyde.reappearCoordinates);
	}
	else {
		//When distance is more than or equal to 8 follow pacman
		copyCoordinates(game.clyde.target, game.pacman.position);
	}
}

void removeGhost(Game& game, Ghost& ghost) {
	char symbol = getAtPosition(game.map, ghost.position);

	if (areCoordinatesEqual(game.pacman.position, ghost.position)) {
		printAtMapWithGameColor(game, ghost.position, game.pacman.symbol);
	}
	else {
		printAtMapWithGameColor(game, ghost.position, ghost.previousSymbolOnCurrentPosition);
	}
}

void printGhostAtPosition(Game& game, Ghost& ghost, Coordinates& newPosition) {
	if (!areCoordinatesEqual(ghost.position, newPosition)) {
		copyCoordinates(ghost.position, newPosition);
		ghost.previousSymbolOnCurrentPosition = getAtPosition(game.map, newPosition);

		if (isGhost(ghost.previousSymbolOnCurrentPosition)) {
			ghost.previousSymbolOnCurrentPosition = getGhostBySymbol(game, ghost.previousSymbolOnCurrentPosition)->previousSymbolOnCurrentPosition;
		}

		if (isPacman(ghost.previousSymbolOnCurrentPosition)) {
			ghost.previousSymbolOnCurrentPosition = FREESPACESYMBOL;
		}
	}

	printAtMapWithGameColor(game, ghost.position, ghost.symbol);
}

void updateGhostPosition(Game& game, Ghost& ghost, Coordinates& newPosition) {
	removeGhost(game, ghost);
	printGhostAtPosition(game, ghost, newPosition);
}

void moveGhostUsingCurrentVector(Game& game, Ghost& ghost) {
	Coordinates newGhostCoordinates = {
		ghost.position.x + ghost.currentVector->x,
		ghost.position.y + ghost.currentVector->y
	};
	reduceCoordinates(game.map, newGhostCoordinates);
	updateGhostPosition(game, ghost, newGhostCoordinates);
}

void setMoveBackwardsOrStop(Game& game, Ghost& ghost) {
	const VectorMovement* oppositeVector = getOppositeVector(ghost.currentVector);

	if (oppositeVector == nullptr) {
		ghost.currentVector = &ZEROVECTOR;
		return;
	}

	Coordinates possibleCoordinates = {
		ghost.position.x + oppositeVector->x,
		ghost.position.y + oppositeVector->y
	};
	reduceCoordinates(game.map, possibleCoordinates);

	if (areCoordinatesValidGhostPosition(game.map, possibleCoordinates)) {
		ghost.currentVector = oppositeVector;
		return;
	}

	ghost.currentVector = &ZEROVECTOR;
}

bool isVectorPositionValidForGhost(Map& map, Ghost& ghost, const VectorMovement* vector) {
	if (vector == nullptr) {
		return false;
	}

	if (vector == &ZEROVECTOR || vector == getOppositeVector(ghost.currentVector)) {
		return false;
	}

	Coordinates possibleCoordinates = {
		ghost.position.x + vector->x,
		ghost.position.y + vector->y
	};
	reduceCoordinates(map, possibleCoordinates);

	return areCoordinatesValidGhostPosition(map, possibleCoordinates);
}

void setGhostVectorToTarget(Game& game, Ghost& ghost) {
	double minDistance = -1;
	int currentIndex = 0;

	for (size_t i = 0; i < VECTORSLENGTH; i++)
	{
		if (isVectorPositionValidForGhost(game.map, ghost, vectors[i])) {
			Coordinates possibleCoordinates = {
				ghost.position.x + vectors[i]->x,
				ghost.position.y + vectors[i]->y
			};
			reduceCoordinates(game.map, possibleCoordinates);
			double currentDistance = getDistance(possibleCoordinates, ghost.target);

			if (minDistance > currentDistance || minDistance == -1) {
				minDistance = currentDistance;
				currentIndex = i;
			}
		}
	}

	if (minDistance != -1) {
		ghost.currentVector = vectors[currentIndex];
		return;
	}

	setMoveBackwardsOrStop(game, ghost);
}

void setGhostVectorToRandom(Game& game, Ghost& ghost) {
	bool isValid[VECTORSLENGTH] = {};
	for (int i = 0; i < VECTORSLENGTH; i++)
	{
		isValid[i] = isVectorPositionValidForGhost(game.map, ghost, vectors[i]);
	}

	unsigned int numberOfPossibleDirections = getNumberOfTruesInBoolArray(isValid, VECTORSLENGTH);

	if (numberOfPossibleDirections == 0) {
		setMoveBackwardsOrStop(game, ghost);
		return;
	}

	int random = getRandom(0, numberOfPossibleDirections);
	random++;

	int count = 0;
	for (size_t i = 0; i < VECTORSLENGTH; i++)
	{
		if (isValid[i]) {
			count++;

			if (random == count) {
				removeGhost(game, ghost);
				Coordinates newPosition = {
					ghost.position.x + vectors[i]->x,
					ghost.position.y + vectors[i]->y
				};
				ghost.currentVector = vectors[i];
				return;
			}
		}
	}
}

void moveGhost(Game& game, Ghost& ghost) {
	startGhostIfNeeded(ghost, game.score);

	if (ghost.hasStarted) {
		if (game.ghostsMustMoveBack) {
			setMoveBackwardsOrStop(game, ghost);

			if (ghost.currentVector != &ZEROVECTOR) {
				moveGhostUsingCurrentVector(game, ghost);
				return;
			}
		}

		if (game.currentGameMode == ChaseMode) {
			ghost.setTarget(game);
			setGhostVectorToTarget(game, ghost);
		}
		else {
			setGhostVectorToRandom(game, ghost);
		}
	}

	moveGhostUsingCurrentVector(game, ghost);
}

void moveGhosts(Game& game) {
	moveGhost(game, game.blinky);
	moveGhost(game, game.pinky);
	moveGhost(game, game.inky);
	moveGhost(game, game.clyde);
	game.ghostsMustMoveBack = false;
}

/*
===================================
Set Up Methods
===================================
*/

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

void setToClosestValidCoordinates(Map& map, Coordinates& coordinates) {
	if (areCoordinatesInMapRange(map, coordinates) && !isWall(getAtPosition(map, coordinates))) {
		return;
	}

	Coordinates result = { -1, -1 };
	double currentMinDistance = -1;

	Coordinates current;

	for (int i = 0; i < map.verticalSize; i++)
	{
		for (int j = 0; j < map.horizontalSize; j++)
		{
			current.x = j;
			current.y = i;

			if (areCoordinatesInMapRange(map, current) && !isWall(getAtPosition(map, current))) {
				double distance = getDistance(current, coordinates);

				if (distance < currentMinDistance || currentMinDistance == -1) {
					copyCoordinates(result, current);
					currentMinDistance = distance;
				}
			}
		}
	}

	copyCoordinates(coordinates, result);
}

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

bool setUpGhost(
	Game& game,
	Ghost& ghost,
	char symbol,
	bool hasStarted,
	char previousSymbolOnCurrentPosition,
	int startScoreValue,
	int chaseModeColorCode,
	const VectorMovement* currentVector,
	void (*setTarget) (Game& game),
	const Coordinates& reappearCoordinates) {
	if (currentVector == nullptr) {
		return false;
	}

	ghost.symbol = symbol;
	ghost.hasStarted = hasStarted;
	ghost.previousSymbolOnCurrentPosition = previousSymbolOnCurrentPosition;
	ghost.startScoreValue = startScoreValue;
	ghost.chaseModeColorCode = chaseModeColorCode;
	ghost.currentVector = currentVector;
	ghost.setTarget = setTarget;
	getPositionOfSymbol(game.map, ghost.position, ghost.symbol);
	copyCoordinates(ghost.reappearCoordinates, reappearCoordinates);
	setToClosestValidCoordinates(game.map, ghost.reappearCoordinates);

	return true;
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

bool setUpPacmanAndGhosts(Game& game) {
	return setUpPacman(game)
		&& setUpGhost(game, game.blinky, BLINKYSYMBOL, false, FREESPACESYMBOL,
			BLINKYSTARTSCORE, BLINKYCOLORCODE, &ZEROVECTOR,
			setBlinkyTarget, { game.map.horizontalSize - 1, 0 })
		&& setUpGhost(game, game.pinky, PINKYSYMBOL, false, FREESPACESYMBOL,
			PINKYSTARTSCORE, PINKYCOLORCODE, &ZEROVECTOR,
			setPinkyTarget, { 0, 0 })
		&& setUpGhost(game, game.inky, INKYSYMBOL, false, FREESPACESYMBOL,
			INKYSTARTSCORE, INKYCOLORCODE, &ZEROVECTOR,
			setInkyTarget, { game.map.horizontalSize - 1, game.map.verticalSize - 1 })
		&& setUpGhost(game, game.clyde, CLYDESYMBOL, false, FREESPACESYMBOL,
			CLYDESTARTSCORE, CLYDECOLORCODE, &ZEROVECTOR,
			setClydeTarget, { 0, game.map.verticalSize - 1 });
}

bool setUpGame(Game& game) {
	srand(time(0));
	showConsoleCursor(false);

	game.movesLeftInFrightenedMode = 0;
	game.ghostsMustMoveBack = false;
	game.score = 0;
	readMap(game.map, MAPFILENAME);

	if (game.map.horizontalSize == 0 || game.map.verticalSize == 0) {
		return false;
	}

	return setUpPacmanAndGhosts(game);
}

/*
===================================
Main Game Methods
===================================
*/

const VectorMovement* listenForInput() {
	if (GetAsyncKeyState(UPKEYSYMBOL) & MASKKEYDOWN) {
		return &UPVECTOR;
	}

	if (GetAsyncKeyState(LEFTKEYSYMBOL) & MASKKEYDOWN) {
		return &LEFTVECTOR;
	}

	if (GetAsyncKeyState(DOWNKEYSYMBOL) & MASKKEYDOWN) {
		return &DOWNVECTOR;
	}

	if (GetAsyncKeyState(RIGHTKEYSYMBOL) & MASKKEYDOWN) {
		return &RIGHTVECTOR;
	}

	return nullptr;
};

void disposeGame(Game& game) {
	disposeMap(game.map);
	showConsoleCursor(true);
};

bool playerWon(Game& game) {
	return game.foodCount == game.score;
};

bool playerLost(Game& game, bool hasGhostColided) {
	return hasGhostColided && game.currentGameMode == ChaseMode;
};

void checkForPacmanNewDirection(Game& game) {
	const VectorMovement* newDirection = listenForInput();
	if (newDirection != nullptr) {
		Coordinates possibleCoordinates = { 
			game.pacman.position.x + newDirection->x,
			game.pacman.position.y + newDirection->y 
		};

		if (areCoordinatesValidPacmanPosition(game.map, possibleCoordinates)) {
			game.pacman.currentDirection = newDirection;
		}
	}
}

void handleEndOfGame(Game& game, bool playerHasWon) {
	setConsoleColor(DEFAULTCOLOR);

	// The end of the map plus one row
	Coordinates messageCoordinates = { 0, game.map.verticalSize + 1 }; 
	
	setCursorPositionWithOffset(messageCoordinates);
	if (playerHasWon) {
		cout << PLAYERHASWONMESSAGE << endl;
	}
	else {
		cout << PLAYERHASLOSTMESSAGE << endl;
	}
}

void handleFrightenedMode(Game& game) {
	if (game.currentGameMode == FrightenedMode) {
		game.movesLeftInFrightenedMode--;
	}
	
	if (game.currentGameMode == FrightenedMode && game.movesLeftInFrightenedMode <= 0) {
		game.movesLeftInFrightenedMode = 0;
		game.currentGameMode = ChaseMode;
	}
}

Ghost* getCurrentCollidingGhost(Game& game) {
	if (areCoordinatesEqual(game.pacman.position, game.blinky.position)) {
		return &game.blinky;
	}

	if (areCoordinatesEqual(game.pacman.position, game.pinky.position)) {
		return &game.pinky;
	}

	if (areCoordinatesEqual(game.pacman.position, game.inky.position)) {
		return &game.inky;
	}

	if (areCoordinatesEqual(game.pacman.position, game.clyde.position)) {
		return &game.clyde;
	}

	return nullptr;
}

bool checkForCollidedGhost(Game& game) {
	Ghost* collidingGhost = getCurrentCollidingGhost(game);

	if (collidingGhost != nullptr) {
		if (game.currentGameMode == FrightenedMode) {
			updateGhostPosition(game, *collidingGhost, collidingGhost->reappearCoordinates);
		}

		// Prints ghost again in case pacman collided into him
		printAtMapWithGameColor(game, collidingGhost->position, collidingGhost->symbol);

		return true;
	}

	return false;
}

void startGame(Game& game) {
	printMapOnConsole(game.map);

	int cycles = 0;
	bool playerHasWon = false;
	while (!playerHasWon) {
		checkForPacmanNewDirection(game);

		if (game.currentGameMode == FrightenedMode && cycles == CYCLESPERFIRSTPACMANMOVEINFRIGHTENEDMODE) {
			movePacman(game);
			checkForCollidedGhost(game);
			playerHasWon = playerWon(game);
		}
		else if (cycles == CYCLESPERMOVE) {
			movePacman(game);

			if (playerLost(game, checkForCollidedGhost(game))) {
				break;
			}

			moveGhosts(game);

			if (playerLost(game, checkForCollidedGhost(game))) {
				break;
			}

			handleFrightenedMode(game);

			cycles = 0;
			playerHasWon = playerWon(game);
		}

		Sleep(MILISECONDSINCYCLE);
		cycles++;
	}

	handleEndOfGame(game, playerHasWon);
	disposeGame(game);
}

int main()
{
	Game game;
	if (setUpGame(game)) {
		startGame(game);
	}
	else {
		cout << ERRORDURINGSETUPMESSAGE << endl;
	}
}