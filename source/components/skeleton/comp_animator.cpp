#include "mcv_platform.h"
#include "comp_animator.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("animator", TCompAnimator);

void TCompAnimator::debugInMenu() {
	
}

void TCompAnimator::load(const json& j, TEntityParseContext& ctx) {
	_animationName = "";

}

void TCompAnimator::initializeAnimations() {
	
	
}

bool TCompAnimator::initializeAnimation(EAnimation animation, EAnimationType animationType, EAnimationSize animationSize, std::string animationName, std::string secondAnimationName, float weight) {
	
	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	AnimationSet auxAnimSet;
	auxAnimSet.animation = animation;
	auxAnimSet.animationType = animationType;
	auxAnimSet.animationSize = animationSize;
	auxAnimSet.animationName = animationName;
	auxAnimSet.animationId = compSkeleton->getAnimationIdByName(auxAnimSet.animationName);
	if (auxAnimSet.animationId == -1) {
		fatal("The first animation non exists");
		return false;
	}
	auxAnimSet.secondAnimationName = secondAnimationName;
	auxAnimSet.secondAnimationId = compSkeleton->getAnimationIdByName(auxAnimSet.secondAnimationName);
	if (auxAnimSet.animationSize == EAnimationSize::DOUBLE && auxAnimSet.secondAnimationId == -1) {
		fatal("The second animation non exists");
		return false;
	}
	auxAnimSet.weight = weight;
	animationsMap[animation] = auxAnimSet;
	return false;
}

void TCompAnimator::update(float dt)
{

}

void TCompAnimator::registerMsgs() {
//	DECL_MSG(TCompAnimator, TMsgEntityCreated, onCreated);
}



bool TCompAnimator::playAnimation(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		return false;
	}
	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	AnimationSet animSet = animationsMap[animation];

	switch (animSet.animationType)
	{
	case EAnimationType::CYCLIC:

		if (animSet.animationType == EAnimationSize::DOUBLE) {
			compSkeleton->changeCyclicAnimation(animSet.animationId, animSet.secondAnimationId, animSet.weight);
		}
		else {
			compSkeleton->changeCyclicAnimation(animSet.animationId);
		}
		break;

	case EAnimationType::ACTION:
		compSkeleton->executeActionAnimation(animSet.animationId);
		break;

	}

}

bool  TCompAnimator::isCyclic(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationSet animSet = animationsMap[animation];
	return animSet.animationType == EAnimationType::CYCLIC;
}

bool  TCompAnimator::isComposed(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationSet animSet = animationsMap[animation];
	return animSet.animationSize == EAnimationSize::DOUBLE;
}

TCompAnimator::EAnimation TCompAnimator::actualAnimation() {

	return actualCyclicAnimation.animation;
}
