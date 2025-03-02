/*
	This file is part of Warzone 2100.
	Copyright (C) 1999-2004  Eidos Interactive
	Copyright (C) 2005-2020  Warzone 2100 Project

	Warzone 2100 is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Warzone 2100 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Warzone 2100; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
#include <string.h>
#include <cstdlib>

#include "lib/framework/frame.h"
#include "lib/framework/fixedpoint.h"
#include "lib/framework/math_ext.h"
#include "lib/ivis_opengl/pieblitfunc.h"
// FIXME Direct iVis implementation include!
#include "lib/ivis_opengl/piematrix.h"
#include "lib/ivis_opengl/piepalette.h"
#include "lib/ivis_opengl/piestate.h"
#include "lib/ivis_opengl/piefunc.h"
#include "lib/ivis_opengl/bitimage.h"
#include "lib/gamelib/gtime.h"
#include "advvis.h"
#include "objects.h"
#include "display3d.h"
#include "map.h"
#include "component.h"
#include "console.h"
#include "radar.h"
#include "mapdisplay.h"
#include "hci.h"
#include "geometry.h"
#include "intimage.h"
#include "loop.h"
#include "warcam.h"
#include "display.h"
#include "mission.h"
#include "multiplay.h"
#include "intdisplay.h"
#include "texture.h"
#include "warzoneconfig.h"
#ifndef GLM_ENABLE_EXPERIMENTAL
	#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/transform.hpp>

#define HIT_NOTIFICATION	(GAME_TICKS_PER_SEC * 2)
#define RADAR_FRAME_SKIP	10

static void applyMinimapOverlay();

bool bEnemyAllyRadarColor = false;     			/**< Enemy/ally radar color. */
RADAR_DRAW_MODE	radarDrawMode = RADAR_MODE_DEFAULT;	/**< Current mini-map mode. */
bool rotateRadar = true; ///< Rotate the radar?
bool radarRotationArrow = true; ///< display arrow when radar rotation enabled?

static PIELIGHT		colRadarAlly, colRadarMe, colRadarEnemy;
static PIELIGHT		tileColours[MAX_TILES];
static UDWORD		*radarBuffer = nullptr;
static UDWORD		*radarOverlayBuffer = nullptr;
static Vector3i		playerpos = {0, 0, 0};

PIELIGHT clanColours[] =
{
	// see frontend2.png for team color order.
	// [r,g,b,a]
	{{0, 255, 0, 255}},		// green  Player 0
	{{255, 192, 40, 255}},          // orange Player 1
	{{255, 255, 255, 255}},	// grey   Player 2
	{{0, 0, 0, 255}},			// black  Player 3
	{{255, 0, 0, 255}},		// red    Player 4
	{{20, 20, 255, 255}},		// blue   Player 5
	{{255, 0, 192, 255}},           // pink   Player 6
	{{0, 255, 255, 255}},		// cyan   Player 7
	{{255, 255, 0, 255}},           // yellow Player 8
	{{144, 0, 255, 255}},           // purple Player 9
	{{200, 255, 255, 255}},         // white  Player A (Should be brighter than grey, but grey is already maximum.)
	{{128, 128, 255, 255}},         // bright blue Player B
	{{128, 255, 128, 255}},         // neon green  Player C
	{{128, 0, 0, 255}},             // infrared    Player D
	{{64, 0, 128, 255}},            // ultraviolet Player E
	{{128, 128, 0, 255}},           // brown       Player F
};

static PIELIGHT flashColours[] =
{
	//right now the flash color is all bright red
	{{254, 37, 37, 200}},	// Player 0
	{{254, 37, 37, 200}},	// Player 1
	{{254, 37, 37, 200}},	// Player 2
	{{254, 37, 37, 200}},	// Player 3
	{{254, 37, 37, 200}},	// Player 4  (notice, brighter red)
	{{254, 37, 37, 200}},	// Player 5
	{{254, 37, 37, 200}},	// Player 6
	{{254, 37, 37, 200}},   // Player 7
	{{254, 37, 37, 200}},   // Player 8
	{{254, 37, 37, 200}},   // Player 9
	{{254, 37, 37, 200}},   // Player A
	{{254, 37, 37, 200}},   // Player B
	{{254, 37, 37, 200}},   // Player C
	{{254, 37, 37, 200}},   // Player D
	{{254, 37, 37, 200}},   // Player E
	{{254, 37, 37, 200}},   // Player F
};

static size_t radarWidth, radarHeight, radarTexWidth, radarTexHeight;
static SDWORD radarCenterX, radarCenterY;
static uint8_t RadarZoom;
static float RadarZoomMultiplier = 1.0f;
static size_t radarBufferSize = 0;
static int frameSkip = 0;
static UDWORD lastBlink = 0;
static const UDWORD BLINK_INTERVAL = GAME_TICKS_PER_SEC / 1;
static const UDWORD BLINK_HALF_INTERVAL = BLINK_INTERVAL / 2;
static const float OVERLAY_OPACITY = 0.5f;

// taken from https://en.wikipedia.org/wiki/Alpha_compositing
PIELIGHT inline mix(PIELIGHT over, PIELIGHT base)
{
	float overAlpha = over.byte.a / 255.0f;
	float overRed = over.byte.r / 255.0f;
	float overGreen = over.byte.g / 255.0f;
	float overBlue = over.byte.b / 255.0f;

	float baseAlpha = base.byte.a / 255.0f;
	float baseRed = base.byte.r / 255.0f;
	float baseGreen = base.byte.g / 255.0f;
	float baseBlue = base.byte.b / 255.0f;

	float newAlpha = overAlpha + baseAlpha * (1 - overAlpha);
	float newRed = (overRed * overAlpha + baseRed * baseAlpha * (1 - overAlpha)) / newAlpha;
	float newGreen = (overGreen * overAlpha + baseGreen * baseAlpha * (1 - overAlpha)) / newAlpha;
	float newBlue = (overBlue * overAlpha + baseBlue * baseAlpha * (1 - overAlpha)) / newAlpha;

	UBYTE returnAlpha = static_cast<UBYTE>(clip<int>(static_cast<int>(newAlpha * 255.0f), 0, 255));
	UBYTE returnRed = static_cast<UBYTE>(clip<int>(static_cast<int>(newRed * 255.0f), 0, 255));
	UBYTE returnGreen = static_cast<UBYTE>(clip<int>(static_cast<int>(newGreen * 255.0f), 0, 255));
	UBYTE returnBlue = static_cast<UBYTE>(clip<int>(static_cast<int>(newBlue * 255.0f), 0, 255));

	return { {returnRed, returnGreen, returnBlue, returnAlpha} };
}

PIELIGHT inline PLfromUDWORD(UDWORD value)
{
	PIELIGHT retColor = { {
		static_cast<UBYTE>((value & 0x000000FF) >> 0),
		static_cast<UBYTE>((value & 0x0000FF00) >> 8),
		static_cast<UBYTE>((value & 0x00FF0000) >> 16),
		static_cast<UBYTE>((value & 0xFF000000) >> 24),
	} };
	return retColor;
}

PIELIGHT inline applyAlpha(PIELIGHT color, float alpha)
{
	PIELIGHT ret = color;
	ret.byte.a = static_cast<uint8_t>(ret.byte.a * alpha);
	return ret;
}

static void DrawRadarTiles();
static void DrawRadarObjects();
static void DrawRadarExtras(const glm::mat4 &modelViewProjectionMatrix);
static void DrawNorth(const glm::mat4 &modelViewProjectionMatrix);
static void setViewingWindow();

static void radarSize(int ZoomLevel)
{
	float zoom = static_cast<float>(ZoomLevel) * RadarZoomMultiplier / 16.0f;
	radarWidth = static_cast<size_t>(radarTexWidth * zoom);
	radarHeight = static_cast<size_t>(radarTexHeight * zoom);
	if (rotateRadar)
	{
		radarCenterX = pie_GetVideoBufferWidth() - BASE_GAP * 4 - static_cast<int>(MAX(radarHeight, radarWidth)) / 2;
		radarCenterY = pie_GetVideoBufferHeight() - BASE_GAP * 4 - static_cast<int>(MAX(radarWidth, radarHeight)) / 2;
	}
	else
	{
		radarCenterX = pie_GetVideoBufferWidth() - BASE_GAP * 4 - static_cast<int>(radarWidth) / 2;
		radarCenterY = pie_GetVideoBufferHeight() - BASE_GAP * 4 - static_cast<int>(radarHeight) / 2;
	}
	debug(LOG_WZ, "radar=(%u,%u) tex=(%zu,%zu) size=(%zu,%zu)", radarCenterX, radarCenterY, radarTexWidth, radarTexHeight, radarWidth, radarHeight);
}

void radarInitVars()
{
	radarTexWidth = 0;
	radarTexHeight = 0;
	RadarZoom = war_GetRadarZoom();
	debug(LOG_WZ, "Resetting radar zoom to %u", RadarZoom);
	radarSize(RadarZoom);
	playerpos = Vector3i(-1, -1, -1);
	frameSkip = 0;
}

bool InitRadar()
{
	// Ally/enemy/me colors
	colRadarAlly = WZCOL_YELLOW;
	colRadarEnemy = WZCOL_RED;
	colRadarMe = WZCOL_WHITE;
	return true;
}

bool resizeRadar()
{
	if (radarBuffer)
	{
		free(radarBuffer);
	}
	if (radarOverlayBuffer)
	{
		free(radarOverlayBuffer);
	}
	radarTexWidth = static_cast<size_t>(std::abs(scrollMaxX - scrollMinX));
	radarTexHeight = static_cast<size_t>(std::abs(scrollMaxY - scrollMinY));
	radarBufferSize = radarTexWidth * radarTexHeight * sizeof(UDWORD);
	radarBuffer = (uint32_t *)malloc(radarBufferSize);
	radarOverlayBuffer = (uint32_t*)malloc(radarBufferSize);
	memset(radarBuffer, 0, radarBufferSize);
	memset(radarOverlayBuffer, 0, radarBufferSize);
	frameSkip = 0;
	if (rotateRadar)
	{
		RadarZoomMultiplier = (float)std::max(RADWIDTH, RADHEIGHT) / std::max<size_t>({radarTexWidth, radarTexHeight, 1});
	}
	else
	{
		RadarZoomMultiplier = 1.0f;
	}
	debug(LOG_WZ, "Setting radar zoom to %u", RadarZoom);
	radarSize(RadarZoom);
	pie_SetRadar(-static_cast<float>(radarWidth) / 2.0f - 1, -static_cast<float>(radarHeight) / 2.0f - 1, static_cast<float>(radarWidth), static_cast<float>(radarHeight), radarTexWidth, radarTexHeight);

	return true;
}

bool ShutdownRadar()
{
	free(radarBuffer);
	radarBuffer = nullptr;
	free(radarOverlayBuffer);
	radarOverlayBuffer = nullptr;
	frameSkip = 0;
	return true;
}

void SetRadarZoom(uint8_t ZoomLevel)
{
	if (ZoomLevel > MAX_RADARZOOM)
	{
		ZoomLevel = MAX_RADARZOOM;
	}
	if (ZoomLevel < MIN_RADARZOOM)
	{
		ZoomLevel = MIN_RADARZOOM;
	}
	debug(LOG_WZ, "Setting radar zoom to %u from %u", ZoomLevel, RadarZoom);
	RadarZoom = ZoomLevel;
	radarSize(RadarZoom);
	frameSkip = 0;
	resizeRadar();
}

uint8_t GetRadarZoom()
{
	return RadarZoom;
}

/** Calculate the radar pixel sizes. Returns pixels per tile. */
static void CalcRadarPixelSize(float *SizeH, float *SizeV)
{
	*SizeH = (float)radarHeight / std::max<size_t>(radarTexHeight, 1);
	*SizeV = (float)radarWidth / std::max<size_t>(radarTexWidth, 1);
}

/** Given a position within the radar, return a world coordinate. */
void CalcRadarPosition(int mX, int mY, int *PosX, int *PosY)
{
	int		sPosX, sPosY;
	float		pixSizeH, pixSizeV;

	Vector2f pos;
	pos.x = mX - radarCenterX;
	pos.y = mY - radarCenterY;
	if (rotateRadar)
	{
		pos = Vector2f_Rotate2f(pos, -playerPos.r.y);
	}
	pos.x += radarWidth / 2.0f;
	pos.y += radarHeight / 2.0f;

	CalcRadarPixelSize(&pixSizeH, &pixSizeV);
	sPosX = static_cast<int>(pos.x / pixSizeH);	// adjust for pixel size
	sPosY = static_cast<int>(pos.y / pixSizeV);
	sPosX += scrollMinX;		// adjust for scroll limits
	sPosY += scrollMinY;

#if REALLY_DEBUG_RADAR
	debug(LOG_ERROR, "m=(%d,%d) radar=(%d,%d) pos(%d,%d), scroll=(%u-%u,%u-%u) sPos=(%d,%d), pixSize=(%f,%f)",
	      mX, mY, radarX, radarY, posX, posY, scrollMinX, scrollMaxX, scrollMinY, scrollMaxY, sPosX, sPosY, pixSizeH, pixSizeV);
#endif

	// old safety code -- still necessary?
	sPosX = clip<int>(sPosX, scrollMinX, scrollMaxX -1);
	sPosY = clip<int>(sPosY, scrollMinY, scrollMaxY -1);

	*PosX = sPosX;
	*PosY = sPosY;
}

void drawRadar()
{
	float	pixSizeH, pixSizeV;

	CalcRadarPixelSize(&pixSizeH, &pixSizeV);

	ASSERT_OR_RETURN(, radarBuffer, "No radar buffer allocated");
	ASSERT_OR_RETURN(, radarOverlayBuffer, "No radar buffer allocated");

	setViewingWindow();
	playerpos = playerPos.p; // cache position

	if (frameSkip <= 0)
	{
		DrawRadarTiles();
		DrawRadarObjects();
		applyMinimapOverlay();
		pie_DownLoadRadar(radarBuffer);
		frameSkip = RADAR_FRAME_SKIP;
	}
	frameSkip--;
	glm::mat4 radarMatrix = glm::translate(glm::vec3(radarCenterX, radarCenterY, 0));
	glm::mat4 orthoMatrix = glm::ortho(0.f, static_cast<float>(pie_GetVideoBufferWidth()), static_cast<float>(pie_GetVideoBufferHeight()), 0.f);
	if (rotateRadar)
	{
		// rotate the map
		radarMatrix *= glm::rotate(UNDEG(playerPos.r.y), glm::vec3(0.f, 0.f, 1.f));
		if (radarRotationArrow)
		{
			DrawNorth(orthoMatrix * radarMatrix);
		}
	}

	pie_RenderRadar(orthoMatrix * radarMatrix);
	DrawRadarExtras(orthoMatrix * radarMatrix * glm::translate(glm::vec3(-static_cast<float>(radarWidth) / 2.f - 1.f, -static_cast<float>(radarHeight) / 2.f - 1.f, 0.f)));
	drawRadarBlips(static_cast<int>(-static_cast<int>(radarWidth) / 2.f - 1), static_cast<int>(-static_cast<int>(radarHeight) / 2.f - 1), pixSizeH, pixSizeV, orthoMatrix * radarMatrix);
}

static void DrawNorth(const glm::mat4 &modelViewProjectionMatrix)
{
	iV_DrawImage(IntImages, RADAR_NORTH, static_cast<int>(-((radarWidth / 2.f) + iV_GetImageWidth(IntImages, RADAR_NORTH) + 1)), static_cast<int>(-(radarHeight / 2.f)), modelViewProjectionMatrix);
}

static PIELIGHT inline appliedRadarColour(RADAR_DRAW_MODE drawMode, MAPTILE *WTile)
{
	PIELIGHT WScr = WZCOL_BLACK;	// squelch warning

	// draw radar on/off feature
	if (!getRevealStatus() && !TEST_TILE_VISIBLE_TO_SELECTEDPLAYER(WTile))
	{
		return WZCOL_TRANSPARENT_BOX;
	}

	switch (drawMode)
	{
	case RADAR_MODE_TERRAIN:
		{
			// draw radar terrain on/off feature
			PIELIGHT col = tileColours[TileNumber_tile(WTile->texture)];

			col.byte.r = static_cast<uint8_t>(sqrtf(col.byte.r * WTile->illumination));
			col.byte.b = static_cast<uint8_t>(sqrtf(col.byte.b * WTile->illumination));
			col.byte.g = static_cast<uint8_t>(sqrtf(col.byte.g * WTile->illumination));
			if (terrainType(WTile) == TER_CLIFFFACE)
			{
				col.byte.r /= 2;
				col.byte.g /= 2;
				col.byte.b /= 2;
			}
			if (!hasSensorOnTile(WTile, selectedPlayer))
			{
				col.byte.r = col.byte.r * 2 / 3;
				col.byte.g = col.byte.g * 2 / 3;
				col.byte.b = col.byte.b * 2 / 3;
			}
			if (!TEST_TILE_VISIBLE_TO_SELECTEDPLAYER(WTile))
			{
				col.byte.r /= 2;
				col.byte.g /= 2;
				col.byte.b /= 2;
			}
			WScr = col;
		}
		break;
	case RADAR_MODE_COMBINED:
		{
			// draw radar terrain on/off feature
			PIELIGHT col = tileColours[TileNumber_tile(WTile->texture)];

			col.byte.r = static_cast<uint8_t>(sqrtf(col.byte.r * (WTile->illumination + WTile->height / ELEVATION_SCALE) / 2));
			col.byte.b = static_cast<uint8_t>(sqrtf(col.byte.b * (WTile->illumination + WTile->height / ELEVATION_SCALE) / 2));
			col.byte.g = static_cast<uint8_t>(sqrtf(col.byte.g * (WTile->illumination + WTile->height / ELEVATION_SCALE) / 2));
			if (terrainType(WTile) == TER_CLIFFFACE)
			{
				col.byte.r /= 2;
				col.byte.g /= 2;
				col.byte.b /= 2;
			}
			if (!hasSensorOnTile(WTile, selectedPlayer))
			{
				col.byte.r = col.byte.r * 2 / 3;
				col.byte.g = col.byte.g * 2 / 3;
				col.byte.b = col.byte.b * 2 / 3;
			}
			if (!TEST_TILE_VISIBLE_TO_SELECTEDPLAYER(WTile))
			{
				col.byte.r /= 2;
				col.byte.g /= 2;
				col.byte.b /= 2;
			}
			WScr = col;
		}
		break;
	case RADAR_MODE_HEIGHT_MAP:
		{
			WScr.byte.r = WScr.byte.g = WScr.byte.b = WTile->height / ELEVATION_SCALE;
		}
		break;
	case RADAR_MODE_NO_TERRAIN:
		{
			WScr = WZCOL_RADAR_BACKGROUND;
		}
		break;
	case NUM_RADAR_MODES:
		{
			assert(false);
		}
		break;
	}
	return WScr;
}

/** Draw the map tiles on the radar. */
static void DrawRadarTiles()
{
	SDWORD	x, y;

	for (x = scrollMinX; x < scrollMaxX; x++)
	{
		for (y = scrollMinY; y < scrollMaxY; y++)
		{
			MAPTILE	*psTile = mapTile(x, y);
			size_t pos = radarTexWidth * (y - scrollMinY) + (x - scrollMinX);

			ASSERT(pos * sizeof(*radarBuffer) < radarBufferSize, "Buffer overrun");
			if (y == scrollMinY || x == scrollMinX || y == scrollMaxY - 1 || x == scrollMaxX - 1)
			{
				radarBuffer[pos] = WZCOL_BLACK.rgba;
				continue;
			}
			radarBuffer[pos] = appliedRadarColour(radarDrawMode, psTile).rgba;
		}
	}
}

/** Draw the droids and structure positions on the radar. */
static void DrawRadarObjects()
{
	UBYTE				clan;
	PIELIGHT			playerCol;
	PIELIGHT			flashCol;
	memset(radarOverlayBuffer, 0, radarBufferSize);
	bool blinkState = (gameTime - lastBlink) / BLINK_HALF_INTERVAL;

	/* Show droids on map - go through all players */
	for (clan = 0; clan < MAX_PLAYERS; clan++)
	{
		DROID		*psDroid;

		//see if have to draw enemy/ally color
		if (bEnemyAllyRadarColor)
		{
			if (clan == selectedPlayer)
			{
				playerCol = colRadarMe;
			}
			else
			{
				playerCol = (selectedPlayer < MAX_PLAYERS && aiCheckAlliances(selectedPlayer, clan) ? colRadarAlly : colRadarEnemy);
			}
		}
		else
		{
			//original 8-color mode
			STATIC_ASSERT(MAX_PLAYERS <= ARRAY_SIZE(clanColours));
			playerCol = clanColours[getPlayerColour(clan)];
		}

		STATIC_ASSERT(MAX_PLAYERS <= ARRAY_SIZE(flashColours));
		flashCol = flashColours[getPlayerColour(clan)];

		/* Go through all droids */
		for (psDroid = apsDroidLists[clan]; psDroid != nullptr; psDroid = psDroid->psNext)
		{
			if (psDroid->pos.x < world_coord(scrollMinX) || psDroid->pos.y < world_coord(scrollMinY)
			    || psDroid->pos.x >= world_coord(scrollMaxX) || psDroid->pos.y >= world_coord(scrollMaxY))
			{
				continue;
			}
			if (psDroid->visibleForLocalDisplay()
			    || (bMultiPlayer && alliancesSharedVision(game.alliance)
					&& selectedPlayer < MAX_PLAYERS && aiCheckAlliances(selectedPlayer, psDroid->player)))
			{
				int	x = psDroid->pos.x / TILE_UNITS;
				int	y = psDroid->pos.y / TILE_UNITS;
				size_t	pos = (x - scrollMinX) + (y - scrollMinY) * radarTexWidth;

				ASSERT(pos * sizeof(*radarOverlayBuffer) < radarBufferSize, "Buffer overrun");
				if (clan == selectedPlayer && gameTime > HIT_NOTIFICATION && gameTime - psDroid->timeLastHit < HIT_NOTIFICATION)
				{
					if (psDroid->selected && !blinkState)
						radarOverlayBuffer[pos] = applyAlpha(flashCol, OVERLAY_OPACITY).rgba;
					else
						radarOverlayBuffer[pos] = flashCol.rgba;
				}
				else
				{
					if (psDroid->selected && !blinkState)
						radarOverlayBuffer[pos] = applyAlpha(playerCol, OVERLAY_OPACITY).rgba;
					else
						radarOverlayBuffer[pos] = playerCol.rgba;
				}
			}
		}
	}

	/* Do the same for structures */
	for (SDWORD x = scrollMinX; x < scrollMaxX; x++)
	{
		for (SDWORD y = scrollMinY; y < scrollMaxY; y++)
		{
			MAPTILE		*psTile = mapTile(x, y);
			STRUCTURE	*psStruct;
			size_t		pos = (x - scrollMinX) + (y - scrollMinY) * radarTexWidth;

			ASSERT(pos * sizeof(*radarOverlayBuffer) < radarBufferSize, "Buffer overrun");
			if (!TileHasStructure(psTile))
			{
				continue;
			}
			psStruct = (STRUCTURE *)psTile->psObject;
			clan = psStruct->player;

			//see if have to draw enemy/ally color
			if (bEnemyAllyRadarColor)
			{
				if (clan == selectedPlayer)
				{
					playerCol = colRadarMe;
				}
				else
				{
					playerCol = (selectedPlayer < MAX_PLAYERS && aiCheckAlliances(selectedPlayer, clan) ? colRadarAlly : colRadarEnemy);
				}
			}
			else
			{
				//original 8-color mode
				playerCol = clanColours[getPlayerColour(clan)];
			}
			flashCol = flashColours[getPlayerColour(clan)];

			if (psStruct->visibleForLocalDisplay()
			    || (bMultiPlayer && alliancesSharedVision(game.alliance)
			        && selectedPlayer < MAX_PLAYERS && aiCheckAlliances(selectedPlayer, psStruct->player)))
			{
				if (clan == selectedPlayer && gameTime > HIT_NOTIFICATION && gameTime - psStruct->timeLastHit < HIT_NOTIFICATION)
				{
					if (psStruct->player == selectedPlayer && psStruct->selected && !blinkState)
						radarOverlayBuffer[pos] = applyAlpha(flashCol, OVERLAY_OPACITY).rgba;
					else
						radarOverlayBuffer[pos] = flashCol.rgba;
				}
				else
				{
					if (psStruct->player == selectedPlayer && psStruct->selected && !blinkState)
						radarOverlayBuffer[pos] = applyAlpha(playerCol, OVERLAY_OPACITY).rgba;
					else
						radarOverlayBuffer[pos] = playerCol.rgba;
				}
			}
		}
	}
	if (gameTime - lastBlink >= BLINK_INTERVAL)
		lastBlink = gameTime;
}

static void applyMinimapOverlay()
{
	size_t radarTexCount = radarTexWidth * radarTexHeight;
	ASSERT(radarTexCount * sizeof(*radarBuffer) <= radarBufferSize, "Buffer overrun");
	ASSERT(radarTexCount * sizeof(*radarOverlayBuffer) <= radarBufferSize, "Buffer overrun");
	for (size_t i = 0; i < radarTexCount; i++)
	{
		if (radarOverlayBuffer[i] == 0)
			continue;
		PIELIGHT baseColor = PLfromUDWORD(radarBuffer[i]);
		PIELIGHT overColor = PLfromUDWORD(radarOverlayBuffer[i]);
		PIELIGHT mixedColor = mix(overColor, baseColor);
		radarBuffer[i] = mixedColor.rgba;
	}
}

/** Rotate an array of 2d vectors about a given angle, also translates them after rotating. */
static void RotateVector2D(Vector3i *Vector, Vector3i *TVector, Vector3i *Pos, int Angle, int Count)
{
	int64_t Cos = iCos(Angle);
	int64_t Sin = iSin(Angle);
	int ox = 0;
	int oy = 0;
	int i;
	Vector3i *Vec = Vector;
	Vector3i *TVec = TVector;

	if (Pos)
	{
		ox = Pos->x;
		oy = Pos->y;
	}

	for (i = 0; i < Count; i++)
	{
		TVec->x = ((Vec->x * Cos + Vec->y * Sin) >> 16) + ox;
		TVec->y = ((Vec->y * Cos - Vec->x * Sin) >> 16) + oy;
		Vec++;
		TVec++;
	}
}

static SDWORD getDistanceAdjust()
{
	int dif = std::max<int>(static_cast<int>(MAXDISTANCE - getViewDistance()), 0);

	return dif / 100;
}

static SDWORD getLengthAdjust()
{
	const int pitch = static_cast<int>(360 - (playerPos.r.x / DEG_1));

	// Max at
	const int lookingDown = (0 - MIN_PLAYER_X_ANGLE);
	const int lookingFar = (0 - MAX_PLAYER_X_ANGLE);

	int dif = MAX(pitch - lookingFar, 0);
	if (dif > (lookingDown - lookingFar))
	{
		dif = (lookingDown - lookingFar);
	}

	return dif / 2;
}

/** Draws a Myth/FF7 style viewing window */
static void setViewingWindow()
{
	float pixSizeH, pixSizeV;
	Vector3i v[4] = {{0, 0, 0}}, tv[4] = {{0, 0, 0}}, centre = {0, 0, 0};
	int	shortX, longX, yDrop, yDropVar;
	int	dif = getDistanceAdjust();
	int	dif2 = getLengthAdjust();
	PIELIGHT colour;
	CalcRadarPixelSize(&pixSizeH, &pixSizeV);
	int x = static_cast<int>(playerPos.p.x * pixSizeH / TILE_UNITS);
	int y = static_cast<int>(playerPos.p.z * pixSizeV / TILE_UNITS);

	shortX = static_cast<int>(((visibleTiles.x / 4) - (dif / 6)) * pixSizeH);
	longX = static_cast<int>(((visibleTiles.x / 2) - (dif / 4)) * pixSizeH);
	yDropVar = static_cast<int>(((visibleTiles.y / 2) - (dif2 / 3)) * pixSizeV);
	yDrop = static_cast<int>(((visibleTiles.y / 2) - dif2 / 3) * pixSizeV);

	v[0].x = longX;
	v[0].y = -yDropVar;

	v[1].x = -longX;
	v[1].y = -yDropVar;

	v[2].x = shortX;
	v[2].y = yDrop;

	v[3].x = -shortX;
	v[3].y = yDrop;

	centre.x = static_cast<int>(x - scrollMinX * pixSizeH);
	centre.y = static_cast<int>(y - scrollMinY * pixSizeV);

	RotateVector2D(v, tv, &centre, playerPos.r.y, 4);

	switch (getCampaignNumber())
	{
	case 1:
	case 2:
		// white
		colour.byte.r = UBYTE_MAX;
		colour.byte.g = UBYTE_MAX;
		colour.byte.b = UBYTE_MAX;
		colour.byte.a = 0x3f;
		break;
	case 3:
		// greenish
		colour.byte.r = 0x3f;
		colour.byte.g = UBYTE_MAX;
		colour.byte.b = 0x3f;
		colour.byte.a = 0x3f;
		break;
	default:
		// black
		colour.rgba = 0;
		colour.byte.a = 0x3f;
		break;
	}

	/* Send the four points to the draw routine and the clip box params */
	pie_SetViewingWindow(tv, colour);
}

static void DrawRadarExtras(const glm::mat4 &modelViewProjectionMatrix)
{
	pie_DrawViewingWindow(modelViewProjectionMatrix);
	RenderWindowFrame(FRAME_RADAR, -1, -1, radarWidth + 2, radarHeight + 2, modelViewProjectionMatrix);
}

/** Does a screen coordinate lie within the radar area? */
bool CoordInRadar(int x, int y)
{
	Vector2f pos;
	pos.x = x - radarCenterX;
	pos.y = y - radarCenterY;
	if (rotateRadar)
	{
		pos = Vector2f_Rotate2f(pos, -playerPos.r.y);
	}
	pos.x += radarWidth / 2.f;
	pos.y += radarHeight / 2.f;

	if (pos.x < 0 || pos.y < 0 || pos.x >= radarWidth || pos.y >= radarHeight)
	{
		return false;
	}
	return true;
}

void radarColour(UDWORD tileNumber, uint8_t r, uint8_t g, uint8_t b)
{
	tileColours[tileNumber].byte.r = r;
	tileColours[tileNumber].byte.g = g;
	tileColours[tileNumber].byte.b = b;
	tileColours[tileNumber].byte.a = 255;
}
