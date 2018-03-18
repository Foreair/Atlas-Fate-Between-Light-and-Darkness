#ifndef _AIC_PATROL
#define _AIC_PATROL

#include "ai_controller.h"
#include "entity/common_msgs.h"
#include "modules/module_ia.h"

class CAIPatrol : public IAIController
{

	/* Atributes */
	std::vector<Waypoint> _waypoints;
	int currentWaypoint;


	float speed = 5.5f;
	float rotationSpeedDeg = 90.0f;
	float rotationSpeed;
	std::string entityToChase = "The Player";
	float fovDeg = 70.f;
	float fov;
	float autoChaseDistance = 15.f;
	float maxChaseDistance = 35.f;
	float maxTimeSuspecting = 3.f;
	float suspectO_Meter = 0.f;
	float dcrSuspectO_Meter = .3f;
	float incrBaseSuspectO_Meter = .5f;
	bool isLastPlayerKnownDirLeft = false;
	float distToAttack = 1.5f;
	float amountRotated = 0.f;
	float maxRotationSeekingPlayerDeg = 90.f;
	float maxRotationSeekingPlayer;
	VEC3 lastPlayerKnownPos = VEC3::Zero;
	VEC3 lastStunnedPatrolKnownPos = VEC3::Zero;
	bool startLightsOn = false;

	/* Timers */
	float timerWaitingInWpt = 0.f;

	DECL_SIBLING_ACCESS();

	void onMsgPatrolCreated(const TMsgEntityCreated& msg);
	void onMsgPlayerDead(const TMsgPlayerDead& msg);
	void onMsgPatrolStunned(const TMsgEnemyStunned& msg);
	void onMsgPatrolShadowMerged(const TMsgPatrolShadowMerged& msg);
	void onMsgPatrolFixed(const TMsgPatrolFixed& msg);

	void rotateTowardsVec(VEC3 objective, float dt);
	bool isPlayerInFov();
	bool isEntityHidden(CHandle h_entity);
	bool isStunnedPatrolInFov();
	bool isStunnedPatrolInPos(VEC3 lastPos);
	CHandle getPatrolInPos(VEC3 lastPos);
	void turnOnLight();
	void turnOffLight();

public:
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	void renderDebug();

	void IdleState(float);				//Does nothing
	void GoToWptState(float);				//Moves to currentWpt
	void WaitInWptState(float);			//Rotates to the desired position and wait for x time
	void NextWptState(float);				//Calculates next currentWpt
	void ClosestWptState(float);			//Calculates the closest waypoint
	void SuspectState(float);				//The enemy thinks it may have seen the player. More or less time depending different factors
	void ShootInhibitorState(float);		//The enemy shoots the inhibitor to the player
	void ChaseState(float);				//Goes towards the player's position				
	void AttackState(float);				//Attack the player
	void IdleWarState(float);				//Waiting before attacks
	void GoToNoiseState(float);			//The enemy goes to the location where he heard something
	void GoToPatrolState(float);			//The enemy goes to the location of a stunned enemy in order to repair it
	void FixOtherPatrolState(float);		//The enemy fixes another enemy
	void GoPlayerLastPosState(float);		//The enemy goes to the last position he has seen the player
	void SeekPlayerState(float);			//The enemy keeps searching the player a bit more
	void StunnedState(float);				//The enemy has been stunned by the player
	void FixedState(float);				//The enemy has been fixed by another enemy
	void ShadowMergedState(float);		//The enemy has been shadowmerged by the player

	void BackState(float);

  void Init();

  void addWaypoint(Waypoint wpt) { _waypoints.push_back(wpt); };
  Waypoint getWaypoint() { return _waypoints[currentWaypoint]; }

  static void registerMsgs();
};

#endif