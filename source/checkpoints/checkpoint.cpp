#include "mcv_platform.h"
#include "checkpoint.h"
#include "components/comp_tags.h"
#include "components/comp_transform.h"
#include "components/ia/comp_bt_patrol.h"
#include "components/ia/comp_bt_mimetic.h"
#include "entity/common_msgs.h"
#include "components/physics/comp_rigidbody.h"

CCheckpoint::CCheckpoint() {
	saved = false;
}

//bool CCheckpoint::init() {
//	saved = false;
//	return true;
//}

bool CCheckpoint::saveCheckPoint(VEC3 playerPos, QUAT playerRotation)
{
	bool error = false;
  enemies.clear();
  entities.clear();

	player.playerPos = playerPos;
	player.playerRot = playerRotation;
	player.saved = true;

	VHandles v_enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	for (int i = 0; i < v_enemies.size(); i++) {
		CEntity* e_enemy = v_enemies[i];

		EnemyStatus enemy;

		TCompTransform* enemyTransform = e_enemy->get<TCompTransform>();
		assert(enemyTransform);
		enemy.enemyPosition = enemyTransform->getPosition();
		enemy.enemyRotation = enemyTransform->getRotation();

		assert(e_enemy->getName());
		enemy.enemyName = std::string(e_enemy->getName());

		TCompAIPatrol * enemyPatrol = e_enemy->get<TCompAIPatrol>();
		TCompAIMimetic * enemyMimetic = e_enemy->get<TCompAIMimetic>();
		assert(enemyPatrol || enemyMimetic);
		if (enemyPatrol != nullptr) {
			enemy.enemyType = TCompIAController::BTType::PATROL;
			enemy.enemyIAStateName = enemyPatrol->getStateForCheckpoint();
		}
		else if(enemyMimetic != nullptr) {
			enemy.enemyType = TCompIAController::BTType::MIMETIC;
			enemy.enemyIAStateName = enemyMimetic->getStateForCheckpoint();
		}

		enemy.saved = true;
		enemies.push_back(enemy);
	}

	saved = true;

	return !error;
}

bool CCheckpoint::loadCheckPoint()
{
	if (saved) {

		/* Player loading */
		CHandle h_player = getEntityByName("The Player");
		if (h_player.isValid() && player.saved) {
			CEntity * e_player = h_player;
			TCompTransform * playerTransform = e_player->get<TCompTransform>();
			playerTransform->setPosition(player.playerPos);
			playerTransform->setRotation(player.playerRot);
      TCompRigidbody * playerRigidbody = e_player->get<TCompRigidbody>();
      playerRigidbody->setGlobalPose(player.playerPos, player.playerRot);

      /* Player Cameras */
      VHandles v_cameras = CTagsManager::get().getAllEntitiesByTag(getID("main_camera"));
      for (int i = 0; i < v_cameras.size(); i++) {
        CEntity* camera = v_cameras[i];
        TMsgCameraReset msg;
        camera->sendMsg(msg);
      }
		}

		/* Enemies loading */
		VHandles v_enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
		for (int i = 0; i < v_enemies.size(); i++) {
			CEntity * e_enemy = v_enemies[i];
			
			/* Iterate through all the enemies in the checkpoint. If someone is missing, delete it (because it means it has been stunned) */
			bool found = false;
			int j = 0;
			while (j < enemies.size() && !found) {
				if (enemies[j].saved && enemies[j].enemyName.compare(e_enemy->getName()) == 0) {
					found = true;
				}
				else {
					j++;
				}
			}

			if (found) {
				/* update its position and state */
				TCompTransform * enemyTransform = e_enemy->get<TCompTransform>();
				enemyTransform->setPosition(enemies[j].enemyPosition);
				enemyTransform->setRotation(enemies[j].enemyRotation);
        TCompRigidbody * enemyRigidbody = e_enemy->get<TCompRigidbody>();
        enemyRigidbody->setGlobalPose(enemies[j].enemyPosition, enemies[j].enemyRotation);

				switch (enemies[j].enemyType) {
					case TCompAIMimetic::BTType::PATROL:
					{
						TCompAIPatrol* enemyAI = e_enemy->get<TCompAIPatrol>();
						enemyAI->setCurrentByName(enemies[j].enemyIAStateName);
						break;
					}
					case TCompAIMimetic::BTType::MIMETIC:
					{
            TCompAIMimetic* enemyAI = e_enemy->get<TCompAIMimetic>();
						enemyAI->setCurrentByName(enemies[j].enemyIAStateName);
						break;
					}
				}
			}
			else {
				/* delete entity */
				v_enemies[i].destroy();
			}
		}

		return true;
	}
	else {
		return false;
	}
}

bool CCheckpoint::deleteCheckPoint()
{
	player.saved = false;
	enemies.clear();
	entities.clear();
	saved = false;

	return true;
}

void CCheckpoint::debugInMenu()
{
  if (ImGui::TreeNode("Checkpoint")) {

    ImGui::Text("Saved: ");
    ImGui::SameLine();
    if (saved) {
      ImGui::TextColored(ImVec4(0, 255, 0, 255), "TRUE");
      if (ImGui::TreeNode("Player")) {
        ImGui::Text("Position: (%f, %f, %f)", player.playerPos.x, player.playerPos.y, player.playerPos.z);
        ImGui::Text("Rotation: (%f, %f, %f)", player.playerRot.x, player.playerRot.y, player.playerRot.z, player.playerRot.w);
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Enemies")) {
        for (int i = 0; i < enemies.size(); i++) {
          if (ImGui::TreeNode(enemies[i].enemyName.c_str())) {
            ImGui::Text("Position: (%f, %f, %f)", enemies[i].enemyPosition.x, enemies[i].enemyPosition.y, enemies[i].enemyPosition.z);
            ImGui::Text("State: %s", enemies[i].enemyIAStateName);
            switch (enemies[i].enemyType) {
              case TCompAIMimetic::BTType::PATROL:
              {
                ImGui::Text("Type: Patrol");
                break;
              }
              case TCompAIMimetic::BTType::MIMETIC:
              {
                ImGui::Text("Type: Mimetic");
                break;
              }
            }
            ImGui::TreePop();
          }
        }
        ImGui::TreePop();
      }
    }
    else {
      ImGui::TextColored(ImVec4(255, 0, 0, 255), "FALSE");

    }

    ImGui::TreePop();
  }
}