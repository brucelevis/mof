require "config";
require "base_define";


Scene = {	m_type=ObjDefine.t_scene;
			m_Id=0,
			m_object=nil,

			m_monsters={},
			m_players={},
			m_pets={},
			m_npcs={},

			m_objectMap={};

			m_monsterMap={},
			m_playerMap={},
			m_petMap={},
			m_npcMap={},

			-- 当前场景中的玩家个数
			m_player_number=0,		

			-- function new();
			-- function create();

			-- function cb_onInit();
			-- function cb_onUpdate();		// 场景刷新接口函数
			-- function cb_onDestroy();		// 场景释放接口函数

			-- function findCreature( objtype, objid );
			-- function addMonster( monster_TemplateId );
			-- function releaseMonster( monsterId );
			-- function addNpc();

		};

--这句是重定义元表的索引，就是说有了这句，这个才是一个类。具体的解释，请百度。
Scene.__index = Scene;

function Scene:new( instId )
    local self = {};  				--初始化self，如果没有这句，那么类所建立的对象改变，其他对象都会改变
    setmetatable(self, Scene);  	--将self的元表设定为Class
    self.m_Id = instId;

  	return self;  					--返回自身
end

function Scene:create( sceneObj )
	-- 调用C++核心模块创建场景
	-- self.m_object, self.m_Id = CCoreImpl_Scene.createScene( g_SceneManagerObject );
	
	-- 根据核心模块需求，场景对象已经创建好，直接传进来，这里不必再请求创建了
	self.m_object = sceneObj;
	if self.m_object == nil then
		return false;
	end	

    _debug("Scene:new() called, New Scene id is "..self.m_Id);

	-- 根据核心模块返回的自身Id在全局场景管理Map中注册自己
	g_sceneMap[self.m_Id] = self;

	-- 场景创建后给上层脚本一个机会，让它可以在里面添加怪物等
	self:func_onInit();
	return true;
end

-- 脚本内部调用它，会请求核心层将场景invalid掉，进而核心层回调脚本的script_inCall_destroyScene->clearAll方法
function Scene:close()
	CCoreImpl_Scene.closeScene(self.m_object);
end

-- 清除场景的所有分配资源,属于暴力清除，不给每个对象跟核心层交互的机会，这个方法一般在摧毁整个对象时调用（设置nil）
function Scene:clearAll()
	local key, object;

	for key,object in pairs(self.m_objectMap) do
		if object ~= nil then
			object.cb_onRelease();
		end
		self.m_objectMap[key] = nil;
	end

	for key,object in pairs(self.m_players) do
		self.m_players[key] = nil;
	end

	for key,object in pairs(self.m_monsters) do
		self.m_monsters[key] = nil;
	end

	for key,object in pairs(self.m_pets) do
		self.m_pets[key] = nil;
	end

	for key,object in pairs(self.m_npcs) do
		self.m_npcs[key] = nil;
	end

	for key,object in pairs(self.m_playerMap) do
		self.m_playerMap[key] = nil;
	end

	for key,object in pairs(self.m_monsterMap) do
		self.m_monsterMap[key] = nil;
	end

	for key,object in pairs(self.m_petMap) do
		self.m_petMap[key] = nil;
	end

	for key,object in pairs(self.m_npcMap) do
		self.m_npcMap[key] = nil;
	end
end

function Scene:findCreature( objtype, objid )
	-- 根据指定的对象类型和id返回对象
	if( objtype == define.t_player ) then
		for i=1,g_MaxScenePlayers,1 do
			if self.m_players[i] ~= nil and self.m_players[i].m_Id == objid then
				return self.m_players[i];
			end
		end
	elseif( objtype == define.t_monster ) then
		for i=1,g_MaxSceneMonsters,1 do
			if self.m_monsters[i] ~= nil and self.m_monsters[i].m_Id == objid then
				return self.m_monsters[i];
			end
		end
	else
		return nil;
	end
	return nil;
end


function Scene:addMonster( monster_modId, posx, posy, bNotifyClient )
	-- 根据怪物的模版id在场景内创建一个怪物

	for i=1,g_MaxSceneMonsters,1 do
		
		if self.m_monsters[i] == nil then
			
			newMonster = Monster:new( monster_modId );
			if newMonster:create( self, posx, posy, bNotifyClient ) == false then
				newMonster = nil;
				return nil;
			end	

			self.m_monsters[i] = newMonster;
			-- 根据核心模块返回的自身Id在场景的怪物管理Map中注册自己
			self.m_monsterMap[newMonster.m_Id] = newMonster;
			self.m_objectMap[newMonster.m_Id] = newMonster;
			
			self:func_onMonsterEnter( newMonster.m_Id, newMonster );
			return newMonster;
		end
	end

	return nil;
end

function Scene:kickPlayer( player )
	-- 踢掉玩家
	-- 首先通知核心层
	CCoreImpl_Scene.kickPlayer( player.m_object );
end

function Scene:releaseMonster( monsterId )
	local monster = self.m_monsterMap[monsterId]
	return self.releaseMonster( monster );
end

function Scene:releaseMonster( monster )
	if monster == nil then
		return false;
	end

	monster:cb_onRelease();
	-- 释放怪物这个都是脚本内发起，所以我们需要知会核心层释放，核心层无需再通知脚本
	-- 这个流程类似 CCoreImpl_Monster.invalidSelf( self.m_object )
	-- 两者在核心层处理方式也是一样的
	CCoreImpl_Scene.releaseMonster( self.m_object, monster.m_object );

	for i=1,g_MaxSceneMonsters,1 do
		
		if self.m_monsters[i] == monster then
			self.m_monsters[i] = nil;
			break;
		end
	end

	self.m_monsterMap[monster.m_Id] = nil;
	self.m_objectMap[monster.m_Id] = nil;
	return true;
end

function Scene:releasePlayer( player )
	if player == nil then
		return false;
	end

	player:cb_onRelease();
	-- 因为释放都是由核心层发起，所以这里我们不会再调用核心层释放流程
	-- CCoreImpl_Scene.releasePlayer( player.m_object );

	for i=1,g_MaxScenePlayers,1 do
		
		if self.m_players[i] == player then
			self.m_players[i] = nil;
			break;
		end
	end

	self.m_playerMap[player.m_Id] = nil;
	self.m_objectMap[player.m_Id] = nil;

	-- 本场景玩家数 －1
	self.m_player_number = self.m_player_number - 1;

	return true;
end

function Scene:releasePet( pet )
	if pet == nil then
		return false;
	end

	pet:cb_onRelease();
	-- 因为释放都是由核心层发起，所以这里我们不会再调用核心层释放流程
	-- CCoreImpl_Scene.releasePet( pet.m_object );


	for i=1,g_MaxScenePets,1 do
		
		if self.m_pets[i] == pet then
			self.m_pets[i] = nil;
			break;
		end
	end

	self.m_petMap[pet.m_Id] = nil;
	self.m_objectMap[pet.m_Id] = nil;

	return true;
end

function Scene:cb_onAddPlayer( roleInstId, playerObj )

	for i=1,g_MaxScenePlayers,1 do
		if self.m_players[i] == nil then
			
			-- 
			local newPlayer = Player:new(roleInstId);
			newPlayer:create( self, playerObj );

			if newPlayer.m_object ~= nil  then 
				self.m_players[i] = newPlayer;
				self.m_playerMap[roleInstId] = newPlayer;
				self.m_objectMap[roleInstId] = newPlayer;
				
				_debug("cb_onAddPlayer : "..roleInstId);
				-- 给上层脚本一个机会钩子，处理玩家进场逻辑
				self:func_onPlayerEnter( roleInstId, newPlayer );

				-- 本场景玩家数＋1
				self.m_player_number = self.m_player_number + 1;

				return true;
			end
			return false;
		end
	end
	return false;
end

function Scene:cb_onAddPet( instId, petObj, ownerRoleId )
	for i=1,g_MaxScenePets,1 do
		if self.m_pets[i] == nil then
			

			-- 
			local ownerPlayer = self.m_playerMap[ownerRoleId];

			-- 获取其主人玩家对象
			if ownerPlayer == nil then
				return false;
			end

			local newPet = Pet:new(instId, ownerPlayer);
			newPet:create( self, petObj );

			if newPet.m_object ~= nil  then 
				self.m_pets[i] = newPet;
				self.m_petMap[instId] = newPet;
				self.m_objectMap[instId] = newPet;

				-- 给上层脚本一个机会钩子，处理玩家进场逻辑
				self:func_onPetEnter( ownerPlayer, instId, newPet );

				return true;
			end
			return false;
		end
	end
	return false;
end

-- 有生物挂掉了，需要通知上层处理
function Scene:cb_onCreatureDead( creatureInstId, killerInstId )
	
	victim = self.m_objectMap[creatureInstId];
	killer = self.m_objectMap[killerInstId];
	
	if victim == nil then
		return;
	end

	local currentTime = CCoreImpl_Scene.getCurrentTime( self.m_object );

	_debug("Scene:cb_onCreatureDead --- 1");

	if victim.m_type == ObjDefine.t_monster then
		-- 设置怪物挂掉的标志
		victim.m_dead = true;
		victim.m_deadTime = currentTime;
		victim.m_deadPosx, victim.m_deadPosy = CCoreImpl_Scene.getPosition( self.m_object, victim.m_object );

	elseif victim.m_type == ObjDefine.t_pet then

		-- 设置怪物挂掉的标志
		victim.m_dead = true;
		victim.m_deadTime = currentTime;
		victim.m_deadPosx, victim.m_deadPosy = CCoreImpl_Scene.getPosition( self.m_object, victim.m_object );

	elseif victim.m_type == ObjDefine.t_player then

		-- 设置怪物挂掉的标志
		victim.m_dead = true;
		victim.m_deadTime = currentTime;
		victim.m_deadPosx, victim.m_deadPosy = CCoreImpl_Scene.getPosition( self.m_object, victim.m_object );
	end

	self:func_onCreatureDead(victim, killer);

	_debug("Scene:cb_onCreatureDead --- 2");

	-- 这里需要将核心层对象释放掉，因为核心层紧接着会将死去的怪物或者其他对象invalid，进一步释放掉，我们脚本里也不能再引用了
	-- victim.m_object = nil;
end

function Scene:cb_onPlayerLeave( playerInstId )

	player = self.m_objectMap[playerInstId]
	if player == nil then
		return;
	end

	-- 给一个机会给上层逻辑进行处理,上层无需释放任何东西，只做逻辑
	self:func_onPlayerLeave( player );

	-- 最后，我们需要将玩家release掉
	self:releasePlayer(player);

end

function Scene:cb_onEnemyTargetJudge( creatureInstId, targetInstId )
	creature = self.m_objectMap[creatureInstId];
	target = self.m_objectMap[targetInstId];

	return self:func_onEnemyTargetJudge( creature, target );
end

function Scene:cb_onInit()
end

function Scene:cb_onUpdate( millisecond )
	-- 刷新怪物，让它们动起来
	for key,object in pairs(self.m_monsters) do
	--for j=1,g_MaxSceneMonsters,1 do
		if object ~= nil then
			object:cb_onUpdate( millisecond );
		end
	end		
	-- 刷新宠物，让它们也动起来

	for key,object in pairs(self.m_pets) do
	--for j=1,g_MaxScenePets,1 do
		if object ~= nil then
			object:cb_onUpdate( millisecond );
		end
	end		
end

function Scene:cb_onDestroy()
	-- 释放所有怪物资源
	for j=1,g_MaxSceneMonsters,1 do
		if self.m_monsters[j] ~= nil then
			-- 场景释放怪物：需要注意，这里不是真正释放，而是场景脚本不再使用
			-- 真正的释放还是在C++核心代码中处理
			self.m_monsters[i]:cb_onRelease();
			self.m_monsters[i] = nil;
		end
	end
end


--[[
	上层逻辑接口，这部分接口函数都可以被子场景类重写，以便实现自己的逻辑
]]--


-- 场景初始化，这个函数可以重载，方便上层脚本添加自己的初始化逻辑，比如增加怪物，增加事件等
function Scene:func_onInit()
end

-- 上层接口，用来处理玩家对象进场的机会钩子
function Scene:func_canPlayerEnter( roleName, lvl, roleJob, vipLvl, honorId, guildName, guildPosition, weaponFasion, bodyFasion, maxHp, atk, def, hit, dodge, cri )
	return true;
end

-- 玩家进入
function Scene:func_onPlayerEnter( roleInstId, player )
end

-- 宠物进入
function Scene:func_onPetEnter( ownerPlayer, instId, newPet )
end

-- 怪物进入
function Scene:func_onMonsterEnter( monsterInstId, monster )
end

-- 每位玩家最多携带几只宠物进场
function Scene:func_getMaxPetCount()
	return 1;
end

-- 有生物挂掉了.这里可以提供一些上层逻辑处理,不需要返回值
function Scene:func_onCreatureDead( victim, killer )
end

-- 有玩家离开场景了,这里我们不需要处理任何清理工作，底层会去做
-- 我们只需要做一些游戏逻辑相关的操作，如果没有事情可以直接返回
function Scene:func_onPlayerLeave( player )
end

function Scene:func_onEnemyTargetJudge( creature, target )
	_debug("func_onEnemyTargetJudge : ---");
	return true;
end

function Scene:func_getPetCtrlMode()
	return PetCtrlMode.t_ctrlserver;
end

