#include <iostream>
#include <vector>
#include <tuple>
#include <utility>
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <Windows.h>
using namespace std;

// visual studio 콘솔 사이즈 설정 - 120 x 40

const float PI = 3.14159f;
float convertRad(float deg) { return deg * PI / 180; };
float dirVX(float rad) { return sinf(rad); };
float dirVY(float rad) { return cosf(rad); };

// 콘솔
const int CONSOLE_W = 120;			
const int CONSOLE_H = 40;	
wchar_t* console;
HANDLE buffer;
DWORD bw = 0;
int convertConsoleCrd(float x, float y) { return static_cast<int>(y) * static_cast<int>(CONSOLE_W) + static_cast<int>(x); };

// 프레임 간격
chrono::time_point<chrono::system_clock> tp_s;
chrono::time_point<chrono::system_clock> tp_e;
float dt;

// 월드
wstring world;
const float WORLD_SIZE = 16.0f;
int convertWorldCrd(float x, float y) { return static_cast<int>(y) * static_cast<int>(WORLD_SIZE) + static_cast<int>(x); };

// 플레이어
const float FOV = convertRad(45);	
const float SPEED = 3.5f;
float playerX = 14.0f;			
float playerY = 5.0f;
float playerA = 0.0f;

void init()
{
	console = new wchar_t[CONSOLE_W * CONSOLE_H];
	buffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(buffer);

	tp_s = chrono::system_clock::now();
	tp_e = chrono::system_clock::now();
}

void frameUpdate()
{
	tp_e = chrono::system_clock::now();
	chrono::duration<float> duration = tp_e - tp_s;
	tp_s = tp_e;
	dt = duration.count();
}

void createWorld()
{
	// # 벽 . 땅
	world += L"################";
	world += L"#..............#";
	world += L"#.......########";
	world += L"#..............#";
	world += L"#......##......#";
	world += L"#......##......#";
	world += L"#..............#";
	world += L"###............#";
	world += L"##.............#";
	world += L"#......####..###";
	world += L"#......#.......#";
	world += L"#......#.......#";
	world += L"#..............#";
	world += L"#......#########";
	world += L"#..............#";
	world += L"################";
}

tuple<bool, float> castRayToWall(float dirX, float dirY, float step)
{
	bool isHitWall = false;
	bool isCorner = false;
	float corner = convertRad(0.4f);
	float distance = 0.0f;
	
	while (!isHitWall && distance < WORLD_SIZE)
	{
		distance += step;
		int rayX = static_cast<int>(playerX + dirX * distance);
		int rayY = static_cast<int>(playerY + dirY * distance);

		if (rayX < 0 || rayX >= WORLD_SIZE || rayY < 0 || rayY >= WORLD_SIZE)
		{
			isHitWall = true;
			distance = WORLD_SIZE;
		}
		else
		{
			if (world.c_str()[convertWorldCrd(rayX, rayY)] == '#')
			{
				isHitWall = true;
				vector<pair<float, float>> p;
				for (int x = 0; x < 2; x++)
					for (int y = 0; y < 2; y++)
					{
						float cx = static_cast<float>(rayX) - playerX + x;
						float cy = static_cast<float>(rayY) - playerY + y;
						float d = sqrt(cx * cx + cy * cy);
						float ip = (dirX * cx / d) + (dirY * cy / d);
						p.push_back(make_pair(d, ip));
					}
				sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });
				isCorner = acos(p.at(0).second) < corner || acos(p.at(1).second) < corner || acos(p.at(2).second) < corner;
			}
		}
	}

	return { isCorner, distance };
}

void render(int x, int y, float distanceToWall, bool isCorner)
{
	short p = ' ';
	int sky = CONSOLE_H / 2.0f - CONSOLE_H / distanceToWall;
	int floor = CONSOLE_H - sky;

	if (y <= sky)
	{
		p = ' ';
	}
	else if (y > sky && y <= floor)
	{
		if		(distanceToWall <= WORLD_SIZE / 4.0f)	p = 0x2588;
		else if (distanceToWall < WORLD_SIZE / 3.0f)	p = 0x2593;
		else if (distanceToWall < WORLD_SIZE / 2.0f)	p = 0x2592;
		else if (distanceToWall < WORLD_SIZE)			p = 0x2591;
		if (isCorner)									p = ' ';
	}
	else if (y > floor)
	{
		float b = 1.0f - (y - CONSOLE_H / 2.0f) / (CONSOLE_H / 2.0f);
		if		(b < 0.25f)								p = '#';
		else if (b < 0.5f)								p = 'x';
		else if (b < 0.75f)								p = '.';
		else if (b < 0.9f)								p = '-';
	}

	console[convertConsoleCrd(x, y)] = p;
}

void control()
{
	if (GetAsyncKeyState(static_cast<unsigned short>('A')) & 0x8000)
		playerA -= (SPEED * 0.5f) * dt;
	if (GetAsyncKeyState(static_cast<unsigned short>('D')) & 0x8000)
		playerA += (SPEED * 0.5f) * dt;
	if (GetAsyncKeyState(static_cast<unsigned short>('W')) & 0x8000)
	{
		playerX += dirVX(playerA) * SPEED * dt;
		playerY += dirVY(playerA) * SPEED * dt;
		if (world.c_str()[convertWorldCrd(playerX, playerY)] == '#')
		{
			playerX -= dirVX(playerA) * SPEED * dt;
			playerY -= dirVY(playerA) * SPEED * dt;
		}
	}
	if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
	{
		playerX -= dirVX(playerA) * SPEED * dt;
		playerY -= dirVY(playerA) * SPEED * dt;
		if (world.c_str()[convertWorldCrd(playerX, playerY)] == '#')
		{
			playerX += dirVX(playerA) * SPEED * dt;
			playerY += dirVY(playerA) * SPEED * dt;
		}
	}
}

int main()
{
	init();
	createWorld();

	while (1)
	{
		frameUpdate();
		control();

		for (int x = 0; x < CONSOLE_W; x++)
		{
			float rayA = (playerA - FOV/2.0f) + (static_cast<float>(x) / CONSOLE_W) * FOV;
			float rayDirX = dirVX(rayA);
			float rayDirY = dirVY(rayA);
			float rayStep = 0.1f;

			tuple<bool, float> rtw = castRayToWall(rayDirX, rayDirY, rayStep);
			const bool isCorner = get<0>(rtw);
			const float distanceToWall = get<1>(rtw);

			for (int y = 0; y < CONSOLE_H; y++)
				render(x, y, distanceToWall, isCorner);
		}

		WriteConsoleOutputCharacter(buffer, console, CONSOLE_W * CONSOLE_H, { 0, 0 }, &bw);
	}

	return 0;
}