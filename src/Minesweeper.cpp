/*
 * Minesweeper.cpp
 *
 *  Created on: 2010-10-12
 *      Author: james
 */
#include <kern/console.h>
#include <kern/system.hpp>
#include <io/Keyboard.h>
using namespace LunOS;
using namespace LunOS::IO;

namespace Minesweeper
{
#define GameWidth 4
#define GameHeight 4

	bool TileOpened[GameWidth][GameHeight];
	bool ContainsMine[GameWidth][GameHeight];
	unsigned char MinesAround[GameWidth][GameHeight];
	bool FirstTurn = false;

	void DrawGame()
	{
		int i,j;
		clearConsole();
		for(j = 0; j < GameHeight; j++)
		{
			for(i = 0; i < GameWidth; i++)
			{
				if(TileOpened[i][j])
				{
					printf("%c",MinesAround[i][j]);
				}
				else
				{
					printf("X");
				}
			}
			printf("\r\n");
		}
	}

	void DrawFinalGame()
	{
		int i,j;
		clearConsole();
		for(j = 0; j < GameHeight; j++)
		{
			for(i = 0; i < GameWidth; i++)
			{
				if(ContainsMine[i][j])
				{
					printf("*");
					continue;
				}
				if(TileOpened[i][j])
				{
					printf("%c",MinesAround[i][j]);
				}
				else
				{
					printf("X");
				}
			}
			printf("\r\n");
		}
	}

	void MineSweeperInitialize()
	{
		int i, j;
		FirstTurn = true;
		for(i = 0; i < GameWidth; i++)
		{
			for(j = 0; j < GameHeight; j++)
			{
				TileOpened[i][j] = false;
				ContainsMine[i][j] = false;
				MinesAround[i][j] = 0;
			}
		}
	}

	bool GetSelectedSpace(int* x, int* y, Keyboard* keyboard)
	{
		int pos = 0;
		bool done = false;
		*x = -1;
		*y = -1;
		while(!done)
		{
			KeyEvent k = keyboard->ReadKeyEvent();
			if(k.type == KEY_DOWN)
			{
				if(*x != -1 && *y != -1 && k.key == VK_Enter)
				{
					break;
				}
				else if(k.key >= VK_1 && k.key <= VK_9)
				{
					if(pos == 0)
					{
						*x = k.key - '0' - 1;
						printf("X:%c\n", k.key);
						pos++;
					}
					else
					{
						*y = k.key - '0' - 1;
						printf("Y:%c\n", k.key);
						break;
					}
				}
			}
		}
		return true;
	}

	void UpdateVisableTiles(int x, int y)
	{
		int i, j;
		TileOpened[x][y] = true;
		if(MinesAround[x][y] != '_')
		{
			// Then we do nothing
		}
		else
		{
			for(i = -1; i <= 1; i++)
			{
				for(j = -1; j <= 1; j++)
				{
					if(x + i >= 0 && x + i < GameWidth
							&& y + j >= 0 && y+j < GameHeight)
					{
						if(TileOpened[x+i][y+j]) continue;
						if(MinesAround[x + i][y + j] == '_')
						{
							UpdateVisableTiles(x + i, y + j);
						}
						else
						{
							TileOpened[x+i][y+j] = true;
						}
					}
				}
			}
		}
	}

	bool SelectSpace(int x, int y, bool* win)
	{

		int i, j;
		if(ContainsMine[x][y])
		{
			// we lost the game
			*win = false;
			return false;
		}
		UpdateVisableTiles(x, y);
		for(i = 0; i < GameWidth; i++)
		{
			for(j = 0; j < GameHeight; j++)
			{
				if(!TileOpened[i][j] && !ContainsMine[i][j])
				{
					*win = false;
					return true;
				}
			}
		}
		*win = true;
		return false;
	}

	void SetupGame(int x, int y)
	{
		int i, j, minesLeft = 3;
		unsigned int CurrentPos = 0;
		// put down the mines
		for(;minesLeft > 0;)
		{
			unsigned int howMany = (unsigned int)timer_ticks;
			howMany = (howMany + CurrentPos) % (GameWidth * GameHeight);
			CurrentPos += howMany;
			int mineX = howMany % GameWidth;
			int mineY = howMany / GameWidth;
			if(((mineX != x) || (mineY != y)) && (!ContainsMine[mineX][mineY]))
			{
				ContainsMine[mineX][mineY] = true;
				minesLeft--;
			}
		}
		// Now figure out how many mines are around each tile
		for(i = 0; i < GameWidth; i++)
		{
			for(j = 0; j < GameHeight; j++)
			{
				int minesAround = 0;
				if(ContainsMine[i][j]) minesAround++;
				if(i > 0)
				{
					if(j > 0)
					{
						if(ContainsMine[i - 1][j - 1]) minesAround++;
					}
					if(j < GameHeight - 1)
					{
						if(ContainsMine[i - 1][j + 1]) minesAround++;
					}
					if(ContainsMine[i - 1][j]) minesAround++;
				}
				if(i < GameWidth - 1)
				{
					if(j > 0)
					{
						if(ContainsMine[i + 1][j - 1]) minesAround++;
					}
					if(j < GameHeight - 1)
					{
						if(ContainsMine[i + 1][j + 1]) minesAround++;
					}
					if(ContainsMine[i + 1][j]) minesAround++;
				}
				if(j > 0)
				{
					if(ContainsMine[i][j - 1]) minesAround++;
				}
				if(j < GameHeight - 1)
				{
					if(ContainsMine[i][j + 1]) minesAround++;
				}

				if(minesAround == 0)
				{
					MinesAround[i][j] = '_';
				}
				else
				{
					MinesAround[i][j] = '0' + minesAround;
				}
			}
		}
	}

	void PlayGame()
	{
		bool win;
		bool GameOver = false;
		Keyboard* keys = Keyboard::GetKeyboardStream();
		while(!GameOver)
		{
			int x, y;
			DrawGame();
			GetSelectedSpace(&x, &y, keys);
			if(FirstTurn)
			{
				SetupGame(x,y);
				FirstTurn = false;
			}
			GameOver = !SelectSpace(x,y, &win);
		}
		keys->Close();
		DrawFinalGame();
		if(win)
		{
			printf("Congratulations you are win!\n");
		}
		else
		{
			printf("You Lose!\n");
		}
	}

	void MineSweeperMain(int argc, unsigned char** argv)
	{
		MineSweeperInitialize();
		PlayGame();
	}
}
