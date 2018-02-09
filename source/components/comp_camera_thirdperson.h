#pragma once

#include "comp_base.h"
#include "camera/camera.h"
#include "entity/common_msgs.h"

class TCompCameraThirdPerson : public CCamera, public TCompBase
{
private:
	CHandle     _h_target;
	std::string _target_name;

	float _speed;
	VEC2 _clamp_angle;
	VEC2 _current_euler;
	VEC3 _clipping_offset;

public:
  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);

  VEC3 CameraClipping(void);

  DECL_SIBLING_ACCESS();
};