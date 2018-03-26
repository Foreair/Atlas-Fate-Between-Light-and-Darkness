#pragma once

#include "comp_base.h"
#include "fsm/context.h"


struct TMsgSetFSMVariable {
	CVariant variant;
	DECL_MSG_ID();
};

class TCompFSM : public TCompBase {
public:

	bool isPaused;
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void onPaused(const TMsgScenePaused& msg);
	void onCreate(const TMsgEntityCreated& msg);
	void onVariableValue(const TMsgSetFSMVariable& msg);

	static void registerMsgs();

private:

	FSM::CContext _context;
	DECL_SIBLING_ACCESS();
};