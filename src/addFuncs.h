#include <Windows.h>
#include <math.h>
#include "proc.h"
#include "offsets.h"
#include <cctype>
#include <vector>


extern struct Vector3 {
	float x, y, z;
};

extern struct Vec4 {
	float x, y, z, w;
};

extern struct Angle {
	float pitch, yaw;
};

extern struct Entity {
	char name[20] = {};
	int aliveFlag, teamID, health, baseAddr;
	float posX, posY, posZHead, posZLeg, pitchAgnle, yawAngle;
};




extern struct Line {
	float x1, y1, x2, y2;
	int color;
};



Angle CalculateAngle(Vector3 myPos, Vector3 enemyPos);

Entity GetEntInfo(HANDLE hProcess, int CurEntAddr);

float calculateAngleDistance(Angle angle1, Angle angle2);

std::vector <Entity> getAllEntities(HANDLE hProcess, int baseAddr);

Entity getNearestEntitie(HANDLE hProcess, int baseAddr, std::vector <Entity> entitieList, bool chooseFriend, bool chooseDead, float radius);

double calculateDistance(int x1, int y1, int z1, int x2, int y2, int z2);

int ifMoreThanFourAfterDot(float num);

std::vector <float> getViewMatrix(HANDLE hProcess, int baseAddr);

POINT WordlToScreen(std::vector <float> mtx, Vector3 pos, int width, int height);