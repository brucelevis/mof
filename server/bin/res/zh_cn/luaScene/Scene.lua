
--local g_Scene;

g_SceneManagerObject;

-- 定义一个怪物数组，怪物

-- {
--	{
		-- hp;
		-- isDead;
		-- attack;
		-- defense;
--	},
--	{}};




-- 本模块能够管理的最多场景对象
g_scenes={};
g_sceneMap={};
g_MaxScene = 128;
g_currentSceneCount = 0;

g_MaxScenePlayers = 32;
g_MaxSceneMonsters = 128;

t_scene = 0;
t_player = 1;
t_monster = 2;


--[[
	场景模型定义
	class Scene{
		
		int m_Id;				// 场景id
		CScene * m_object;		// 场景对应的C++对象
		
		Monster m_monsters[];	// 场景中的所有怪物
		Player m_players[];		// 场景中的所有玩家
		Npc m_npcs[];			// 场景中的所有npc
		
		Monster m_monsterMap[];	// 怪物的id映射表
		Player m_playerMap[];	// 玩家的id映射表
		Npc m_npcMap[];			// npc的id映射表

		function inCall_OnUpdate(this);	// 场景刷新接口函数
		function inCall_OnDestroy(this);	// 场景释放接口函数

		function method_addMonster(this, monTplId);
		function method_findCreature(type, id);
	}
]]--
-- Scene::update函数

function script_inCall_onLoad( SceneManagerObject )
	g_SceneManagerObject = SceneManagerObject;
end 

function scene_inCall_NewScene()
	-- 场景初始化, 将脚本函数变成对象的内部成员，调用的时候可以更加面向对象化
	for i=1,g_MaxScene,1 do
		if g_scenes[i] == nil then
			g_scenes[i] = {};
			g_scenes[i].m_object, g_scenes[i].m_Id = CCoreImpl_Scene.createScene( g_SceneManagerObject );
			if( g_scenes[i] == nil )
				return 0;
			
			g_sceneMap[g_scenes[i].m_Id] = g_scenes[i];

			g_scenes[i].addMonster = scene_method_addMonster;
			g_scenes[i].findCreature = scene_method_findCreature;
			g_scenes[i].m_monsters = {};
			g_scenes[i].m_monsterMap = {};
			g_scenes[i].m_players = {};
			g_scenes[i].m_playerMap = {};
			g_scenes[i].m_npcs={};
			g_scenes[i].m_npcMap={};
			return 1;
		end
	end	
end

function scene_inCall_AddPlayer( sceneId, roleObj )

	local scene = g_sceneMap[sceneId];
	if( scene == nil )
		return 0;

	-- 以下几项可以放在一个接口中获取，加快速度
	local player_level = CCoreImpl_Player.getLevel( roleObj );
	local player_sex = CCoreImpl_Player.getSex( roleObj );
	local player_roleid = CCoreImpl_Player.getRoleId( roleObj );
	local player_guild = CCoreImpl_Player.getGuild( roleObj );

	-- 根据脚本要求判断是否要加入该玩家
	if scene:canPlayerEnter( player_level, player_sex, player_roleid, player_guild ) == true  then 
		-- 如果能够加入，则：
		for i=1,g_MaxScenePlayers,1 do
			if scene.m_players[i] == nil then
				scene.m_players[i] = {};
				scene.m_players[i].m_object, scene.m_players[i].m_Id = CCoreImpl_Scene.createPlayer( scene.m_object, roleObj );
				return 1;
			end
		end
	end
	return 0;
end

function scene_inCall_OnUpdate( sceneId )
	
	local scene = g_sceneMap[sceneId];
	if( scene == nil )
		return 0;

	print("++"..scene.m_Id);

	for j=0,g_MaxSceneMonsters,1 do
		if scene.m_monsters[j] ~= nil then
			--monster_action(scene.m_monsters[j]);
			-- 怪物做点场景相关的周期性的事情
			scene.m_monsters[i]:cb_OnUpdate  monster_method_DoSceneStep( scene.m_monsters[j] );
		end
	end
end

function scene_inCall_OnDestroy( sceneId )
	local scene = g_sceneMap[sceneId];
	if( scene == nil )
		return 0;

	for j=0,g_MaxSceneMonsters,1 do
		if scene.m_monsters[j] ~= nil then
			-- 场景释放怪物：需要注意，这里不是真正释放，而是场景脚本不再使用
			-- 真正的释放还是在C++核心代码中处理
			monster_method_DoRelease(scene.m_monsters[j]);
			scene.m_monsters[i] = nil;
		end
	end
end


--[[
	脚本内调用方法
]]--

function scene_method_addMonster( scene, monster_TemplateId )
	for i=1,g_MaxSceneMonsters,1 do
		if scene.m_monsters[i] == nil then
			scene.m_monsters[i] = {};
			-- C++ 模块创建怪物后，返回怪物的句柄（指针容器），同时返回的还有怪物的Id
			scene.m_monsters[i].m_object, scene.m_monsters[i].m_Id = CCoreImpl_Scene.createMonster( scene.m_object, monster_TemplateId );
			scene.m_monsters[i].m_Scene = scene;

			-- 建立一个 id <--> monster 的映射
			scene.m_monsterMap[scene.m_monsters[i].m_Id] = scene_monsters[i].
			break;
		end
	end
end

function scene_method_findCreature( scene, objtype, objid )
	if( objtype == t_player ) then
		for i=1,g_MaxScenePlayers,1 do
			if scene.m_players[i] ~= nil and scene.m_players[i].m_Id == objid then
				return scene.m_players[i];
			end
		end
	else if( objtype == t_monster ) then
		for i=1,g_MaxSceneMonsters,1 do
			if scene.m_monsters[i] ~= nil and scene.m_monsters[i].m_Id == objid then
				return scene.m_monsters[i];
			end
		end
	else
		return nil;
	end
	return nil;
end





--[[
	怪物模型定义
	class Monster{
		
		int m_Id;				// 怪物id
		Scene m_Scene;			// 怪物所处场景对象
		Monster * m_object;		// 怪物对应的C++对象

		int m_status;

		function cb_OnUpdate(float dt);
		function cb_onDead( LivingObject * pKiller );
		function method_DoRelease(this);
		function method_DoSceneStep(this);
	}
]]--

function monster_cb_OnUpdate_1( sceneId, monsterId )
	
	-- 怪物AI逻辑
	-- 由C++模块中的 update 心跳调用

	-- 如果返回 0 就表示需要继续执行C++模块中的怪物 update AI
	-- 如果返回 1 就表示不需要继续执行C++模块中的怪物 update AI

  	-- 玩法设计：
	-- 比如追杀第一个进入场景的怪物
	-- 比如判断哪个玩家头顶有宝藏标志，就追杀哪个玩家
	-- 比如
	--switch(m_status){
	--
	--}

	local scene = g_sceneMap[sceneId];
	if( scene == nil )
		return 0;

	local monster = scene.m_monsterMap[monsterId];
	if( monster == nil )
		return 0;

	-- 怪物脚本逻辑应该是属于简单逻辑,不应该处理复杂的状态和效果等具体事务
	-- 相当于只发指令，而不处理指令效果，所以相应的接口函数也需要类似客户端
	-- 消息那样对待；还有一层意思，就是C++核心模块不要加入任何怪物AI处理，
	-- 而是等待脚本调用相应的动作方法
	--

	-- 当场景中还没有玩家进入的时候，怪物们的ai不启动，直接返回
	if monster.scene.m_player_number <= 0 then 
		return 0;
	end

	-- 比如一个普通怪物:
	if monster.target == nil then
		-- 怪物没有攻击目标, 告知引擎我们需要的怪物类型，让它去帮我们搜索, C++返回的只是一个id 和 type
		targettype, targetid=CCoreImpl_Scene.searchTarget( monster.scene.m_object, monster.m_object, monster.enemyType, monster.targetHint );
		monster.target = monster.scene.findCreature( monster.scene, targettype, targetid );
	end
	
	if monster.target ~= nil and CCoreImpl_Monster.targetInSight( monster.m_object, monster.target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( monster.m_object );
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget(monter.target);
			return 0;
		else
			-- 没有到时间攻击，让核心层驱动怪物在目标附近巡逻游弋
			CCoreImpl_Monster.patrolAround( monster.m_object, monster.target.m_object );
			return 0;
		end
	else
		-- 调用C++核心模块追击目标
		CCoreImpl_Monster.pursuitTarget( monster.m_object, monster.target.m_object );
	end
	return 0;
end

function monster_cb_OnUpdate_2( monster )
--
	-- 比如一个活动怪物（要求按照自己的固定轨迹运动，如果玩家在身边就攻击一下，但不追踪
	-- 
	if monster.target == nil then
		-- 怪物没有攻击目标, 告知引擎我们需要的怪物类型，让它去帮我们搜索
		targettype, targetid=CCoreImpl_Monster.searchTarget( monster.scene.m_object, monster.enemyType );
		monster.target = monster.scene.findCreature( monster.scene, targettype, targetid );
	end
	
	if monster.target ~= nil and CCoreImpl_Monster.targetInSight( monster.m_object, monster.target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( monster.m_object );
	
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget( monster.m_object, monter.target.m_object );
			return 0;
			-- return 0;
		else
			-- 没有到时间攻击，不能让核心层驱动怪物在目标附近巡逻游弋
			-- CCoreImpl_Monster.patrol( monster.m_object, monster.target.m_object );
			-- return 0;
		end
	else
		-- 不追击目标
		-- monster.purchTarget( monster.target );
	end
	
	-- 怪物不管是否攻击，依旧按照路线行驶
	CCoreImpl_Monster.WalkRoutine( monster.m_object, monster.routine[monster.currentPoint].x,  monster.routine[monster.currentPoint].y);
	return 0;
end

function monster_cb_OnUpdate_3( monster )
--
	-- 再如一个活动怪物（要追击头顶锦标的VIP玩家）
	-- 
	if monster.target == nil then
		-- 怪物没有攻击目标, 我们不将target决定权交给核心层，我们自行处理
		-- 寻找具有10550这个vip腰牌物品的玩家
		for i=1,g_MaxSceneMonsters,1 do
			if monster.scene.m_players[i] ~= nil and CCoreImpl_Player.hasItem( monster.scene.m_players[i].m_object, 10550 ) then
				monster.target = monster.scene.m_players[i];
				break;
			end 
		end
		-- monster.target = CCoreImpl_Monster.searchTarget( monster.enemyType );
	else if monster.target ~= nil and CCoreImpl_Player.hasItem( monster.target, 10550 ) == false then
		-- 目标玩家已经没有这件物品，可能转移给了别人，放弃它，重新寻找
		for i=1,g_MaxSceneMonsters,1 do
			if monster.scene.m_players[i] ~= nil and CCoreImpl_Player.hasItem( monster.scene.m_players[i].m_object, 10550 ) then
				monster.target = monster.scene.m_players[i];
				break;
			end 
		end
	end
	
	if monster.target ~= nil and CCoreImpl_Monster.targetInSight( monster.m_object, monster.target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( monster.m_object );
	
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget(monter.target);
			return 0;
		else
			CCoreImpl_Monster.patrolAround( monster.m_object, monster.target.m_object );
			return 0;
		end
	else
		-- 调用C++核心模块追击目标
		CCoreImpl_Monster.pursuitTarget( monster.m_object, monster.target.m_object );
	end
	return 0;
end

function monster_cb_OnUpdate_4( monster )
--
	-- 设计一种活动怪物（它们是分成两派互相对战的，挂掉的还会自动再次刷出来）
	-- 
	if monster.target == nil then
		-- 怪物没有攻击目标, 我们不将target决定权交给核心层，我们自行处理
		-- 寻找跟自己分组不一样的其他怪物
		for i=1,g_MaxSceneMonsters,1 do
			if monster.scene.m_monsters[i] ~= nil and monster.target.group ~= monster.group then
				monster.target = monster.scene.m_monsters[i];
				break;
			end 
		end
	end
	
	if monster.target ~= nil and CCoreImpl_Monster.targetInSight( monster, monster.target ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( monster.m_object );
	
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget(monter.target);
			return 0;
		else
			CCoreImpl_Monster.patrolAround( monster.m_object, monster.target.m_object );
			return 0;
		end
	else
		-- 调用C++核心模块追击目标
		CCoreImpl_Monster.purchTarget( monster.m_object, monster.target.m_object );
	end
	return 0;
end


function monster_cb_OnUpdate_5( monster )
--
	-- 设计一种活动怪物（它们是从场景的一个地方出发，到另一个地方去采矿的，采完矿会回到出发地，过一段时间会再次重复）
	-- 
	if monster.target == nil then
		-- 怪物没有攻击目标, 我们不将target决定权交给核心层，我们自行处理
		-- 寻找跟自己分组不一样的其他怪物
		for i=1,g_MaxSceneMonsters,1 do
			if monster.scene.m_monsters[i] ~= nil and monster.target.group ~= monster.group then
				monster.target = monster.scene.m_monsters[i];
				break;
			end 
		end
	end
	
	if monster.target ~= nil and CCoreImpl_Monster.targetInSight( monster.target ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( monster.m_object );
	
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget(monter.target);
			return 0;
		else
			CCoreImpl_Monster.patrolAround( monster.m_object, monster.target.m_object );
			return 0;
		end
	else
		-- 追击目标
		monster.purchTarget( monster.target );
	end
	return 0;
end



function monster_cb_onDead( monster )
	-- 怪物挂掉的事件
	return 0;
end

function monster_method_DoRelease( monster )
	return 0;
end

function monster_method_DoSceneStep( monster )
	return 0;
end






--[[
	下面是场景逻辑控制代码
]]--
function NewScene()
	for i=1,g_MaxScene,1 do
		if g_scenes[i] == nil then
			g_scenes[i] = {};
			-- C++ 核心层调用返回两个值，一个是对象句柄容器，一个是场景Id
			g_scenes[i].m_object, sceneId = CCoreImpl_Scene.createScene(sceneId);
			g_scenes[i].m_Id = sceneId;
			g_scenes[i].m_monsters = {};
			g_scenes[i].m_players = {};
			
			break;
		end
	end
end

function test( scene )
	scene_cb_OnUpdate( scene );
end

function OnInit( ... )
	-- body

	-- 场景初始化
	--[[
	for i=1,g_MaxScene,1 do
		g_scenes[i].m_Id = 0;
		g_scenes[i].update = update;
		for j=1,g_MaxSceneMonsters,1 do
			g_scenes[i].m_monsters = {{},{},{},{},{},{},{},{}};
			g_scenes[i].m_monsters[j] = nil;
		end
	end
	]]--

end

function OnUpdate( ... )
	-- body

	for i=1,g_MaxScene,1 do
		if g_scenes[i] ~= nil and g_scenes[i].m_Id ~= -1 then
			-- ...
			-- 这里我们并不是要去主动调用各个场景的update,这些cb_OnUpdate方法应该交给
			-- C++ 核心调度去做，我们这里只留这么个接口以备不时之需。
			-- scene_update(g_scenes[i]);
			test(g_scenes[i]);
		end
	end
end


print("hello".."world");

OnInit();

NewScene(109);
NewScene(102);
OnUpdate();




--[[
	人物模型定义
	class Player{
		
		int m_Id;				// 人物的临时id
		Scene m_Scene;			// 人物所处场景对象
		Monster * m_object;		// 人物ß对应的C++对象LivingObject

		int m_status;

		// 人物正在（技能）攻击目标对象，根据不同返回值C++核心做不同处理ß
		function cb_onIsHitTarget( LivingObject * pTarget );
		function cb_onIsSkillTarget( LivingObject * pTarget, int skillId );

		// 人物挂掉的通知
		function cb_onDead();

		// 人物被打中时的钩子调用，根据不同返回值C++核心做不同处理
		function cb_onBeAttacked( LivingObject * pAttacker )
		function cb_onBeSkill( LivingObject * pAttacker, int skillid );

		function method_DoRelease(this);
		function method_DoSceneStep(this);
	}
]]--


