#include"olcConsoleGameEngine.h"
#include <algorithm>
#include <filesystem>
#include <Windows.h>
#include <vector>
#include <string>
/*
关于chrono
是c++的一个时间库
https://blog.csdn.net/albertsh/article/details/105468687
*/
#include<chrono>
using namespace std;


class OneLoneCoder_FPS :public olcConsoleGameEngine
{

public:
	OneLoneCoder_FPS() {
		m_sAppName = L"FPS";
	}
	virtual bool OnUserCreate()
	{
		SetCursorHidden(true);
		init_map();
		// 检查资源文件
		vector<wstring> files =
		{
			L"E:/VSPRO/DOOM_FPS/SpriteEditor/fps_wall1.spr",
			L"E:/VSPRO/DOOM_FPS/SpriteEditor/fps_lamp1.spr",
			L"E:/VSPRO/DOOM_FPS/SpriteEditor/fps_fireball1.spr"
		};

		for (const auto& file : files)
		{
			DWORD attr = GetFileAttributesW(file.c_str());

			if (attr == INVALID_FILE_ATTRIBUTES ||
				(attr & FILE_ATTRIBUTE_DIRECTORY))
			{
				MessageBoxW(
					nullptr,
					(L"资源文件不存在：\n" + file).c_str(),
					L"文件加载错误",
					MB_OK | MB_ICONWARNING
				);

				return false;  // 如果在 OnUserCreate() 中
			}
		}
		// 全部检查通过，加载资源
			spriteWall = new olcSprite(L"E:/VSPRO/DOOM_FPS/SpriteEditor/fps_wall1.spr");
			spriteLamp = new olcSprite(L"E:/VSPRO/DOOM_FPS/SpriteEditor/fps_lamp1.spr" );
			spriteFireBall = new olcSprite(L"E:/VSPRO/DOOM_FPS/SpriteEditor/fps_fireball1.spr");

		listObjects = {
			{ 8.5f,8.5f,spriteLamp,0.0f,0.0f,FALSE },
			{ 3.5f,3.5f,spriteLamp,0.0f,0.0f,FALSE },
			{ 10.5f,10.5f,spriteLamp,0.0f,0.0f,FALSE },
		};
		fDepthBuffer = new float[ScreenWidth()];
		return TRUE;
	}
	virtual bool OnUserUpdate(float felapsedTime) {


		//游戏控件
		// ESC 切换菜单
		if (m_keys[VK_ESCAPE].bPressed)
		{
			bShowExitMenu = !bShowExitMenu;

			if (bShowExitMenu)
			{
				// 打开菜单：显示鼠标，解除限制
				ClipCursor(NULL);
				SetCursorHidden(false);
			}
			else
			{
				// 关闭菜单：隐藏鼠标
				SetCursorHidden(true);
			}
		}

		// 菜单状态
		if (bShowExitMenu)
		{
			DrawExitMenu();

			// 鼠标点击退出按钮
			int cx = ScreenWidth() / 2;
			int cy = ScreenHeight() / 2;

			int mx = GetMouseX();
			int my = GetMouseY();

			if (GetMouse(0).bReleased)
			{
				if (mx >= cx - 15 && mx <= cx + 15 &&
					my >= cy - 2 && my <= cy + 2)
				{
					// 返回 false，结束游戏循环
					return false;
				}
			}

			// 菜单打开时暂停游戏逻辑
			return true;
		}

		////旋转
		//if (m_keys[L'Q'].bHeld)
		//	fPlayerA -= (0.8f) * felapsedTime;
		//if (m_keys[L'E'].bHeld)
		//	fPlayerA += (0.8f) * felapsedTime;
		// 鼠标控制旋转
		// 获取窗口客户区
		HWND hwnd = GetForegroundWindow();

		RECT rect;
		GetClientRect(hwnd, &rect);

		// 转换为屏幕坐标
		POINT p1 = { rect.left, rect.top };
		POINT p2 = { rect.right, rect.bottom };

		ClientToScreen(hwnd, &p1);
		ClientToScreen(hwnd, &p2);

		rect.left = p1.x;
		rect.top = p1.y;
		rect.right = p2.x;
		rect.bottom = p2.y;

		// 限制鼠标在窗口内
		ClipCursor(&rect);

		// 鼠标旋转
		POINT center = {
			(rect.left + rect.right) / 2,
			(rect.top + rect.bottom) / 2
		};

		POINT mouse;
		GetCursorPos(&mouse);

		int deltaX = mouse.x - center.x;

		fPlayerA += deltaX * 0.005f;

		// 鼠标重新居中
		SetCursorPos(center.x, center.y);
		//前进后退 左右
		if (m_keys[L'W'].bHeld) {
			//撞墙检测
			if (!HitTheWall(1, felapsedTime)) {
				fPlayerX += sinf(fPlayerA) * fSpeed * felapsedTime;
				fPlayerY += cosf(fPlayerA) * fSpeed * felapsedTime;
			}
		}
		if (m_keys[L'S'].bHeld) {
			if (!HitTheWall(2, felapsedTime)) {
				fPlayerX -= sinf(fPlayerA) * fSpeed * felapsedTime;
				fPlayerY -= cosf(fPlayerA) * fSpeed * felapsedTime;
			}
		}
		if (m_keys[L'D'].bHeld) {
			//撞墙检测
			if (!HitTheWall(3, felapsedTime)) {
				fPlayerX += cosf(fPlayerA) * fSpeed * felapsedTime;
				fPlayerY -= sinf(fPlayerA) * fSpeed * felapsedTime;
			}
		}
		if (m_keys[L'A'].bHeld) {
			//撞墙检测
			if (!HitTheWall(4, felapsedTime)) {
				fPlayerX -= cosf(fPlayerA) * fSpeed * felapsedTime;
				fPlayerY += sinf(fPlayerA) * fSpeed * felapsedTime;
			}
		}
		////开火
		//if (m_keys[VK_SPACE].bReleased) {
		//	sObject FireBall;
		//	FireBall.x = fPlayerX;
		//	FireBall.y = fPlayerY;
		//	FireBall.vx = sinf(fPlayerA) * 8.0f;
		//	FireBall.vy = cosf(fPlayerA) * 8.0f;

		//	FireBall.sprite = spriteFireBall;
		//	FireBall.bRemove = FALSE;

		//	listObjects.push_back(FireBall);
		//}
		if (GetMouse(0).bReleased)
		{
			sObject FireBall;

			FireBall.x = fPlayerX;
			FireBall.y = fPlayerY;

			FireBall.vx = sinf(fPlayerA) * 8.0f;
			FireBall.vy = cosf(fPlayerA) * 8.0f;

			FireBall.sprite = spriteFireBall;
			FireBall.bRemove = FALSE;

			listObjects.push_back(FireBall);
		}
		for (int x = 0; x < ScreenWidth(); x++) {

			float fRayAngle = (fPlayerA - fFov / 2.0f) + ((float)x / (float)ScreenWidth() * fFov);

			//到墙的距离
			float fDistanceToWall = 0;
			float fStepSize = 0.095f;
			//撞墙标志
			bool bHitWall = FALSE;
			//边界标志
			bool bBoundary = FALSE;
			//视野的单位向量，表示玩家看的方向
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			float fSampleX = 0.0f;
			while (!bHitWall && fDistanceToWall < fDepth) {
				fDistanceToWall += fStepSize;
				//看到的坐标                    
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
				//出界？
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = TRUE;
					fDistanceToWall = fDepth;
				}
				else {
					//撞墙?
					//光线是入站的,测试以查看光线单元是否为墙块
					if (map[nTestY * nMapWidth + nTestX] == '#') {

						bHitWall = TRUE;

						//得到看到的墙的四个角
						float fBlockMidX = (float)nTestX + 0.5f;
						float fBlockMidY = (float)nTestY + 0.5f;

						float fTextPointX = fPlayerX + fEyeX * fDistanceToWall;
						float fTextPointY = fPlayerY + fEyeY * fDistanceToWall;

						float fTestAngle = atan2f((fTextPointY - fBlockMidY), (fTextPointX - fBlockMidX));

						if (fTestAngle >= -pi * 0.25f && fTestAngle < pi * 0.25f)
							fSampleX = fTextPointY - (float)nTestY;
						if (fTestAngle >= pi * 0.25f && fTestAngle < pi * 0.75f)
							fSampleX = fTextPointX - (float)nTestX;
						if (fTestAngle < -pi * 0.25f && fTestAngle >= -pi * 0.75f)
							fSampleX = fTextPointX - (float)nTestX;
						if (fTestAngle >= pi * 0.75f || fTestAngle < -pi * 0.75f)
							fSampleX = fTextPointY - (float)nTestY;
					}
				}
				//天花板
				int nCeiling = (float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)fDistanceToWall);
				//地板
				int nFloor = ScreenHeight() - nCeiling;

				//存入缓冲区
				fDepthBuffer[x] = fDistanceToWall;
				for (int y = 0; y < ScreenHeight(); y++) {
					if (y < nCeiling) {
						Draw(x, y, L' ');//screen[y * nScreenWidth + x] = ' ';
					}
					else if (y >= nCeiling && y <= nFloor) {
						float fSampleY = ((float)y - (float)nCeiling) / ((float)nFloor - (float)nCeiling);
						if (fDistanceToWall < fDepth)Draw(x, y, spriteWall->SampleGlyph(fSampleX, fSampleY), spriteWall->SampleColour(fSampleX, fSampleY));
						else Draw(x, y, PIXEL_SOLID, 0);
					}
					else {
						Draw(x, y, PIXEL_SOLID, FG_DARK_GREEN);//screen[y * nScreenWidth + x] = Floor_hash;
					}
				}
			}
		}
		//更新&显示对象
		for (auto& object : listObjects) {
			object.x += object.vx * felapsedTime;
			object.y += object.vy * felapsedTime;
			if (map.c_str()[(int)object.x * nMapWidth + (int)object.y] == '#') {
				object.bRemove = TRUE;
				//if(++Hurtreceived[(int)object.x * nMapWidth + (int)object.y]>5)
				//	map[(int)object.x * nMapWidth + (int)object.y]=L' ';

			}
			//物体坐标
			float fVecX = object.x - fPlayerX;
			float fVecY = object.y - fPlayerY;
			//物体与玩家的距离
			float fDistanceFromPlayer = sqrtf(fVecY * fVecY + fVecX * fVecX);

			//视野的单位向量，表示玩家看的方向
			float fEyeX = sinf(fPlayerA);
			float fEyeY = cosf(fPlayerA);
			float fObjectAngle = atan2f(fEyeY, fEyeX) - atan2f(fVecY, fVecX);
			if (fObjectAngle < -pi)
				fObjectAngle += 2.0f * pi;
			if (fObjectAngle > pi)
				fObjectAngle -= 2.0f * pi;
			//判断物体是否在视野范围内
			bool bInPlayerFov = fabs(fObjectAngle) < fFov / 2.0f;
			if (bInPlayerFov && fDistanceFromPlayer >= 0.5f && fDistanceFromPlayer < fDepth) {

				float fObjectCeiling = (float)(ScreenHeight() / 2.0) - ScreenHeight() / fDistanceFromPlayer;
				float fObjectFloor = ScreenHeight() - fObjectCeiling;
				//物件高
				float fObjectHeight = fObjectFloor - fObjectCeiling;
				//物件高宽比
				float fObjectAspectRatio = (float)object.sprite->nHeight / ((float)object.sprite->nWidth);
				//物件宽
				float fObjectWidth = fObjectHeight / fObjectAspectRatio;

				float fMiddleOfObject = (0.5f * (fObjectAngle / (fFov / 2.0f)) + 0.5f) * (float)ScreenWidth();

				for (float lx = 0;lx < fObjectWidth;lx++)
					for (float ly = 0; ly < fObjectHeight; ly++) {
						float fSampleX = lx / fObjectWidth;
						float fSampleY = ly / fObjectHeight;
						wchar_t c = object.sprite->SampleGlyph(fSampleX, fSampleY);
						int nObjectColumn = (int)(fMiddleOfObject + lx - (fObjectWidth / 2.0f));
						if (nObjectColumn >= 0 && nObjectColumn < ScreenWidth()) {
							if (c != L' ' && fDepthBuffer[nObjectColumn] >= fDistanceFromPlayer) {
								Draw(nObjectColumn, fObjectCeiling + ly, c, object.sprite->SampleColour(fSampleX, fSampleY));
								fDepthBuffer[nObjectColumn] = fDistanceFromPlayer;
							}
						}
					}
			}
		}
		listObjects.remove_if([](sObject& o) {return o.bRemove; });

		//显示地图
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapHeight; ny++)
				Draw(nx + 1, ny + 1, map[ny * nMapWidth + nx]);
		//显示玩家位置
		Draw((int)+1, (int)fPlayerY + 1, L'P');// screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = '*';
		return TRUE;
	}
	virtual bool OnUserDestroy()
	{
		// 解除鼠标限制
		ClipCursor(NULL);

		// 恢复鼠标显示
		SetCursorHidden(false);

		return true;
	}
private:
	float pi = 3.1415926f;
	//玩家信息
	float fPlayerX = 8.0f;//玩家坐标
	float fPlayerY = 8.0f;
	float fPlayerA = 0.0f;//玩家朝向
	float fFov = pi / 4.0;//视野范围
	float fDepth = 20;//视野深度
	float fSpeed = 4.0f;

	//场地
	int nMapWidth = 32;
	int nMapHeight = 32;
	wstring map;
	int* Hurtreceived;
	//场地初始化函数
	//墙壁图案
	olcSprite* spriteWall;
	//路灯图案
	olcSprite* spriteLamp;
	//子弹图案
	olcSprite* spriteFireBall;
	//对象类
	struct sObject {
		//坐标
		float x;
		float y;
		//图案
		olcSprite* sprite;
		//子弹使用的速度向量
		float vx, vy;
		bool bRemove;
	};
	list<sObject> listObjects;
	//深度缓冲区
	float* fDepthBuffer = nullptr;
	// 退出菜单状态
	bool bShowExitMenu = false;
	// 鼠标显示状态
	bool bCursorHidden = true;
	//判断玩家向此方向运动是否会撞墙
	bool HitTheWall(int x, float felapsedTime) {
		float fX = fPlayerX;
		float fY = fPlayerY;
		bool WillItCrash = FALSE;
		switch (x)
		{
		case 1:
			fX += sinf(fPlayerA) * fSpeed * felapsedTime;
			fY += cosf(fPlayerA) * fSpeed * felapsedTime;
			if (map[(int)fY * nMapWidth + (int)fX] == '#')
				WillItCrash = TRUE;
			break;
		case 2:
			fX -= sinf(fPlayerA) * fSpeed * felapsedTime;
			fY -= cosf(fPlayerA) * fSpeed * felapsedTime;
			if (map[(int)fY * nMapWidth + (int)fX] == '#')
				WillItCrash = TRUE;
			break;
		case 3:
			fX += cosf(fPlayerA) * fSpeed * felapsedTime;
			fY -= sinf(fPlayerA) * fSpeed * felapsedTime;
			if (map[(int)fY * nMapWidth + (int)fX] == '#')
				WillItCrash = TRUE;
			break;
		case 4:
			fX -= cosf(fPlayerA) * fSpeed * felapsedTime;
			fY += sinf(fPlayerA) * fSpeed * felapsedTime;
			if (map[(int)fY * nMapWidth + (int)fX] == '#')
				WillItCrash = TRUE;
		default:
			break;
		}
		return WillItCrash;
	}
	void SetCursorHidden(bool hide)
	{
		if (bCursorHidden == hide)
			return;

		ShowCursor(hide ? FALSE : TRUE);
		bCursorHidden = hide;
	}
	void DrawExitMenu()
	{
		int cx = ScreenWidth() / 2;
		int cy = ScreenHeight() / 2;

		// 菜单背景
		Fill(cx - 35, cy - 15,
			cx + 35, cy + 15,
			PIXEL_SOLID, FG_DARK_GREY);

		// 菜单标题
		DrawString(cx - 5, cy - 10,
			L"GAME MENU", FG_WHITE);

		// 退出按钮
		int x1 = cx - 20;
		int y1 = cy - 2;
		int x2 = cx + 20;
		int y2 = cy + 3;

		int mx = GetMouseX();
		int my = GetMouseY();

		bool hover =
			mx >= x1 && mx <= x2 &&
			my >= y1 && my <= y2;

		short color = hover ? FG_GREEN : FG_WHITE;

		Fill(x1, y1, x2, y2,
			PIXEL_SOLID, color);

		DrawString(cx - 4, cy,
			L"EXIT GAME", FG_BLACK);

		DrawString(cx - 9, cy + 9,
			L"PRESS ESC TO RESUME", FG_WHITE);
	}
	void init_map() {
		//for (int x = 0; x < nMapWidth; x++)
		//	for (int y = 0; y < nMapHeight; y++)
		//		Hurtreceived[y * nMapHeight + x]=0;
		map += L"################################";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#      #            #  #       #";
		map += L"#      #       #    #  #       #";
		map += L"#              #    #          #";
		map += L"#         ######    #     ######";
		map += L"#              #               #";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#                              #";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"###                 #        ###";
		map += L"###                 #        ###";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#      #       #    #  #       #";
		map += L"#      #            #  #       #";
		map += L"#              #    #          #";
		map += L"#         ######    #     ######";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"#              #               #";
		map += L"#              #    #          #";
		map += L"#              #    #          #";
		map += L"################################";
	}
};

int main() {

	OneLoneCoder_FPS game;
	game.ConstructConsole(240, 160, 4, 4);
	game.Start();

	return 0;
}
