#include"olcConsoleGameEngine.h"
#include<vector>
#include <algorithm>
/*
关于chrono
是c++的一个时间库
https://blog.csdn.net/albertsh/article/details/105468687
*/
#include<chrono>
using namespace std;


class OneLoneCoder_FPS :public olcConsoleGameEngine {

public:
	OneLoneCoder_FPS() {
		m_sAppName = L"FPS";
	}
	virtual bool OnUserCreate() {
		init_map();
		spriteWall = new olcSprite(L"E:/Pro/SpriteEditor/fps_wall1.spr");
		spriteLamp = new olcSprite(L"D:/C++ project/SpriteEditor/SpriteEditor/fps_lamp1.spr");
		spriteFireBall = new olcSprite(L"D:/C++ project/SpriteEditor/SpriteEditor/fps_fireball1.spr");
		listObjects = {
			{8.5f,8.5f,spriteLamp,0.0f,0.0f,FALSE},
			{3.5f,3.5f,spriteLamp,0.0f,0.0f,FALSE},
			{10.5f,10.5f,spriteLamp,0.0f,0.0f,FALSE},
		};
		fDepthBuffer = new float[ScreenWidth()];
		return TRUE;
	}
	virtual bool OnUserUpdate(float felapsedTime) {
		//游戏控件
		//旋转
		if (m_keys[L'Q'].bHeld)
			fPlayerA -= (0.8f) * felapsedTime;
		if (m_keys[L'E'].bHeld)
			fPlayerA += (0.8f) * felapsedTime;
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
		//开火
		if (m_keys[VK_SPACE].bReleased) {
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
		Draw((int)fPlayerX + 1, (int)fPlayerY + 1, L'P');// screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = '*';
		return TRUE;
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


	//场地
	int nMapWidth = 32;
	int nMapHeight = 32;
	wstring map;
	int* Hurtreceived;

	//场地初始化函数
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
};

int main() {

	OneLoneCoder_FPS game;
	game.ConstructConsole(240, 160, 4, 4);
	game.Start();

	return 0;
}
