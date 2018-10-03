#include "mcv_platform.h"
#include "module_scene_manager.h"
#include "handle/handle.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "modules/game/module_game_manager.h"
#include "components/comp_group.h"
#include "components/lighting/comp_light_point.h"
#include "components/comp_render.h"
#include "components/comp_transform.h"
#include "render/texture/material.h"
#include "render/render_objects.h"
#include "resources/json_resource.h"

// for convenience
using json = nlohmann::json;

CModuleSceneManager::CModuleSceneManager(const std::string& name)
    : IModule(name)
{}

/* Pre-load all the scenes from boot.json */
void CModuleSceneManager::loadJsonScenes(const std::string filepath) {

    sceneCount = 0;

    json jboot = loadJson(filepath);
    _default_scene = jboot.value("default_scene","scene_intro");

    for (auto it = std::next(jboot.begin(),1); it != jboot.end(); ++it) {
   
        sceneCount++;
        std::string scene_name = it.key();
        std::vector< std::string > groups_subscenes = jboot[scene_name]["scene_group"];
        
        // Create the scene and store it
        Scene * scene = createScene(scene_name);
        scene->groups_subscenes = groups_subscenes;
        auto& data = jboot[scene_name]["static_data"];
        scene->navmesh = data.value("navmesh", "");
        scene->initial_script_name = data.value("initial_script", "");

        scene->env_fog = data.count("env_fog") ? loadVEC3(data["env_fog"]) : cb_globals.global_fog_env_color;
        scene->ground_fog = data.count("ground_fog") ? loadVEC3(data["ground_fog"]) : cb_globals.global_fog_color;
        scene->shadow_color = data.count("shadow_color") ? loadVEC3(data["shadow_color"]) : cb_globals.global_shadow_color;

        scene->env_fog_density = data.value("env_fog_density", cb_globals.global_fog_density);
        scene->ground_fog_density = data.value("ground_fog_density", cb_globals.global_fog_ground_density);

        scene->scene_exposure = data.value("exposure", cb_globals.global_exposure_adjustment);
        scene->scene_ambient = data.value("ambient", cb_globals.global_ambient_adjustment);
        scene->scene_gamma = data.value("gamma", cb_globals.global_gamma_correction_enabled);
        scene->scene_tone_mapping = data.value("tone_mapping", cb_globals.global_tone_mapping_mode);
        scene->scene_shadow_intensity = data.value("shadow_intensity", cb_globals.global_shadow_intensity);

        _scenes.insert(std::pair<std::string, Scene*>(scene_name, scene));
    }
}

bool CModuleSceneManager::start() {

    // Load a persistent scene and the listed ones
    // Store at persistent scene, inviolable data.
    _persistentScene = createScene("Persistent_Scene");
    _persistentScene->isLoaded = true;

    loadJsonScenes("data/boot.json");
    //std::async(std::launch::async, &CModuleSceneManager::sceneThreadMain, this);
    sceneThread = std::thread(&CModuleSceneManager::sceneThreadMain, this);
    //sceneThread.detach();

    return true;
}

bool CModuleSceneManager::stop() {

    unLoadActiveScene();

    ending_thread = true;
    sceneThread.join();

    return true;
}

void CModuleSceneManager::update(float delta) {

    // TO-DO, Maybe not needed
}

Scene* CModuleSceneManager::createScene(const std::string& name) {

    Scene* scene = new Scene();
    scene->name = name;
    scene->navmesh = "UNDEFINED";
    scene->initial_script_name = "UNDEFINED";
    scene->isLoaded = false;

    return scene;
}

/* Method used to load a listed scene (must be in the database) */
bool CModuleSceneManager::loadScene(const std::string & name) {

    auto it = _scenes.find(name);
    if (it != _scenes.end())
    {
        // Send a message to notify the scene loading.
        // Useful if we want to show a load splash menu

        if (_activeScene != nullptr && _activeScene->name != name) {
            /* If the new scene is different from the actual one => delete checkpoint */
            CModuleGameManager gameManager = CEngine::get().getGameManager();
            gameManager.deleteCheckpoint();
        }

        unLoadActiveScene();

        // Load the subscene
        Scene * current_scene = it->second;
        if (current_scene->navmesh.compare("") != 0) {
            Engine.getNavmeshes().buildNavmesh(current_scene->navmesh);
        }

        for (auto& scene_name : current_scene->groups_subscenes) {
            dbg("Autoloading scene %s\n", scene_name.c_str());
            TEntityParseContext ctx;
            parseScene(scene_name, ctx);
        }

        // Renew the active scene
        current_scene->isLoaded = true;
        setActiveScene(current_scene);

        // Move this to LUA.
        CHandle h_camera = getEntityByName("TPCamera");
        if (h_camera.isValid())
            Engine.getCameras().setDefaultCamera(h_camera);

        h_camera = getEntityByName("main_camera");
        if (h_camera.isValid())
            Engine.getCameras().setOutputCamera(h_camera);

        TMsgSceneCreated msg;
        EngineEntities.broadcastMsg(msg);

		CModuleGameManager gameManager = CEngine::get().getGameManager();
		/* TODO: Comprobar que se sigue en la misma escena */
		gameManager.loadCheckpoint();
        Engine.getLogic().execEvent(EngineLogic.SCENE_START, current_scene->name);

        // Set the global data.
        cb_globals.global_fog_color = current_scene->ground_fog;
        cb_globals.global_fog_env_color = current_scene->env_fog;
        cb_globals.global_fog_density = current_scene->env_fog_density;
        cb_globals.global_fog_ground_density = current_scene->ground_fog_density;
        cb_globals.global_exposure_adjustment = current_scene->scene_exposure;
        cb_globals.global_ambient_adjustment = current_scene->scene_ambient;
        cb_globals.global_gamma_correction_enabled = current_scene->scene_gamma;
        cb_globals.global_tone_mapping_mode = current_scene->scene_tone_mapping;
        cb_globals.global_shadow_intensity = current_scene->scene_shadow_intensity;
        cb_globals.global_shadow_color = current_scene->shadow_color;
        cb_globals.updateGPU();

        return true;
    }

    return false;
}

bool CModuleSceneManager::unLoadActiveScene() {

    // This will allow us to mantain the gamestate.

    // Get the current active scene
    // Free memory related to non persistent data.
    // Warning: persistent data will need to avoid deletion
    if (_activeScene != nullptr) {

        EngineLogic.clearDelayedScripts();
        EngineLogic.execEvent(EngineLogic.SCENE_END, _activeScene->name);

        EngineEntities.destroyAllEntities();
        EngineCameras.deleteAllCameras();
        EngineIA.clearSharedBoards();
        EngineNavmeshes.destroyNavmesh();
        EngineInstancing.clearInstances();
        EngineParticles.killAll();

        _activeScene->isLoaded = false;
        _activeScene = nullptr;

	    /* TODO: Delete checkpoint */
        return true;
    }

    return false;
}

/* Some getters and setters */

Scene* CModuleSceneManager::getActiveScene() {

    return _activeScene;
}

Scene* CModuleSceneManager::getSceneByName(const std::string& name) {

    return _scenes[name];
}

void CModuleSceneManager::setActiveScene(Scene* scene) {

    //unLoadActiveScene();
    _activeScene = scene;
}

std::string CModuleSceneManager::getDefaultSceneName() {
    return _default_scene;
}

void CModuleSceneManager::sceneThreadMain()
{
    while (!ending_thread) {

        //Resources
        const std::string resource = Resources.getFirstPendingResource();
        if (resource.compare("") != 0) {

            const json& j = Resources.get(resource)->as<CJsonResource>()->getJson();
            std::vector<PreResource> scene_resources;
            dbg("Ejecutando thread con resource %s\n", resource.c_str());
            parseResourceScene(j, scene_resources);
            Sleep(1);
            for (int i = 0; i < scene_resources.size(); i++) {
                Resources.get(scene_resources[i].name);
                Sleep(scene_resources[i].isBig ? 1000 : 100);
            }

        }
        Sleep(1);
    }
    dbg("Ending thread\n");
}

void CModuleSceneManager::preloadScene(const std::string& sceneName) {
    auto it = _scenes.find(sceneName);
    if (it != _scenes.end())
    {
        for (auto& scene_name : it->second->groups_subscenes) {
            dbg("Preloading scene %s\n", scene_name.c_str());
            Resources.addPendingResource(scene_name);
        }
    }
}

void CModuleSceneManager::parseResourceScene(const json& j, std::vector<PreResource>& scene_resources) {

    assert(j.is_array());
    for (int i = 0; i < j.size(); i++){

        auto& j_item = j[i];

        assert(j_item.is_object());

        int n_entities = j_item.count("entity");
        if (n_entities) {
            auto& j_entity = j_item["entity"];

            if (j_entity.count("prefab")) {

                std::string prefab_src = j_entity["prefab"];
                const json& j_prefab = Resources.get(prefab_src)->as<CJsonResource>()->getJson();

                parseResourceScene(j_prefab, scene_resources);
            }
            else {
                if (j_entity.count("render")) {
                    auto& j_render = j_entity["render"];
                    for (auto& render_item = j_render.begin(); render_item != j_render.end(); render_item++) {
                        if (render_item.value().count("mesh")) {
                            std::string s_mesh = render_item.value().value("mesh", "");
                            PreResource preres{ s_mesh, false };
                            if (s_mesh.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), preres) == scene_resources.end()) {
                                scene_resources.push_back(preres);
                            }
                        }
                        if (render_item.value().count("materials")) {
                            std::vector<std::string> j_materials = render_item.value()["materials"];
                            for (auto& material : j_materials) {
                                PreResource preres{ material, true };
                                if (material.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), preres) == scene_resources.end()) {
                                    scene_resources.push_back(preres);
                                }
                            }
                        }
                    }
                }
                //if (j_entity.count("skeleton")) {

                //}
            }
        }
    }
}