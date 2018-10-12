function onSceneStart_scene_coliseo()
	toButton(toEntity(getEntityByName("COL_mesh_terminal02")):getCompByName("button")):setCanBePressed(true);
	toButton(toEntity(getEntityByName("COL_mesh_terminal01")):getCompByName("button")):setCanBePressed(false);
end

function onScenePartialStart_scene_coliseo()
	onSceneStart_scene_coliseo();
	movePlayerToRefPos("col_intro_suelo001", i_ref_pos);
end

function onSceneStart_scene_coliseo_2()
	toButton(toEntity(getEntityByName("COL_mesh_terminal02")):getCompByName("button")):setCanBePressed(false);
	toButton(toEntity(getEntityByName("COL_mesh_terminal01")):getCompByName("button")):setCanBePressed(true);
end

function onScenePartialStart_scene_coliseo_2()
	onSceneStart_scene_coliseo_2();
	movePlayerToRefPos("col_zone_a_suelo002", i_ref_pos);
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"col_zone_a_framedoor001\")):getCompByName(\"door\")):open();", 0.5);
end

function onScenePartialEnd_scene_coliseo()
	i_ref_pos = getPlayerLocalCoordinatesInReferenceTo("col_zone_a_suelo002");
end

function onScenePartialEnd_scene_coliseo_2()
	i_ref_pos = getPlayerLocalCoordinatesInReferenceTo("col_bc_suelo003");
end

function onTriggerEnter_COL_trigger_opendoor_intro_player()
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"col_intro_framedoor\")):getCompByName(\"door\")):open()", 0.5);
	getEntityByName("COL_trigger_opendoor_intro"):destroy();
end

function onTriggerEnter_COL_trigger_opendoor_zonea_player()
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"col_zone_a_framedoor001\")):getCompByName(\"door\")):open()", 0.5);
	getEntityByName("COL_trigger_opendoor_zonea"):destroy();
end

function onTriggerEnter_COL_trigger_corridor_intro_player()
	closeIntroDoor();
end

function onTriggerEnter_COL_trigger_corridor_intro01_player()
	closeIntroDoor();
end

function onTriggerEnter_COL_trigger_corridor_intro02_player()
	closeIntroDoor();
end

function closeIntroDoor()
	toDoor(toEntity(getEntityByName("col_intro_framedoor")):getCompByName("door")):close();
	getEntityByName("COL_trigger_corridor_intro"):destroy();
	getEntityByName("COL_trigger_corridor_intro01"):destroy();
	getEntityByName("COL_trigger_corridor_intro02"):destroy();
end

function onTriggerEnter_COL_trigger_corridor_zonea_player()
	closeZoneADoor();
end

function onTriggerEnter_COL_trigger_corridor_zonea01_player()
	closeZoneADoor();
end

function onTriggerEnter_COL_trigger_corridor_zonea02_player()
	closeZoneADoor();
end

function closeZoneADoor()
	toDoor(toEntity(getEntityByName("col_zone_a_framedoor001")):getCompByName("door")):close();
	getEntityByName("COL_trigger_corridor_zonea"):destroy();
	getEntityByName("COL_trigger_corridor_zonea01"):destroy();
	getEntityByName("COL_trigger_corridor_zonea02"):destroy();
end

function transition_coliseum_to_zone_a(button_handle)
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
	--makeVisibleByTag("corridor", true);
	toDoor(toEntity(getEntityByName("col_zone_a_framedoor001")):getCompByName("door")):open();
end

function transition_coliseum_to_courtyard(button_handle)
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
	--makeVisibleByTag("corridor", true);
	toDoor(toEntity(getEntityByName("col_bc_framedoor002")):getCompByName("door")):open();
end

function onTriggerEnter_COL_trigger_corridor_zone_a_player()
	getEntityByName("COL_trigger_corridor_zone_a"):destroy();
	tdoor = toDoor(toEntity(getEntityByName("col_zone_a_framedoor001")):getCompByName("door"));
	tdoor:setClosedScript("destroyColPreloadZoneA()");
	tdoor:close();
end

function destroyColPreloadZoneA()
	destroyPartialScene();
	execScriptDelayed("preloadScene(\"scene_zone_a\")", 0.1);
end

function onTriggerEnter_COL_trigger_corridor_bc_player()
	getEntityByName("COL_trigger_corridor_bc"):destroy();
	tdoor = toDoor(toEntity(getEntityByName("col_bc_framedoor002")):getCompByName("door"));
	tdoor:setClosedScript("destroyColPreloadBC()");
	tdoor:close();
end

function destroyColPreloadBC()
	destroyPartialScene();
	execScriptDelayed("preloadScene(\"scene_basilic_courtyard\")", 0.1);
end