// FPS++.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
/*
	This program expects the console dimensions to be set to 
	120 Columns by 40 Rows. I recommend a small font "Consolas" at size 16. You can do this
	by running the program, and right clicking on the console title bar, and specifying 
	the properties. You can also choose to default to them in the future.
	
	Controls: A = Turn Left, D = Turn Right, W = Walk Forwards, S = Walk Backwards
*/

#include <Windows.h>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>

using namespace std;

int screenWidth = 120;
int screenHeight = 40;

/* PLAYER STRUCTURE */
float posX = 8.0f; // X position
float posY = 8.0f; // Y position
float posA = 8.0f; //Angle of player looking at

int mapHeight = 16;
int mapWidth = 16;

float ff0V = 3.14159 / 4.0;
float depth = 16.0f;

int main()
{
	//Create Screen Buffer
	wchar_t* screen = new wchar_t[screenWidth * screenHeight];
	HANDLE handleConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(handleConsole);
	DWORD dwordBytesWritten = 0;

	wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#........#.....#";
	map += L"#......#####...#";
	map += L"#........#.....#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..........#####";
	map += L"#..............#";
	map += L"################";


	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	//Game Loop
	while (1)
	{
		tp2 = chrono::system_clock::now();
		chrono::duration<float> chronoElapsedTime = tp2 - tp1;
		tp1 = tp2;
		float elapsedTime = chronoElapsedTime.count();



		/**
		** Player Controls
		**/
		// Handle CCW Rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
		{
			posA -= (0.8f * elapsedTime);
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
		{
			posA += (0.8f * elapsedTime);
		}

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			posX += sinf(posA) * 5.0f * elapsedTime;
			posY += cosf(posA) * 5.0f * elapsedTime;

			if (map[(int)posY * mapWidth + (int)posX] == '#')
			{
				posX -= sinf(posA) * 5.0f * elapsedTime;
				posY -= cosf(posA) * 5.0f * elapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			posX -= sinf(posA) * 5.0f * elapsedTime;
			posY -= cosf(posA) * 5.0f * elapsedTime;

			if (map[(int)posY * mapWidth + (int)posX] == '#')
			{
				posX += sinf(posA) * 5.0f * elapsedTime;
				posY += cosf(posA) * 5.0f * elapsedTime;
			}
		}



		for (int x = 0; x < screenWidth; x++)
		{
			//Calculate of projected ray angle into world space
			float rayAngle = (posA - ff0V / 2.0f) + ((float)x / (float)screenWidth * ff0V);

			//Distance from the wall and player
			float distanceWall = 0;
			
			float eyeX = sinf(rayAngle); //Unit Vector for ray in player space
			float eyeY = cosf(rayAngle);

			bool hitWall = false; // Set when ray hits wall block
			bool boundary = false;		// Set when ray hits boundary between two wall blocks
			while (!hitWall && distanceWall < depth)
			{

				distanceWall += 0.1f;

				int TestHitX = (int)(posX + eyeX * distanceWall);
				int TestHitY = (int)(posY + eyeY * distanceWall);

				//Test if ray is out of bounds
				if (TestHitX < 0 || TestHitX >= mapWidth || TestHitY < 0 || TestHitY >= mapHeight)
				{
					hitWall = true; // Set Distance to maximum depth
					distanceWall = depth;
				}
				else
				{
					//Ray is inbounds to test if ray cell is a wall block
					if (map[TestHitY * mapWidth + TestHitX] == '#')
					{
						hitWall = true;

						vector<pair<float, float>> p; // distance, dot

						// Test each corner of hit tile, storing the distance from
						// the player, and the calculated dot product of the two rays
						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								float vy = (float)TestHitY + ty - posY;
								float vx = (float)TestHitX + tx - posX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (eyeX * vx / d) + (eyeY * vy / d);

								p.push_back(make_pair(d, dot));

							}

						// Sort Pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

						// First two/three are closest (we will never see all four)
						float bound = 0.01;
						if (acos(p.at(0).second) < bound) boundary = true;
						if (acos(p.at(1).second) < bound) boundary = true;
						if (acos(p.at(2).second) < bound) boundary = true;

					}
				}
			}

			//Calculate distance to ceiling and floor
			int ceiling = (float)(screenHeight / 2.0f) - screenHeight / ((float)distanceWall);
			int floor = screenHeight - ceiling;

			short shade = ' ';

			if (distanceWall <= depth / 4.0f)		shade = 0x2588; //Very Close
			else if (distanceWall <= depth / 3.0f)	shade = 0x2593;
			else if (distanceWall <= depth / 2.0f)	shade = 0x2592;
			else if (distanceWall <= depth)			shade = 0x2591;
			else									shade = ' '; //Too far away

			if (boundary)			shade = ' '; // Black it out


			for (int y = 0; y < screenHeight; y++)
			{
				if (y <= ceiling)
					screen[y * screenWidth + x] = ' ';
				else if (y > ceiling && y <= floor)
					screen[y * screenWidth + x] = shade;
				else // floor
				{
					//Shade floor based on distance
					float b = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));

					if (b < 0.25f)		shade = '#';
					else if (b < 0.5f)	shade = 'x';
					else if (b < 0.75f)	shade = '.';
					else if (b < 0.9f)	shade = '-';
					else				shade = ' ';

					screen[y * screenWidth + x] = shade;
				}
			}

		}
	
		// Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", posX, posY, posA, 1.0f / elapsedTime);

		// Display Map
		for (int nx = 0; nx < mapWidth; nx++)
			for (int ny = 0; ny < mapWidth; ny++)
			{
				screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth + nx];
			}
		screen[((int)posX + 1) * screenWidth + (int)posY] = 'P';

		// Display Frame
		screen[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(handleConsole, screen, screenWidth* screenHeight, { 0,0 }, & dwordBytesWritten);

	}

	return 0;
}
