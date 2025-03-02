include("script/campaign/transitionTech.js");
include("script/campaign/libcampaign.js");
include("script/campaign/templates.js");

const TRANSPORT_LIMIT = 4;
var transporterIndex; //Number of transport loads sent into the level
var startedFromMenu;

camAreaEvent("vtolRemoveZone", function(droid)
{
	camSafeRemoveObject(droid, false);
	resetLabel("vtolRemoveZone", THE_COLLECTIVE);
});

//Attack and destroy all those who resist the Machine! -The Collective
function secondVideo()
{
	camPlayVideos({video: "MB2A_MSG2", type: CAMP_MSG});
}

//Damage the base and droids for the player
function preDamageStuff()
{
	var droids = enumDroid(CAM_HUMAN_PLAYER);
	var structures = enumStruct(CAM_HUMAN_PLAYER);
	var x = 0;

	for (x = 0; x < droids.length; ++x)
	{
		var droid = droids[x];
		if (!camIsTransporter(droid))
		{
			setHealth(droid, 45 + camRand(20));
		}
	}

	for (x = 0; x < structures.length; ++x)
	{
		var struc = structures[x];
		setHealth(struc, 45 + camRand(45));
	}
}

function getDroidsForCOLZ()
{
	var droids = [];
	var count = 6 + camRand(5);
	var templates;
	var sensors = [cTempl.comsens, cTempl.comsens];
	var usingHeavy = false;

	if (camRand(100) < 50)
	{
		templates = [cTempl.npcybm, cTempl.commgt, cTempl.npcybc, cTempl.npcybr];
	}
	else
	{
		templates = [cTempl.cohct, cTempl.commrl, cTempl.comorb];
		usingHeavy = true;
	}

	for (let i = 0; i < count; ++i)
	{
		if (!i && usingHeavy)
		{
			droids.push(sensors[camRand(sensors.length)]); //bring a sensor
		}
		else
		{
			droids.push(templates[camRand(templates.length)]);
		}
	}

	return droids;
}

//Send Collective transport units
function sendCOTransporter()
{
	var tPos = getObject("COTransportPos");
	var nearbyDefense = enumRange(tPos.x, tPos.y, 15, THE_COLLECTIVE, false);

	if (nearbyDefense.length > 0)
	{
		var list = getDroidsForCOLZ();
		camSendReinforcement(THE_COLLECTIVE, camMakePos("COTransportPos"), list,
			CAM_REINFORCE_TRANSPORT, {
				entry: { x: 125, y: 100 },
				exit: { x: 125, y: 70 }
			}
		);
	}
	else
	{
		removeTimer("sendCOTransporter");
	}
}

//Extra transport units are only awarded to those who start Beta campaign
//from the main menu. Otherwise a player can just bring in there Alpha units
function sendPlayerTransporter()
{
	if (!camDef(transporterIndex))
	{
		transporterIndex = 0;
	}

	if (transporterIndex === TRANSPORT_LIMIT)
	{
		downTransporter();
		return;
	}

	var droids = [];
	var list = [cTempl.prhct, cTempl.prhct, cTempl.prhct, cTempl.prltat, cTempl.prltat, cTempl.npcybr, cTempl.prrept];

	for (let i = 0; i < 10; ++i)
	{
		droids.push(list[camRand(list.length)]);
	}

	camSendReinforcement(CAM_HUMAN_PLAYER, camMakePos("landingZone"), droids,
		CAM_REINFORCE_TRANSPORT, {
			entry: { x: 87, y: 126 },
			exit: { x: 87, y: 126 }
		}
	);
}

//Continuously spawns heavy units on the north part of the map every 7 minutes
function mapEdgeDroids()
{
	var TankNum = 8 + camRand(6);
	var list = [cTempl.npcybm, cTempl.npcybr, cTempl.commrp, cTempl.cohct];

	var droids = [];
	for (let i = 0; i < TankNum; ++i)
	{
		droids.push(list[camRand(list.length)]);
	}

	camSendReinforcement(THE_COLLECTIVE, camMakePos("groundUnitPos"), droids, CAM_REINFORCE_GROUND);
}

function vtolAttack()
{
	var list = [cTempl.colcbv];
	camSetVtolData(THE_COLLECTIVE, "vtolAppearPos", "vtolRemoveZone", list, camChangeOnDiff(camMinutesToMilliseconds(3)), "COCommandCenter");
}

function groupPatrol()
{
	camManageGroup(camMakeGroup("edgeGroup"), CAM_ORDER_ATTACK, {
		regroup: true,
		count: -1,
	});

	camManageGroup(camMakeGroup("IDFGroup"), CAM_ORDER_DEFEND, {
		pos: [
			camMakePos("waypoint1"),
			camMakePos("waypoint2")
		]
	});

	camManageGroup(camMakeGroup("sensorGroup"), CAM_ORDER_PATROL, {
		pos: [
			camMakePos("waypoint1"),
			camMakePos("waypoint2")
		]
	});
}

//Build defenses around oil resource
function truckDefense()
{
	if (enumDroid(THE_COLLECTIVE, DROID_CONSTRUCT).length === 0)
	{
		removeTimer("truckDefense");
		return;
	}

	const DEFENSES = ["CO-Tower-LtATRkt", "PillBox1", "CO-WallTower-HvCan"];
	camQueueBuilding(THE_COLLECTIVE, DEFENSES[camRand(DEFENSES.length)]);
}

//Gives starting tech and research.
function cam2Setup()
{
	const COLLECTIVE_RES = [
		"R-Wpn-MG1Mk1", "R-Sys-Engineering02",
		"R-Defense-WallUpgrade06", "R-Struc-Materials06",
		"R-Vehicle-Engine03", "R-Vehicle-Metals03", "R-Cyborg-Metals03",
		"R-Wpn-Cannon-Accuracy02", "R-Wpn-Cannon-Damage04",
		"R-Wpn-Cannon-ROF01", "R-Wpn-Flamer-Damage03", "R-Wpn-Flamer-ROF01",
		"R-Wpn-MG-Damage05", "R-Wpn-MG-ROF02", "R-Wpn-Mortar-Acc01",
		"R-Wpn-Mortar-Damage03", "R-Wpn-Mortar-ROF01",
		"R-Wpn-Rocket-Accuracy02", "R-Wpn-Rocket-Damage04",
		"R-Wpn-Rocket-ROF03", "R-Wpn-RocketSlow-Accuracy03",
		"R-Wpn-RocketSlow-Damage04", "R-Sys-Sensor-Upgrade01"
	];

	for (let x = 0, l = STRUCTS_ALPHA.length; x < l; ++x)
	{
		enableStructure(STRUCTS_ALPHA[x], CAM_HUMAN_PLAYER);
	}

	camCompleteRequiredResearch(PLAYER_RES_BETA, CAM_HUMAN_PLAYER);
	camCompleteRequiredResearch(ALPHA_RESEARCH_NEW, THE_COLLECTIVE);
	camCompleteRequiredResearch(COLLECTIVE_RES, THE_COLLECTIVE);
	camCompleteRequiredResearch(ALPHA_RESEARCH_NEW, CAM_HUMAN_PLAYER);

	if (difficulty >= HARD)
	{
		camUpgradeOnMapTemplates(cTempl.commc, cTempl.commrp, THE_COLLECTIVE);
	}

	enableResearch("R-Wpn-Cannon-Damage04", CAM_HUMAN_PLAYER);
	enableResearch("R-Wpn-Rocket-Damage04", CAM_HUMAN_PLAYER);
	preDamageStuff();
}

//Get some higher rank droids.
function setUnitRank(transport)
{
	const DROID_EXP = [128, 64, 32, 16];
	var droids;
	var mapRun = false;

	if (transport)
	{
		droids = enumCargo(transport);
	}
	else
	{
		mapRun = true;
		//These are the units in the base already at the start.
		droids = enumDroid(CAM_HUMAN_PLAYER).filter((dr) => (!camIsTransporter(dr)));
	}

	for (let i = 0, len = droids.length; i < len; ++i)
	{
		var droid = droids[i];
		if (!camIsSystemDroid(droid))
		{
			setDroidExperience(droid, DROID_EXP[mapRun ? 0 : (transporterIndex - 1)]);
		}
	}
}

//Bump the rank of the first batch of transport droids as a reward.
function eventTransporterLanded(transport)
{
	if (transport.player === CAM_HUMAN_PLAYER)
	{
		if (!camDef(transporterIndex))
		{
			transporterIndex = 0;
		}

		transporterIndex += 1;

		if (startedFromMenu)
		{
			setUnitRank(transport);
		}

		if (transporterIndex >= TRANSPORT_LIMIT)
		{
			queue("downTransporter", camMinutesToMilliseconds(1));
		}
	}
}

//Warn that something bad happened to the fifth transport
function reallyDownTransporter()
{
	if (startedFromMenu)
	{
		removeTimer("sendPlayerTransporter");
	}
	setReinforcementTime(LZ_COMPROMISED_TIME);
	playSound("pcv443.ogg");
}

function downTransporter()
{
	camCallOnce("reallyDownTransporter");
}

function eventTransporterLaunch(transport)
{
	if (transporterIndex >= TRANSPORT_LIMIT)
	{
		queue("downTransporter", camMinutesToMilliseconds(1));
	}
}

function eventGameLoaded()
{
	if (transporterIndex >= TRANSPORT_LIMIT)
	{
		setReinforcementTime(LZ_COMPROMISED_TIME);
	}
}

function eventStartLevel()
{
	const PLAYER_POWER = 5000;
	var startpos = getObject("startPosition");
	var lz = getObject("landingZone"); //player lz
	var enemyLz = getObject("COLandingZone");
	var tent = getObject("transporterEntry");
	var text = getObject("transporterExit");

	camSetStandardWinLossConditions(CAM_VICTORY_STANDARD, "SUB_2_1S");
	setReinforcementTime(LZ_COMPROMISED_TIME);

	centreView(startpos.x, startpos.y);
	setNoGoArea(lz.x, lz.y, lz.x2, lz.y2, CAM_HUMAN_PLAYER);
	setNoGoArea(enemyLz.x, enemyLz.y, enemyLz.x2, enemyLz.y2, 5);
	startTransporterEntry(tent.x, tent.y, CAM_HUMAN_PLAYER);
	setTransporterExit(text.x, text.y, CAM_HUMAN_PLAYER);

	camSetArtifacts({
		"COCommandCenter": { tech: "R-Sys-Engineering02" },
		"COArtiPillbox": { tech: "R-Wpn-MG-ROF02" },
		"COArtiCBTower": { tech: "R-Sys-Sensor-Upgrade01" },
	});

	setMissionTime(camChangeOnDiff(camHoursToSeconds(1)));
	setPower(PLAYER_POWER, CAM_HUMAN_PLAYER);
	cam2Setup();

	//C2A_BASE2 is not really a base
	camSetEnemyBases({
		"CONorthBase": {
			cleanup: "CONorth",
			detectMsg: "C2A_BASE1",
			detectSnd: "pcv379.ogg",
			eliminateSnd: "pcv394.ogg",
		},
		"CONorthWestBase": {
			cleanup: "CONorthWest",
			detectMsg: "C2A_BASE2",
			detectSnd: "pcv379.ogg",
			eliminateSnd: "pcv394.ogg",
		},
	});

	camManageTrucks(THE_COLLECTIVE);
	setUnitRank(); //All pre-placed player droids are ranked.
	camPlayVideos({video: "MB2A_MSG", type: MISS_MSG});
	startedFromMenu = false;

	//Only if starting Beta directly rather than going through Alpha
	if (enumDroid(CAM_HUMAN_PLAYER, DROID_SUPERTRANSPORTER).length === 0)
	{
		startedFromMenu = true;
		sendPlayerTransporter();
		setTimer("sendPlayerTransporter", camMinutesToMilliseconds(5));
	}
	else
	{
		setReinforcementTime(camMinutesToSeconds(5)); // 5 min.
	}

	queue("secondVideo", camSecondsToMilliseconds(12));
	queue("groupPatrol", camChangeOnDiff(camMinutesToMilliseconds(1)));
	queue("vtolAttack", camChangeOnDiff(camMinutesToMilliseconds(3)));
	setTimer("truckDefense", camChangeOnDiff(camMinutesToMilliseconds(3)));
	setTimer("sendCOTransporter", camChangeOnDiff(camMinutesToMilliseconds(4)));
	setTimer("mapEdgeDroids", camChangeOnDiff(camMinutesToMilliseconds(7)));

	truckDefense();
}
