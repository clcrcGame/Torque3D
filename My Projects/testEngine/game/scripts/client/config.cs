// Torque Input Map File
if (isObject(moveMap)) moveMap.delete();
new ActionMap(moveMap);
moveMap.bindCmd(keyboard, "escape", "", "handleEscape();");
moveMap.bind(keyboard, "f2", showPlayerList);
moveMap.bind(keyboard, "ctrl h", hideHUDs);
moveMap.bind(keyboard, "alt p", doScreenShotHudless);
moveMap.bind(keyboard, "a", moveleft);
moveMap.bind(keyboard, "d", moveright);
moveMap.bind(keyboard, "left", moveleft);
moveMap.bind(keyboard, "right", moveright);
moveMap.bind(keyboard, "w", moveForward);
moveMap.bind(keyboard, "s", movebackward);
moveMap.bind(keyboard, "up", moveForward);
moveMap.bind(keyboard, "down", movebackward);
moveMap.bind(keyboard, "e", moveup);
moveMap.bind(keyboard, "c", movedown);
moveMap.bind(keyboard, "space", jump);
moveMap.bind(keyboard, "lcontrol", doCrouch);
moveMap.bind(keyboard, "lshift", doSprint);
moveMap.bind(keyboard, "f", setZoomFOV);
moveMap.bind(keyboard, "z", toggleZoom);
moveMap.bind(keyboard, "v", toggleFreeLook);
moveMap.bind(keyboard, "tab", toggleFirstPerson);
moveMap.bind(keyboard, "alt c", toggleCamera);
moveMap.bind(keyboard, "alt p", showPerimeterHUD);
moveMap.bindCmd(keyboard, "ctrl k", "commandToServer(\'suicide\');", "");
moveMap.bindCmd(keyboard, "1", "commandToServer(\'use\',\"Ryder\");", "");
moveMap.bindCmd(keyboard, "2", "commandToServer(\'use\',\"Lurker\");", "");
moveMap.bindCmd(keyboard, "3", "commandToServer(\'use\',\"LurkerGrenadeLauncher\");", "");
moveMap.bindCmd(keyboard, "4", "commandToServer(\'use\',\"ProxMine\");", "");
moveMap.bindCmd(keyboard, "5", "commandToServer(\'use\',\"DeployableTurret\");", "");
moveMap.bindCmd(keyboard, "r", "commandToServer(\'reloadWeapon\');", "");
moveMap.bind(keyboard, "0", unmountWeapon);
moveMap.bind(keyboard, "alt w", throwWeapon);
moveMap.bind(keyboard, "alt a", tossAmmo);
moveMap.bind(keyboard, "q", nextWeapon);
moveMap.bind(keyboard, "ctrl q", prevWeapon);
moveMap.bind(keyboard, "u", toggleMessageHud);
moveMap.bind(keyboard, "pageup", pageMessageHudUp);
moveMap.bind(keyboard, "pagedown", pageMessageHudDown);
moveMap.bind(keyboard, "p", resizeMessageHud);
moveMap.bind(keyboard, "f3", startRecordingDemo);
moveMap.bind(keyboard, "f4", stopRecordingDemo);
moveMap.bind(keyboard, "f8", dropCameraAtPlayer);
moveMap.bind(keyboard, "f7", dropPlayerAtCamera);
moveMap.bindCmd(keyboard, "n", "toggleNetGraph();", "");
moveMap.bindCmd(keyboard, "ctrl z", "carjack();", "");
moveMap.bind(keyboard, "h", turnLeft);
moveMap.bind(keyboard, "j", turnRight);
moveMap.bind(keyboard, "k", panUp);
moveMap.bind(keyboard, "l", panDown);
moveMap.bind(keyboard, ";", celebrationWave);
moveMap.bind(keyboard, "apostrophe", celebrationSalute);
moveMap.bind(mouse0, "xaxis", yaw);
moveMap.bind(mouse0, "yaxis", pitch);
moveMap.bind(mouse0, "button0", mouseFire);
moveMap.bind(mouse0, "button1", mouseButtonZoom);
moveMap.bind(mouse0, "zaxis", mouseWheelWeaponCycle);
