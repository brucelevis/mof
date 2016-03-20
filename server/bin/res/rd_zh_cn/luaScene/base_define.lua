--[[
	场景内物件的类型定义：
	
	跟核心层的对象定义保持一致
	这样就可以直接从核心层传递参数相对应了

    eWorldObjectScene = 1,
    eWorldObjectRole = 2,
    eWorldObjectPet = 3,
    eWorldObjectNPC = 4,
    eWorldObjectRobot = 5,
    eWorldObjectTimer = 6,
    eWorldObjectSkillAction = 7,
    eWorldObjectSkillFly = 8
]]--

ObjDefine = {
	t_scene = 1,
	t_player = 2,
	t_pet = 3,
	t_monster = 4,
	t_robot = 5,
};

SceneDefine = {
	t_turnMatch = 1,
	t_activity = 2,
	t_timecontrol = 3,
};

TargetDefine = {
	t_player = 1,
	t_monster = 2,
	t_pet = 4,
};

PetCtrlMode = {
	t_ctrlclient=1,
	t_ctrlserver = 2,
	t_ctrlclientserver = 3,
};


