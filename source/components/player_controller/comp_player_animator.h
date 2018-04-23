#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"
#include "components/skeleton/comp_animator.h"

class TCompPlayerAnimator;

class TCompPlayerAnimator : public TCompAnimator
{
public:
	enum EAnimation { IDLE = 0, WALK, ATTACK, DEATH, DEAD, CROUCH_IDLE, CROUCH_WALK, CROUCH_WALK_SLOW, RUN, HIT_FRONT, HIT_BACK, FALL, LAND_SOFT, METRALLA_START, METRALLA_MIDDLE, METRALLA_FINISH, SM_ENTER, SM_EXIT, SM_POSE};
	struct TMsgExecuteAnimation {

		TCompPlayerAnimator::EAnimation animation;
		float speed;
		DECL_MSG_ID();
	};
	
	DECL_SIBLING_ACCESS();

	void debugInMenu();
	static void registerMsgs();
	void onCreated(const TMsgEntityCreated& msg);
	void playMsgAnimation(const TMsgExecuteAnimation& msg);

	
	void initializeAnimations();
	bool playAnimation(TCompPlayerAnimator::EAnimation animation, float speed = 1.0f);
};

