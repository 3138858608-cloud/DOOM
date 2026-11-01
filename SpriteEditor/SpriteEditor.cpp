#include<iostream>
#include<string>
#include <vector>
#include <windows.h>
using namespace std;

#include "olcConsoleGameEngine.h"

class SpriteEditor :public olcConsoleGameEngine {
public:
	SpriteEditor() {
		m_sAppName = L"Sprite Editor";
	}
private:
	int nPosX = 0;
	int nPosY = 0;
	int nOffsetX = 0;
	int nOffsetY = 0;
	int nZoom = 0;
	int nCurrentGlyph = PIXEL_SOLID;
	int nCurrentColourFG = FG_RED;
	int nCurrentColourBG = FG_BLACK;
	olcSprite* sprite = nullptr;
	wstring sCurrentSpriteFile;
	// 单个像素的状态
	struct PixelState {
		int x, y;
		wchar_t glyph;
		short colour;
	};
	// 一次编辑操作：修改前和修改后的状态
	struct EditAction {
		PixelState before;
		PixelState after;
	};
	// 编辑历史
	vector<EditAction> history;
	size_t historyPos = 0;
	// 操作提示
	wstring sStatus = L"Ready";
	float fStatusTime = 0.0f;
	// 设置像素状态
	void ApplyPixel(const PixelState& p) {
		if (sprite == nullptr)
			return;

		sprite->SetGlyph(p.x, p.y, p.glyph);
		sprite->SetColour(p.x, p.y, p.colour);
	}

	// 修改一个像素，并记录历史
	void EditPixel(int x, int y, wchar_t glyph, short colour) {
		if (sprite == nullptr)
			return;

		if (x < 0 || x >= sprite->nWidth ||
			y < 0 || y >= sprite->nHeight)
			return;

		PixelState before{
			x, y,
			sprite->GetGlyph(x, y),
			(short)sprite->GetColour(x, y)
		};

		// 如果像素没有变化，就不记录历史
		if (before.glyph == glyph && before.colour == colour)
			return;

		PixelState after{ x, y, glyph, colour };

		// 如果之前撤销过操作，删除后续的重做历史
		history.resize(historyPos);

		history.push_back({ before, after });
		historyPos = history.size();

		ApplyPixel(after);
	}

	// 撤销
	void Undo() {
		if (historyPos == 0)
			return;

		--historyPos;
		ApplyPixel(history[historyPos].before);

		sStatus = L"Undo";
		fStatusTime = 2.0f;
	}

	// 重做
	void Redo() {
		if (historyPos >= history.size())
			return;

		ApplyPixel(history[historyPos].after);
		++historyPos;

		sStatus = L"Redo";
		fStatusTime = 2.0f;
	}
protected:
	virtual bool OnUserCreate() {
		sprite = new olcSprite(32, 32);
		sCurrentSpriteFile = L"fps_fireball1.spr";
		return TRUE;
	}
	virtual bool OnUserUpdate(float felapsedTime) {
		if (fStatusTime > 0.0f) {
			fStatusTime -= felapsedTime;

			if (fStatusTime <= 0.0f) {
				fStatusTime = 0.0f;
				sStatus = L"Ready";
			}
		}
		//Zooming
		if (m_keys[VK_PRIOR].bReleased)nZoom <<= 1;
		if (m_keys[VK_NEXT].bReleased) nZoom >>= 1;
		if (nZoom > 32)nZoom = 32;
		if (nZoom < 2)nZoom = 2;
		//Brushes
		if (m_keys[VK_F1].bReleased)nCurrentGlyph = PIXEL_SOLID;
		if (m_keys[VK_F2].bReleased)nCurrentGlyph = PIXEL_THREEQUARTERS;
		if (m_keys[VK_F3].bReleased)nCurrentGlyph = PIXEL_HALF;
		if (m_keys[VK_F4].bReleased)nCurrentGlyph = PIXEL_QUARTER;
		// 撤销：Ctrl + Z
		if (m_keys[VK_CONTROL].bHeld &&
			m_keys[L'Z'].bReleased) {
			Undo();
		}

		// 重做：Ctrl + Y
		if (m_keys[VK_CONTROL].bHeld &&
			m_keys[L'Y'].bReleased) {
			Redo();
		}
		//Colours
		for (int i = 0; i < 8; i++)
			if (m_keys[L"01234567"[i]].bReleased)
				if (m_keys[VK_SHIFT].bReleased)
					nCurrentColourFG = i + 8;
				else
					nCurrentColourFG = i;
		if (m_keys[VK_F7].bReleased) nCurrentColourBG--;
		if (m_keys[VK_F8].bReleased) nCurrentColourBG++;
		if (nCurrentColourBG < 0)nCurrentColourBG = 15;
		if (nCurrentColourBG > 15)nCurrentColourBG = 0;
		//Cursing
		if (m_keys[VK_SHIFT].bHeld) {
			if (m_keys[VK_UP].bReleased) nOffsetY++;
			if (m_keys[VK_DOWN].bReleased) nOffsetY--;
			if (m_keys[VK_LEFT].bReleased) nOffsetX++;
			if (m_keys[VK_RIGHT].bReleased) nOffsetX--;
		}
		else {
			if (m_keys[VK_UP].bReleased) nPosY--;
			if (m_keys[VK_DOWN].bReleased) nPosY++;
			if (m_keys[VK_LEFT].bReleased) nPosX--;
			if (m_keys[VK_RIGHT].bReleased) nPosX++;
		}
		if (sprite != nullptr) {
			if (nPosX < 0)nPosX = 0;
			if (nPosX >= sprite->nWidth)nPosX = sprite->nWidth - 1;
			if (nPosY < 0)nPosY = 0;
			if (nPosY >= sprite->nHeight)nPosY = sprite->nHeight - 1;
			// 键盘绘制
			if (m_keys[VK_SPACE].bReleased) {
				EditPixel(
					nPosX, nPosY,
					(wchar_t)nCurrentGlyph,
					(short)(nCurrentColourFG | (nCurrentColourBG << 4))
				);
			}

			// 键盘擦除
			if (m_keys[VK_DELETE].bReleased) {
				EditPixel(nPosX, nPosY, L' ', 0);
			}
			// F9：加载文件
			if (m_keys[VK_F9].bReleased) {
				if (sprite->Load(sCurrentSpriteFile)) {
					history.clear();
					historyPos = 0;

					sStatus = L"Load successful";
				}
				else {
					sStatus = L"Load failed";
				}

				fStatusTime = 3.0f;
			}

			// F10：保存文件
			if (m_keys[VK_F10].bReleased) {
				if (sprite->Save(sCurrentSpriteFile)) {
					sStatus = L"Save successful";
				}
				else {
					sStatus = L"Save failed";
				}

				fStatusTime = 3.0f;
			}
		}
		// 鼠标绘制和擦除
		int mouseX = GetMouseX();
		int mouseY = GetMouseY();

		// 画布左上角是 (10, 10)
		if (mouseX >= 10 && mouseX < 138 &&
			mouseY >= 10 && mouseY < 74) {

			int pixelX = (mouseX - 10) / nZoom + nOffsetX;
			int pixelY = (mouseY - 10) / nZoom + nOffsetY;

			if (pixelX >= 0 && pixelX < sprite->nWidth &&
				pixelY >= 0 && pixelY < sprite->nHeight) {

				// 左键：绘制当前笔刷
				if (GetMouse(0).bHeld) {
					EditPixel(
						pixelX, pixelY,
						(wchar_t)nCurrentGlyph,
						(short)(nCurrentColourFG | (nCurrentColourBG << 4))
					);
				}

				// 右键：擦除
				if (GetMouse(1).bHeld) {
					EditPixel(pixelX, pixelY, L' ', 0);
				}
			}
		}
		// 清屏
		Fill(0, 0, ScreenWidth(), ScreenHeight(), L' ', 0);

		// 菜单
		DrawString(
			1, 1,
			L"F1=100% F2=75% F3=50% F4=25% F9=Load F10=Save"
		);

		// 调色板
		for (int i = 0; i < 8; i++) {
			DrawString(1 + 6 * i, 3, to_wstring(i) + L"=");

			if (m_keys[VK_SHIFT].bHeld)
				Draw(1 + 6 * i + 4, 3, PIXEL_SOLID, i + 8);
			else
				Draw(1 + 6 * i + 4, 3, PIXEL_SOLID, i);
		}

		// 当前笔刷
		DrawString(1, 5, L"Current Brush = ");
		Draw(18, 5, nCurrentGlyph,
			nCurrentColourFG | (nCurrentColourBG << 4));

		// 操作状态
		DrawString(1, 6, L"Status: " + sStatus);
		DrawString(1, 7, L"LMB: Draw RMB: Erase Ctrl+Z: Undo Ctrl+Y: Redo");

		// 画布边框
		for (int x = 9; x <= 138; x++) {
			Draw(x, 9);
			Draw(x, 74);
		}

		for (int y = 9; y <= 74; y++) {
			Draw(9, y);
			Draw(138, y);
		}

		// 绘制精灵编辑区
		if (sprite != nullptr) {
			int nVisiblePixelsX = 128 / nZoom;
			int nVisiblePixelsY = 64 / nZoom;

			for (int x = 0; x < nVisiblePixelsX; x++) {
				for (int y = 0; y < nVisiblePixelsY; y++) {
					int sx = x - nOffsetX;
					int sy = y - nOffsetY;

					if (sx < 0 || sx >= sprite->nWidth ||
						sy < 0 || sy >= sprite->nHeight)
						continue;

					Fill(
						x * nZoom + 10,
						y * nZoom + 10,
						(x + 1) * nZoom + 10,
						(y + 1) * nZoom + 10,
						sprite->GetGlyph(sx, sy),
						sprite->GetColour(sx, sy)
					);

					// 空白像素显示为点，便于编辑
					if (sprite->GetGlyph(sx, sy) == L' ') {
						Draw(x * nZoom + 10, y * nZoom + 10, L'.');
					}

					// 当前画笔位置
					if (sx == nPosX && sy == nPosY) {
						Draw(x * nZoom + 10, y * nZoom + 10, L'0');
					}
				}
			}
		}
		return TRUE;
	}

};
int main() {
	SpriteEditor game;
	game.ConstructConsole(160, 80, 8, 8);
	game.Start();
	return 0;
}