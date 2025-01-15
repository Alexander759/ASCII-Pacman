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

char MAPFILENAME[] = "map.txt";

char UNABLETOOPENFILEMESSAGE[] = "Couldn't open file!";
char FILETOOSHORTMESSAGE[] = "File shorter than expected!";

#pragma endregion

#pragma region GenericMethods

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

#pragma endregion

#pragma region MapFunctions

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


int main()
{
	Map map;
	readMap(map, MAPFILENAME);

	for (int i = 0; i < map.verticalSize; i++)
	{
		for (int j = 0; j < map.horizontalSize; j++)
		{
			cout << getAtPosition(map, { j, i });
		}
		cout << endl;
	}

	disposeMap(map);
}