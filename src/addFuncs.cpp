#include "addFuncs.h"
#include <Windows.h>
#include <math.h>
#include "proc.h"
#include "offsets.h"
#include <iostream>
#include <cctype>
#include <vector>
#include <sstream>

#define M_PI 3.14159265358979323846



Angle CalculateAngle(Vector3 myPos, Vector3 enemyPos) {
	Vector3 delta = { enemyPos.x - myPos.x, enemyPos.y - myPos.y, enemyPos.z - myPos.z };
	float yaw = (atan2(delta.y, delta.x) * 180.0 / M_PI) + 90;
	if (yaw < 0) {
		yaw += 360.0;
	}

	float distanceXY = sqrt(delta.x * delta.x + delta.y * delta.y);
	float pitch = atan2(delta.z, distanceXY) * 180.0 / M_PI;
	return { pitch, yaw };
}

Entity GetEntInfo(HANDLE hProcess, int CurEntAddr) {
	char name[20];
	int aliveFlag;
	int health;
	int teamID;
	float posX;
	float posY;
	float posZHead;
	float posZLeg;
	float pitchAgnle;
	float yawAngle;
	int baseAddr = CurEntAddr;


	ReadProcessMemory(hProcess, (BYTE*)(baseAddr+PlayerClass::xCordOffset), &posX, sizeof(posX), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::yCordOffset), &posY, sizeof(posY), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::zLegCordOffset), &posZLeg, sizeof(posZLeg), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::zHeadCordOffset), &posZHead, sizeof(posZHead), nullptr);

	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::pitchViewOffset), &pitchAgnle, sizeof(pitchAgnle), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::yawViewOffset), &yawAngle, sizeof(yawAngle), nullptr);

	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::nameOffset), &name, sizeof(name), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::aliveOffset), &aliveFlag, sizeof(aliveFlag), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::teamOffset), &teamID, sizeof(teamID), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + PlayerClass::healthOffsets), &health, sizeof(health), nullptr);

	Entity newEntity;
	strcpy_s(newEntity.name, sizeof(newEntity.name), name);
	newEntity.aliveFlag = !aliveFlag;
	newEntity.health = health;
	newEntity.teamID = teamID;
	newEntity.posX = posX;
	newEntity.posY = posY;
	newEntity.posZHead = posZHead;
	newEntity.posZLeg = posZLeg;
	newEntity.pitchAgnle = pitchAgnle;
	newEntity.yawAngle = yawAngle;
	newEntity.baseAddr = baseAddr;

	return newEntity;

}

float calculateAngleDistance(Angle angle1, Angle angle2) {

	double yaw1Rad = angle1.yaw * (M_PI / 180.0);
	double pitch1Rad = angle1.pitch * (M_PI / 180.0);
	double yaw2Rad = angle2.yaw * (M_PI / 180.0);
	double pitch2Rad = angle2.pitch * (M_PI / 180.0);

	
	double dYaw = yaw2Rad - yaw1Rad;
	double dPitch = pitch2Rad - pitch1Rad;

	
	double distanceInRadians = sqrt(dYaw * dYaw + dPitch * dPitch);

	
	return distanceInRadians;
}

std::vector <Entity> getAllEntities(HANDLE hProcess, int baseAddr) {
	std::vector <Entity> entities = {};

	int myAddr;
	float myposX, myposY, myposZHead;
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + basePtrOffset::localPlayerOffset), &myAddr, sizeof(myAddr), nullptr);

	ReadProcessMemory(hProcess, (BYTE*)(myAddr + PlayerClass::xCordOffset), &myposX, sizeof(myposX), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(myAddr + PlayerClass::yCordOffset), &myposY, sizeof(myposY), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(myAddr + PlayerClass::zHeadCordOffset), &myposZHead, sizeof(myposZHead), nullptr);

	int EntListAddr;

	int EntListPtrBaseAddr = baseAddr + basePtrOffset::EntityListOffset;

	ReadProcessMemory(hProcess, (BYTE*)EntListPtrBaseAddr, &EntListAddr, sizeof(EntListAddr), nullptr);
	
	int emptyRow = 0;

	for (int i = 0; i < 128; i += 4) {
		float entposX = 0, entposY = 0, entposZHead = 0;
		
		int CurEntAddr;
		ReadProcessMemory(hProcess, (BYTE*)EntListAddr + i, &CurEntAddr, sizeof(CurEntAddr), nullptr);
		ReadProcessMemory(hProcess, (BYTE*)CurEntAddr + PlayerClass::xCordOffset, &entposX, sizeof(entposX), nullptr);
		ReadProcessMemory(hProcess, (BYTE*)CurEntAddr + PlayerClass::yCordOffset, &entposY, sizeof(myposY), nullptr);
		ReadProcessMemory(hProcess, (BYTE*)CurEntAddr + PlayerClass::zHeadCordOffset, &entposZHead, sizeof(entposZHead), nullptr);

		if (entposX == myposX && entposY == myposY && entposZHead == myposZHead) {

			continue;
		}

		if (entposX > 0 && entposY > 0 && !(ifMoreThanFourAfterDot(entposX)) && !(ifMoreThanFourAfterDot(entposY))) {
			Entity curEnt = GetEntInfo(hProcess, CurEntAddr);
			entities.push_back(curEnt);
			emptyRow = 0;

		}
		else {
			emptyRow++;
		}

		if (emptyRow >= 5) {
			break;
		}

	}
	return entities;
}

double calculateDistance(int x1, int y1, int z1, int x2, int y2, int z2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

Entity getNearestEntitie(HANDLE hProcess, int baseAddr, std::vector <Entity> entitieList, bool chooseFriend, bool chooseDead, float radius) {

	

	int meAddr;
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + basePtrOffset::localPlayerOffset), &meAddr, sizeof(meAddr), nullptr);
	int MyTeam;
	float  MyYaw, MyPitch, MyposX, MyposY, MyposZHead;

	ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::yawViewOffset), &MyYaw, sizeof(MyYaw), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::pitchViewOffset), &MyPitch, sizeof(MyPitch), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::xCordOffset), &MyposX, sizeof(MyposX), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::yCordOffset), &MyposY, sizeof(MyposY), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::zHeadCordOffset), &MyposZHead, sizeof(MyposZHead), nullptr);
	ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::teamOffset), &MyTeam, sizeof(MyTeam), nullptr);


	Angle myAngles = { MyPitch, MyYaw };
	Vector3 myHeadPos = { MyposX, MyposY, MyposZHead };

	float minDistanceToAng = 999;
	Angle entAng = {99, 99};
	Entity nearestEnt = {};
	Angle nearestAngle = { 0, -1 };

	for (Entity curEnt : entitieList) {
		if (!chooseFriend && (curEnt.teamID == MyTeam)) {
			continue;
		}
		if (!chooseDead && !curEnt.aliveFlag) {
			continue;
		}

		Vector3 entHeadPos = { curEnt.posX, curEnt.posY, curEnt.posZHead };
		Angle currentNewAngle = CalculateAngle(myHeadPos, entHeadPos);

		if (minDistanceToAng > calculateAngleDistance(myAngles, currentNewAngle)) {
			minDistanceToAng = calculateAngleDistance(myAngles, currentNewAngle);

			if (radius == -1) {
				nearestEnt = curEnt;
				continue;
			}


			float deltaYawPixels = (currentNewAngle.yaw - myAngles.yaw) * (1920 / 360.0f);
			float deltaPitchPixels = (currentNewAngle.pitch - myAngles.pitch) * (1080 / 180.0f);
			float distance = std::sqrt(deltaYawPixels * deltaYawPixels + deltaPitchPixels * deltaPitchPixels);

			if (distance > radius) {
				continue;
			}

			nearestEnt = curEnt;
			

			

		}
	}


	return nearestEnt;
}

int ifMoreThanFourAfterDot(float num) {
	std::stringstream ss;
	ss << num;
	std::string numAsString = ss.str();
	size_t decimalPos = numAsString.find('.');
	if (decimalPos != std::string::npos) {
		int numDigits = numAsString.length() - decimalPos - 1;

		return numDigits > 4;
	}

	return false;
}

std::vector <float> getViewMatrix(HANDLE hProcess, int baseAddr) {

	int baseMatrixAddr = baseAddr + basePtrOffset::viewMatrixOffset;
	std::vector <float> mtx = {};


	for (int i = 0; i <= 60; i += 4) {
		float tempVal;
		ReadProcessMemory(hProcess, (BYTE*)baseMatrixAddr + i, &tempVal, sizeof(tempVal), nullptr);

		mtx.push_back(tempVal);
	}

	
	return mtx;
}

POINT WordlToScreen(std::vector <float> mtx, Vector3 pos, int width, int height) {

	POINT twoD;

	float screenW = (mtx[3] * pos.x) + (mtx[7] * pos.y) + (mtx[11] * pos.z) + mtx[15];
	int counter = 0;

	
	
	for (float i : mtx) {
		std::cout << i << " ";
		if (counter == 4) {
			std::cout << std::endl;
			counter = 0;
		}
		counter++;
	}
	Sleep(50);
	system("cls");

	if (screenW > 0.001f) {

		float screenX = (mtx[0] * pos.x) + (mtx[4] * pos.y) + (0 * pos.z) + mtx[12]; //roll v matrice pomelyal na 0 chtob vernut6 (0 * pos.z) --> (m[8] * pos.z)
		float screenY = (mtx[1] * pos.x) + (mtx[5] * pos.y) + (mtx[9] * pos.z) + mtx[13];

		float camX = width / 2.f;
		float camY = height / 2.f;

		float x = camX + (camX * screenX / screenW);
		float y = camY - (camY * screenY / screenW);

		twoD.x = (int)x;
		twoD.y = (int)y;

		return twoD;

	}
	else {
		return POINT(-999, -999);
	}
}