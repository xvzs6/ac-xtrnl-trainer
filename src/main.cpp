#include "proc.h"
#include "offsets.h"
#include "addFuncs.h"
#include <iostream>
#include <vector>
#include <ctime>

#include <Windows.h>
#include <sstream>
#include <thread>
#include <iomanip>
#include <cctype>
#include <algorithm>

#include <Dwmapi.h>
#include <iostream>
#include <string>
#include <sstream>
#include <random>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#pragma comment(lib, "d3d11.lib")

#include <dwmapi.h>
#include <d3d11.h>
#include <d3dcompiler.h>



#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define tPI (2 * M_PI)

int screenWidth = GetSystemMetrics(SM_CXSCREEN);;
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

HANDLE hSemaphore;
bool aimThreadRunning = false;
bool espThreadRunning = false;
bool tpThreadRunning = false;
bool flyThreadRunning = false;

bool fFire = false;
bool tpFriend = true;

bool drawSnap = true;
bool drawNick = true;
bool drawTeam = true;
bool drawFov = true;
bool drawHp = true;
bool drawDist = true;

bool lgbtMenu = false;

float headOffset = 0;

int WindowH = 1080;
int WindowW = 1720;


bool box3D = false;
bool fov360 = false;

int aimDelay = 0;
float fovRad = WindowH/5;

float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};

float FriendPen[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

float EnemyPen[4] = { 1.0f, 1.0f, 0.0f, 1.0f };



HDC hdc = GetDC(FindWindowA(NULL, "AssaultCube"));

void DrawConsoleText(HDC hdc, LPCWSTR text, int X, int Y, COLORREF color, int fontSize = 20, LPCWSTR fontName = L"Verdana") {
	if (hdc == NULL) return;


	
	HFONT hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, fontName);
	if (!hFont) return;

	SIZE size;
	GetTextExtentPoint32(hdc, text, lstrlenW(text), &size);

	int newX = X - (size.cx / 2);
	int newY = Y - (size.cy / 2);

	SelectObject(hdc, hFont);
	SetTextColor(hdc, color);
	SetBkMode(hdc, TRANSPARENT);


	
	TextOut(hdc, newX, newY, text, lstrlen(text));

	DeleteObject(hFont);
}

void DrawLine(float StartX, float StartY, float EndX, float EndY, COLORREF color, int thick = 1)
{
	int a, b = 0;
	HPEN hOPen;
	HPEN hNPen = CreatePen(PS_SOLID, thick, color);// penstyle, width, color
	hOPen = (HPEN)SelectObject(hdc, hNPen);
	MoveToEx(hdc, StartX, StartY, NULL); //start
	a = LineTo(hdc, EndX, EndY); //end
	DeleteObject(SelectObject(hdc, hOPen));
}

void DrawCircle(HDC hdc, int xCenter, int yCenter, int radius)
{

	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);


	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(155, 155, 155));

	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

	Ellipse(hdc, xCenter - radius, yCenter - radius, xCenter + radius, yCenter + radius);

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);

	DeleteObject(hPen);
}


void msgBox() {

	const char* command = "cscript //nologo C:\\main\\alert1.vbs";
	system(command);

}

bool isCharArrayEmpty(const char arr[20]) {
	return std::all_of(arr, arr + 20, [](char c) { return c == '\0'; });
}

void waitClose() {
	while (true) {
		if (GetAsyncKeyState(VK_END) & 0x8000) {
			const char* command = "cscript //nologo C:\\main\\alert2.vbs";
			system(command);
			exit(0);
		}
		Sleep(100);
	}
}

bool IsWindowActive(const char* windowTitle) {
	HWND hwnd = FindWindowA(NULL, windowTitle);

	return (hwnd == GetForegroundWindow());
}

float DegToRad(float input) {
	return input * (M_PI / 180);;
}

void aim(HANDLE hProcess, int baseAddr) {

	int meAddr;
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + basePtrOffset::localPlayerOffset), &meAddr, sizeof(meAddr), nullptr);

	int entListAddr;
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + basePtrOffset::EntityListOffset), &entListAddr, sizeof(entListAddr), nullptr);

	
	Entity nearEnt;
	while (true) {
		if (aimThreadRunning) {
			
			
			if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {

				std::vector <Entity> entities = getAllEntities(hProcess, baseAddr);
				
				if (fov360) {
					nearEnt = getNearestEntitie(hProcess, baseAddr, entities, fFire, 0, -1);
				}
				else {
					nearEnt = getNearestEntitie(hProcess, baseAddr, entities, fFire, 0, fovRad - fovRad / 3.3);
				}
	
				
				

				if (isCharArrayEmpty(nearEnt.name)) {
					continue;
				}

				while (GetAsyncKeyState(VK_RBUTTON) & 0x8000){
					
					float myposX, myposY, myposZHead;

					ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::xCordOffset), &myposX, sizeof(myposX), nullptr);
					ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::yCordOffset), &myposY, sizeof(myposY), nullptr);
					ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::zHeadCordOffset), &myposZHead, sizeof(myposZHead), nullptr);

					Vector3 myUpdatedPos = { myposX, myposY, myposZHead };

					float entposX, entposY, entposZHead, entAlive;
					ReadProcessMemory(hProcess, (BYTE*)(nearEnt.baseAddr + PlayerClass::xCordOffset), &entposX, sizeof(entposX), nullptr);
					ReadProcessMemory(hProcess, (BYTE*)(nearEnt.baseAddr + PlayerClass::yCordOffset), &entposY, sizeof(entposY), nullptr);
					ReadProcessMemory(hProcess, (BYTE*)(nearEnt.baseAddr + PlayerClass::zHeadCordOffset), &entposZHead, sizeof(entposZHead), nullptr);
					ReadProcessMemory(hProcess, (BYTE*)(nearEnt.baseAddr + PlayerClass::aliveOffset), &entAlive, sizeof(entAlive), nullptr);

					if (entAlive) {
						std::this_thread::sleep_for(std::chrono::milliseconds(aimDelay));
						break;
					}

					Vector3 updatedNearEntPos = { entposX, entposY, entposZHead - headOffset};
					Angle newAimAngle = CalculateAngle(myUpdatedPos, updatedNearEntPos);
					WriteProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::yawViewOffset), &newAimAngle.yaw, sizeof(newAimAngle.yaw), nullptr);
					WriteProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::pitchViewOffset), &newAimAngle.pitch, sizeof(newAimAngle.pitch), nullptr);
				}
			}
		}

		else {
			Sleep(1000);
		}
	}

}

void esp(HANDLE hProcess, int baseAddr) {

	std::vector <Line> lineDrawBuffer = {};
	int myAddr;
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + basePtrOffset::localPlayerOffset), &myAddr, sizeof(myAddr), nullptr);

	while (true) {
		

		if (espThreadRunning) {

			int myTeam = 0;
			ReadProcessMemory(hProcess, (BYTE*)(myAddr + PlayerClass::teamOffset), &myTeam, sizeof(myTeam), nullptr);

			std::vector <float> mtx = getViewMatrix(hProcess, baseAddr);
			std::vector <Entity> entities = getAllEntities(hProcess, baseAddr);

			
			int red, green, blue;
			for (Entity ent : entities) {
				if (!ent.aliveFlag) {
					continue;
				}
				if (ent.teamID == myTeam) {
					if (!drawTeam) {
						continue;
					}
					red = static_cast<int>(FriendPen[0] * 255.0f + 0.5f);
					green = static_cast<int>(FriendPen[1] * 255.0f + 0.5f);
					blue = static_cast<int>(FriendPen[2] * 255.0f + 0.5f);
				}
				else {
					red = static_cast<int>(EnemyPen[0] * 255.0f + 0.5f);
					green = static_cast<int>(EnemyPen[1] * 255.0f + 0.5f);
					blue = static_cast<int>(EnemyPen[2] * 255.0f + 0.5f);
				}

				COLORREF colorRef = RGB(red, green, blue);


				POINT MidBodyScreenCords = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - 2}, WindowW, WindowH);
				
				if (!(MidBodyScreenCords.x == -999 && MidBodyScreenCords.y == -999)) {

					if (box3D) {
						POINT RightTop = WordlToScreen(mtx, { float(ent.posX) - float(1.2), ent.posY + 1, float(ent.posZHead) + float(0.85) }, WindowW, WindowH);
						POINT LeftBot = WordlToScreen(mtx, { float(ent.posX) + float(1.2), ent.posY + 1, ent.posZHead - 4 }, WindowW, WindowH);
						POINT RightTop2 = WordlToScreen(mtx, { float(ent.posX) - float(1.2), ent.posY - 1, float(ent.posZHead) + float(0.85) }, WindowW, WindowH);
						POINT LeftBot2 = WordlToScreen(mtx, { float(ent.posX) + float(1.2), ent.posY - 1, ent.posZHead - 4 }, WindowW, WindowH);

						//behind box
						DrawLine(RightTop.x, RightTop.y, RightTop.x, LeftBot.y, colorRef);
						DrawLine(LeftBot.x, RightTop.y, LeftBot.x, LeftBot.y, colorRef);
						DrawLine(LeftBot.x, LeftBot.y, RightTop.x, LeftBot.y, colorRef);
						DrawLine(LeftBot.x, RightTop.y, RightTop.x, RightTop.y, colorRef);

						//front box
						DrawLine(RightTop2.x, RightTop2.y, RightTop2.x, LeftBot2.y, colorRef);
						DrawLine(LeftBot2.x, RightTop2.y, LeftBot2.x, LeftBot2.y, colorRef);
						DrawLine(LeftBot2.x, LeftBot2.y, RightTop2.x, LeftBot2.y, colorRef);
						DrawLine(LeftBot2.x, RightTop2.y, RightTop2.x, RightTop2.y, colorRef);

						//wide boxes
						DrawLine(LeftBot.x, RightTop.y, LeftBot2.x, RightTop2.y, colorRef);
						DrawLine(LeftBot.x, LeftBot.y, LeftBot2.x, LeftBot2.y, colorRef);
						DrawLine(RightTop.x, RightTop.y, RightTop2.x, RightTop2.y, colorRef);
						DrawLine(RightTop.x, LeftBot.y, RightTop2.x, LeftBot2.y, colorRef);
					}
					else {
						POINT overHead = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead + 1}, WindowW, WindowH);
						POINT underLeg = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - 4}, WindowW, WindowH);


						float delta = underLeg.y - overHead.y;

						delta /= 4;


						DrawLine(overHead.x, overHead.y, overHead.x - delta, overHead.y, colorRef, 1.5);
						DrawLine(overHead.x, overHead.y, overHead.x + delta, overHead.y, colorRef, 1.5);

						DrawLine(underLeg.x, underLeg.y, underLeg.x - delta, underLeg.y, colorRef, 1.5);
						DrawLine(underLeg.x, underLeg.y, underLeg.x + delta, underLeg.y, colorRef, 1.5);

						DrawLine(overHead.x + delta, overHead.y, underLeg.x + delta, underLeg.y, colorRef, 1.5);
						DrawLine(overHead.x - delta, overHead.y, underLeg.x - delta, underLeg.y, colorRef, 1.5);
					}

					

					if (drawSnap) {
						POINT underLegScreenpos = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - 4 }, WindowW, WindowH);
						

						DrawLine(underLegScreenpos.x, underLegScreenpos.y, WindowW / 2, WindowH, colorRef, 2);
					}

					if (drawNick) {
						POINT underLegScreenpos = WordlToScreen(mtx, { ent.posX, ent.posY, float(ent.posZHead) + float(1) }, WindowW, WindowH);

						WCHAR unicodeString[20];
						MultiByteToWideChar(CP_ACP, 0, ent.name, -1, unicodeString, 20);

						DrawConsoleText(hdc, unicodeString, underLegScreenpos.x, underLegScreenpos.y - 10, colorRef, 12);
					}
					
					if (drawHp) {
						POINT overHead = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead + 1 }, WindowW, WindowH);
						POINT underLeg = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - 4 }, WindowW, WindowH);

						float delta = underLeg.y - overHead.y;

						float deltaToWidth = delta / 3.25;

						DrawLine(overHead.x + deltaToWidth, overHead.y, underLeg.x + deltaToWidth, underLeg.y, 0x00000, 3.5);
						


						if (ent.health >= 90){
							DrawLine(overHead.x + deltaToWidth, overHead.y, underLeg.x + deltaToWidth, underLeg.y, 0x00ff00, 3.5);
						}

						else if (ent.health >= 80) {
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead + float(0.45) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x00ff00, 3.5);
						}

						else if (ent.health >= 70) {
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - float(0.1) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x008000, 3.5);
						}

						else if (ent.health >= 60) {
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - float(0.65) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x008000, 3.5);
						}

						else if (ent.health >= 50) {
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - float(1.2) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x30d5c8, 3.5);
						}

						else if (ent.health >= 40) {
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - float(1.75) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x30d5c8, 3.5);
						}

						else if (ent.health >= 30) {
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - float(2.3) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x42AAFF, 3.5);
						}

						else if (ent.health >= 20) {
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - float(2.85) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x42AAFF, 3.5);
						}

						else if (ent.health >= 10) {
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - float(3.4) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x0000FF, 3.5);
						}

						else if (ent.health >= 0){
							POINT tempHeight = WordlToScreen(mtx, { ent.posX, ent.posY, ent.posZHead - float(3.95) }, WindowW, WindowH);
							DrawLine(tempHeight.x + deltaToWidth, tempHeight.y, underLeg.x + deltaToWidth, underLeg.y, 0x0000FF, 3.5);
						}

					}
					if (drawDist) {
						POINT underLegScreenpos = WordlToScreen(mtx, { ent.posX, ent.posY, float(ent.posZHead) - 4 }, WindowW, WindowH);

						float myX, myY, myZ;
						ReadProcessMemory(hProcess, (BYTE*)(myAddr + PlayerClass::xCordOffset), &myX, sizeof(myX), nullptr);
						ReadProcessMemory(hProcess, (BYTE*)(myAddr + PlayerClass::yCordOffset), &myY, sizeof(myY), nullptr);
						ReadProcessMemory(hProcess, (BYTE*)(myAddr + PlayerClass::zLegCordOffset), &myZ, sizeof(myZ), nullptr);

						double dist = calculateDistance(myX, myY, myZ, ent.posX, ent.posY, ent.posZLeg) / 2;

						int roundedDist = static_cast<int>(std::round(dist));
						std::wstring DistString = std::to_wstring(roundedDist);

						std::wstring templateString = L"[%fm.]";

						std::wstring finalString = templateString;
						finalString.replace(finalString.find(L"%f"), 2, DistString);
						LPCWSTR lpcwstrDist = finalString.c_str();
	
						DrawConsoleText(hdc, lpcwstrDist, underLegScreenpos.x, underLegScreenpos.y + 10, colorRef, 12);

					}
				}
			}
		}


		if (aimThreadRunning){
			if (drawFov && !fov360) {
				DrawCircle(hdc, WindowW / 2, WindowH / 2, fovRad);
			}
		}
		else if (!espThreadRunning && !espThreadRunning) {
			Sleep(1000);
		}

		
	}
}

void fly(HANDLE hProcess, int baseAddr) {
	float newXPos, newYPos, newZPos, newYaw, newPitch;

	int meAddr;
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + basePtrOffset::localPlayerOffset), &meAddr, sizeof(meAddr), nullptr);


	while (true) {


		if (flyThreadRunning) {


			ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::xCordOffset), &newXPos, sizeof(newXPos), NULL);
			ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::yCordOffset), &newYPos, sizeof(newYPos), NULL);
			ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::zHeadCordOffset), &newZPos, sizeof(newZPos), NULL);

			std::cout << newXPos << " " << newYPos << " " << newZPos << std::endl;

			// Чтение углов взгляда игрока
			ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::yawViewOffset), &newYaw, sizeof(newYaw), NULL);
			ReadProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::pitchViewOffset), &newPitch, sizeof(newPitch), NULL);

			float yawRad = newYaw * (M_PI / 180);

			float pitchRad = newPitch * (M_PI / 180);

			// Вектор направления движения в горизонтальной плоскости
			float dx = cos(yawRad) * 0.5;
			float dz = sin(yawRad) * 0.5;

			if (GetAsyncKeyState('W') & 0x8000) {
				newXPos += dx;
				newZPos += dz;
			}
			if (GetAsyncKeyState('S') & 0x8000) {
				newXPos -= dx;
				newZPos -= dz;
			}
			if (GetAsyncKeyState('A') & 0x8000) {
				newXPos -= dz;
				newZPos += dx;
			}
			if (GetAsyncKeyState('D') & 0x8000) {
				newXPos += dz;
				newZPos -= dx;
			}
			
			WriteProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::xCordOffset), &newXPos, sizeof(newXPos), NULL);
			WriteProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::yCordOffset), &newYPos, sizeof(newYPos), NULL);
			WriteProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::zHeadCordOffset), &newZPos, sizeof(newZPos), NULL);
			Sleep(5);

		}

		else {
			Sleep(1000);
		}

	}
}

void tp(HANDLE hProcess, int baseAddr) {

	Entity nearEnt;

	int meAddr;
	ReadProcessMemory(hProcess, (BYTE*)(baseAddr + basePtrOffset::localPlayerOffset), &meAddr, sizeof(meAddr), nullptr);


	while (true) {
		
		if (tpThreadRunning) {
			
			if (GetAsyncKeyState(0x46)) {
				std::vector <Entity> entities = getAllEntities(hProcess, baseAddr);

				if (fov360) {
					nearEnt = getNearestEntitie(hProcess, baseAddr, entities, tpFriend, 0, -1);
				}
				else {
					nearEnt = getNearestEntitie(hProcess, baseAddr, entities, tpFriend, 0, fovRad + fovRad / 3.3);
				}


				if (isCharArrayEmpty(nearEnt.name)) {
					continue;
				}

				int entAlive;
				float entposX, entposY, entposZHead;

				ReadProcessMemory(hProcess, (BYTE*)(nearEnt.baseAddr + PlayerClass::aliveOffset), &entAlive, sizeof(entAlive), nullptr);

				if (entAlive) {
					continue;
				}

				ReadProcessMemory(hProcess, (BYTE*)(nearEnt.baseAddr + PlayerClass::xCordOffset), &entposX, sizeof(entposX), nullptr);
				ReadProcessMemory(hProcess, (BYTE*)(nearEnt.baseAddr + PlayerClass::yCordOffset), &entposY, sizeof(entposY), nullptr);
				ReadProcessMemory(hProcess, (BYTE*)(nearEnt.baseAddr + PlayerClass::zLegCordOffset), &entposZHead, sizeof(entposZHead), nullptr);

				WriteProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::xCordOffset), &entposX, sizeof(entposX), nullptr);
				WriteProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::yCordOffset), &entposY, sizeof(entposX), nullptr);
				WriteProcessMemory(hProcess, (BYTE*)(meAddr + PlayerClass::zLegCordOffset), &entposZHead, sizeof(entposX), nullptr);

				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}


}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


float hue = 0.0f;

ImVec4 HSBToRGB(float hue, float saturation, float brightness) {
	float c = brightness * saturation;
	float x = c * (1 - std::abs(std::fmod(hue / 60.0f, 2) - 1));
	float m = brightness - c;

	if (0 <= hue && hue < 60) {
		return ImVec4(c + m, x + m, m, 1.0f);
	}
	else if (60 <= hue && hue < 120) {
		return ImVec4(x + m, c + m, m, 1.0f);
	}
	else if (120 <= hue && hue < 180) {
		return ImVec4(m, c + m, x + m, 1.0f);
	}
	else if (180 <= hue && hue < 240) {
		return ImVec4(m, x + m, c + m, 1.0f);
	}
	else if (240 <= hue && hue < 300) {
		return ImVec4(x + m, m, c + m, 1.0f);
	}
	else {
		return ImVec4(c + m, m, x + m, 1.0f);
	}
}

int selectedTab = 0;
char resolutionInput1[32] = { 0 };
char resolutionInput2[32] = { 0 };
ImVec4 rainbowColor = HSBToRGB(210.f, 1.0f, 1.0f);
bool confirmed = false;

void RenderUI() {
	

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	if (lgbtMenu) {
		hue += 0.2f;
		if (hue > 360.0f) {
			hue = 0.0f;
		}

		rainbowColor = HSBToRGB(hue, 1.0f, 1.0f);
		colors[ImGuiCol_SliderGrab] = rainbowColor;
		colors[ImGuiCol_CheckMark] = rainbowColor;
		colors[ImGuiCol_Text] = rainbowColor;
		
	}
	
	style.FrameBorderSize = 0.5f;
	style.FrameRounding = 2.0f;

	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.6f, 0.4f, 1.0f);

	ImGui::PushStyleColor(ImGuiCol_Border, rainbowColor);
	colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.5f, 0.8f, 0.5f);


	ImGuiIO& io = ImGui::GetIO();
	float fps = io.Framerate;

	ImGui::SetNextWindowSize(ImVec2(335, 300));
	ImGui::SetNextWindowPos(ImVec2(1000, 500), ImGuiCond_FirstUseEver);


	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

	ImGui::Begin("Tabbed Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	ImGui::PushItemWidth(120);
	if (ImGui::Button("ESP")) selectedTab = 0;
	ImGui::SameLine();
	if (ImGui::Button("AIM")) selectedTab = 1;
	ImGui::SameLine();
	if (ImGui::Button("MISC")) selectedTab = 2;
	ImGui::SameLine();
	if (ImGui::Button("MENU")) selectedTab = 3;
	ImGui::PopItemWidth();

	ImGui::Separator();

	if (selectedTab == 0) {
		ImGui::Checkbox("ESP toggle", &espThreadRunning);
		

		//colors
		ImVec4 Vec4colorEnemy = { EnemyPen[0], EnemyPen[1],  EnemyPen[2],  EnemyPen[3] };
		ImVec4 Vec4colorTeam = { FriendPen[0], FriendPen[1],  FriendPen[2],  FriendPen[3] };



		if (ImGui::ColorButton("Enemy color", Vec4colorEnemy)) {
			ImGui::CloseCurrentPopup();
			ImGui::OpenPopup("ColorPickerPopupEnemy");
		}
		ImGui::SameLine();
		ImGui::Text("Enemy color");

		if (ImGui::BeginPopup("ColorPickerPopupEnemy")) {
			if (ImGui::ColorPicker4("", (float*)&EnemyPen, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs)) {
			}
			ImGui::EndPopup();
		}

		
		if (ImGui::ColorButton("Team color", Vec4colorTeam)) {
			ImGui::CloseCurrentPopup();
			ImGui::OpenPopup("ColorPickerPopupTeam");
		}
		ImGui::SameLine();
		ImGui::Text("Team color");

		if (ImGui::BeginPopup("ColorPickerPopupTeam")) {
			if (ImGui::ColorPicker4("", (float*)&FriendPen, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs)) {
			}
			ImGui::EndPopup();
		}

		
		
		//colors
		ImGui::Dummy(ImVec2(0.0f, 4.0f));

		if (ImGui::RadioButton("2D Box", box3D == 0)) {
			box3D = false;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("3D Box", box3D == 1)) {
			box3D = true;
		}
		ImGui::Checkbox("Draw team", &drawTeam);
		ImGui::Checkbox("Snaplines", &drawSnap);
		ImGui::Checkbox("Health", &drawHp);
		ImGui::Checkbox("Distance", &drawDist);
	}

	else if (selectedTab == 1) {
		ImGui::Checkbox("AIM toggle", &aimThreadRunning);
		ImGui::Checkbox("Aim at team", &fFire);
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(100);
		if (!fov360) {
			ImGui::SliderFloat("Fov radius ", &fovRad, 5.0f, 250.0f);
			ImGui::SameLine();
		}
		
		ImGui::Checkbox("FOV 360 degree", &fov360);
		ImGui::Checkbox("Draw Fov", &drawFov);
		ImGui::SetNextItemWidth(100);
		ImGui::SliderInt("Delay after kill (mil sec)", &aimDelay, 0, 1000);
		
	}

	else if (selectedTab == 2) {
		ImGui::Checkbox("Tp to enemy [F]", &tpThreadRunning);

		if (tpThreadRunning) {
			float newPosX = ImGui::GetCursorPosX() + 30.0f;
			ImGui::SetCursorPosX(newPosX);
			ImGui::Checkbox("include team", &tpFriend);
		}
	}

	else if (selectedTab == 3) {

		ImGui::Text("change you game res");
		ImGui::SetNextItemWidth(100);
		ImGui::InputText("##ResolutionInput1", resolutionInput1, IM_ARRAYSIZE(resolutionInput1));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		ImGui::InputText("##ResolutionInput2", resolutionInput2, IM_ARRAYSIZE(resolutionInput2));

		if (ImGui::Button("change")) {
			WindowW = atoi(resolutionInput1);
			WindowH = atoi(resolutionInput2);
			confirmed = true;
		}
		ImGui::Dummy(ImVec2(0.0f, 8.0f));
		ImGui::Checkbox("LGBTQ+ menu", &lgbtMenu);

		ImGui::Dummy(ImVec2(0.0f, 20.0f));
		ImGui::Text("fr: %.1f", fps);
	}


	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}




LRESULT WINAPI WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam)) {
		return 0L;
	}

	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0L;
	}

	return DefWindowProc(window, msg, wParam, lParam);
}

ID3D11Device* device{ nullptr };
ID3D11DeviceContext* device_context{ nullptr };
IDXGISwapChain* swap_chain{ nullptr };
ID3D11RenderTargetView* render_targer_view{ nullptr };
D3D_FEATURE_LEVEL level{};

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {

	DWORD procId = GetProcId(L"ac_client.exe");

	if (!procId) {
		const char* command = "cscript //nologo C:\\main\\alert3.vbs";
		system(command);
		while (!procId) {
			procId = GetProcId(L"ac_client.exe");
			Sleep(500);
		}
	}
	

	
	std::thread t1(msgBox);
	std::thread t2(waitClose);

	int baseAddr = GetBaseAddress(procId);
	HANDLE hProcess = 0;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

	
	std::thread aimThrd(aim, hProcess, baseAddr);
	std::thread espThrd(esp, hProcess, baseAddr);
	std::thread tpThrd(tp, hProcess, baseAddr);

	//
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* stream;
	freopen_s(&stream, "CON", "w", stdout);

	bool running = true;

	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = instance;
	wc.lpszClassName = L"External Overlay Class";

	RegisterClassExW(&wc);



	while (running) {
		HWND window = NULL;

		if (GetAsyncKeyState(VK_INSERT) & 1) {
			

			window = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, L"Overlay", WS_POPUP, 0, 0, screenWidth, screenHeight-20, nullptr, nullptr, wc.hInstance, nullptr);


			SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
			{
				RECT client_area{};
				GetClientRect(window, &client_area);

				RECT window_area{};
				GetWindowRect(window, &window_area);

				POINT diff{};
				ClientToScreen(window, &diff);

				MARGINS margins{
					window_area.left + (diff.x - window_area.left),
					window_area.top + (diff.y - window_area.top),
					client_area.right,
					client_area.bottom
				};

				DwmExtendFrameIntoClientArea(window, &margins);
			}

			DXGI_SWAP_CHAIN_DESC sd{};
			sd.BufferDesc.RefreshRate.Numerator = 60U;
			sd.BufferDesc.RefreshRate.Denominator = 1U;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.SampleDesc.Count = 1U;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 2U;
			sd.OutputWindow = window;
			sd.Windowed = TRUE;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			constexpr D3D_FEATURE_LEVEL levels[2]{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_0
			};

			D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0U, levels, 2U, D3D11_SDK_VERSION, &sd, &swap_chain, &device, &level, &device_context);

			ID3D11Texture2D* back_buffer = nullptr;
			swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

			if (back_buffer) {
				device->CreateRenderTargetView(back_buffer, nullptr, &render_targer_view);
				back_buffer->Release();
			}
			else {
				return 1;
			}

			ShowWindow(window, cmd_show);
			UpdateWindow(window);

			ImGui::CreateContext();
			ImGui::StyleColorsDark();

			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX11_Init(device, device_context);

			//
			ImGuiIO& io = ImGui::GetIO();
			

			Sleep(200);
			while (!GetAsyncKeyState(VK_INSERT) & 1) {

				MSG msg;
				while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);

					if (msg.message == WM_QUIT) {

					}

				}

				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();

				ImGui::NewFrame();

				//

				RenderUI();

				//

				ImGui::Render();

				constexpr float color[4]{ 0.f, 0.f, 0.f, 0.f };
				device_context->OMSetRenderTargets(1U, &render_targer_view, nullptr);
				device_context->ClearRenderTargetView(render_targer_view, color);
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

				swap_chain->Present(1U, 0U);
			}
			DestroyWindow(window);
		}
		
	}


	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	if (swap_chain) {
		swap_chain->Release();
	}

	if (device_context) {
		device_context->Release();
	}

	if (device) {
		device->Release();
	}

	if (render_targer_view) {
		render_targer_view->Release();
	}


	return 0;
}