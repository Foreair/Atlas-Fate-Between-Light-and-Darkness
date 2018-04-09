#pragma once

#include "components/comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"

class CPhysicsCollider;
class TCompTransform;

class TCompCollider : public TCompBase {

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);
	void onTriggerEnter(const TMsgTriggerEnter& msg);
	void onTriggerExit(const TMsgTriggerExit& msg);

	DECL_SIBLING_ACCESS();

public:

	bool player_inside;
	std::map<uint32_t, TCompTransform*> handles;

	// Collider parameter description
	CPhysicsCollider * config;

	~TCompCollider();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	/* Auxiliar methods */
	bool collisionDistance(const VEC3 & org, const VEC3 & dir, float maxDistance);

	static void registerMsgs();
};