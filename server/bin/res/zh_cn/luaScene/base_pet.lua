require "config"
require "base_define"
require "base_scene"

--[[
	宠物模型定义
	class Pet{
		
		int m_Id;				// 宠物的临时id
		Scene m_Scene;			// 宠物所处场景对象
		Pet * m_object;			// 宠物ß对应的C++对象LivingObject
		Player m_owner;			// 宠物的主人对象

		int m_tplId;			// 宠物的资源模板

		// 人物正在（技能）攻击目标对象，根据不同返回值C++核心做不同处理ß
		function cb_onAttackingTarget( LivingObject * pTarget );
		function cb_onSkillingTarget( LivingObject * pTarget, int skillId );

		// 人物挂掉的通知
		function cb_onDead();

		// 人物被打中时的钩子调用，根据不同返回值C++核心做不同处理
		function cb_onBeAttacked( LivingObject * pAttacker )
		function cb_onBeSkilled( LivingObject * pAttacker, int skillid );

		function Release(this);
		function SceneStep(this);
	}
]]--

Pet = {	m_type=ObjDefine.t_pet,
		m_Id,
		m_object=nil,
		m_Scene=nil,

		m_owner=nil,

		m_dead=false,
		m_deadTime=0,
		m_deadPosx=0,		-- 死亡地点,时间
		m_deadPosy=0,

		m_tplId=0,

		-- 战斗阵营相关
		m_group=0,

		m_targetType=ObjDefine.t_player or ObjDefine.t_monster,

		m_target=nil,
		-- function new(scene);
		-- function create(monster_tplid, posx, posy);
			
		--// 宠物正在（技能）攻击目标对象，根据不同返回值C++核心做不同处理ß
		-- function cb_onAttackingTarget( LivingObject * pTarget );
		-- function cb_onSkillingTarget( LivingObject * pTarget, int skillId );

		-- // 宠物挂掉的通知
		-- function cb_onDead();

		-- // 宠物被打中时的钩子调用，根据不同返回值C++核心做不同处理
		-- function cb_onBeAttacked( LivingObject * pAttacker )
		-- function cb_onBeSkilled( LivingObject * pAttacker, int skillid );

		-- function Release(this);
	};

--这句是重定义元表的索引，就是说有了这句，这个才是一个类。具体的解释，请百度。
Pet.__index = Pet;

function Pet:new( instId, ownerPlayer )
    local self = {};  					--初始化self，如果没有这句，那么类所建立的对象改变，其他对象都会改变
    setmetatable(self, Pet); 	 		--将self的元表设定为Class
    self.m_owner = ownerPlayer;
    self.m_Id = instId;
  	return self;  						--返回自身
end

function Pet:create( scene, petObject )
	-- 调用C++核心模块创建场景
	-- self.m_object, self.m_Id = CCoreImpl_Scene.createPet( scene.m_object, self.m_roleId, posx, posy );
	self.m_object = petObject;
	self.m_Scene = scene;
	if self.m_object ~= nil  then
		self.m_owner:addPet( self );
		return true;
	end
	return false;
end

function Pet:release()
	self.m_Scene:releasePet(self);
end

function Pet:isDead()
	if self.m_dead == true then
		return true;
	end
	if self.m_object ~= nil then
		self.m_dead = CCoreImpl_Scene.creatureIsDead(self.m_Scene.m_object, self.m_object);
		return self.m_dead;
	end
	return false;
end

-- 帮助函数，能够实现一个默认的寻找（攻击，救援）目标的方法
function Pet:searchTarget()
	
	local targets = {};
	local targetNum = 0;
	local key, value;
	local player, monster, pet, target;

	local count = 0;

	-- 首先遍历场景内所有的玩家
	--if self.m_targetType and TargetDefine.t_player then
		for key,player in pairs(self.m_Scene.m_players) do
			-- print(key, value);
			if player ~= nil and (player:isDead() == false) then
				if self.m_Scene:func_onEnemyTargetJudge( self, player ) == true then 
					if CCoreImpl_Monster.targetInSight( self.m_object, player.m_object) and (player:isDead() == false) then
						targets[count] = player;
						count = count + 1;
					end
				end
			end
		end
	--end

	-- 然后判断所有怪物
	--if self.m_targetType and TargetDefine.t_monster then
		for key,monster in pairs(self.m_Scene.m_monsters) do
			-- print(key, value);
			if monster ~= nil and (monster:isDead() == false) then 
				if self.m_Scene:func_onEnemyTargetJudge( self, monster ) == true then 
					if CCoreImpl_Monster.targetInSight( self.m_object, monster.m_object) and (monster:isDead() == false) then
						targets[count] = monster;
						count = count + 1;
					end
				end
			end
		end
	--end

	-- 判断所有宠物
	--if self.m_targetType and TargetDefine.t_pet then
		for key,pet in pairs(self.m_Scene.m_pets) do
			-- print(key, value);
			if pet ~= nil and (pet:isDead() == false) then 
				if self.m_Scene:func_onEnemyTargetJudge( self, pet ) == true then 
					if CCoreImpl_Monster.targetInSight( self.m_object, pet.m_object) and (pet:isDead() == false)  then
						targets[count] = pet;
						count = count + 1;
					end
				end
			end
		end
	--end

	-- 最后我们寻找一个距离我们最近的 (将核心层的逻辑搬进来，看左右两边哪边人多，就找那边最近的)
	local leftMinDistance = 100000000;
    local rightMinDistance = 100000000;
    local leftTarget = nil;
    local rightTarget = nil;
    local leftCount = 0;
    local rightCount = 0;

    local self_posx, self_posy = CCoreImpl_Scene.getPosition( self.m_Scene.m_object, self.m_object );

	for key,target in pairs(targets) do
		repeat
        	if target.m_Id == self.m_Id then
        		-- 是自己：跳出repeat,直接判断下一个
            	break;
        	end

        	if target:isDead() or (target.m_object == nil) then
        		break;
        	end

	        local posx, posy = CCoreImpl_Scene.getPosition( self.m_Scene.m_object, target.m_object );
   	     	if posx > self_posx then
            
            	rightCount = rightCount + 1;
            	local dis = math.abs( posx - self_posx);

            	if rightMinDistance > dis then
                	rightTarget = target;
                	rightMinDistance = dis;
            	end

        	else
            	
            	leftCount = leftCount + 1;
            	local dis = math.abs( posx - self_posx );

            	if leftMinDistance > dis then
            	
                	leftTarget = target;
                	leftMinDistance = dis;
            	end
        	end
        until true;
	end

	if leftCount > rightCount then
	 	return leftTarget;
	else
	 	return rightTarget;
	end
end

-- 具体的怪物逻辑可以放到上层交给脚本程序员编写
function Pet:func_OnUpdate_1( millisecond )
	
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

	if self.m_Scene == nil then
		return 0;
	end

	-- 怪物脚本逻辑应该是属于简单逻辑,不应该处理复杂的状态和效果等具体事务
	-- 相当于只发指令，而不处理指令效果，所以相应的接口函数也需要类似客户端
	-- 消息那样对待；还有一层意思，就是C++核心模块不要加入任何怪物AI处理，
	-- 而是等待脚本调用相应的动作方法
	--

	-- 当场景中还没有玩家进入的时候，怪物们的ai不启动，直接返回
	if self.m_Scene.m_player_number <= 0 then 
		return 0;
	end

	if self.m_target ~= nil and self.m_target:isDead() then
		self.m_target = nil;
	end

	-- 比如一个普通怪物:
	if self.m_target == nil then
		-- 怪物没有攻击目标, 告知引擎我们需要的怪物类型，让它去帮我们搜索, C++返回的只是一个id 和 type
		-- local targettype, targetid=self:searchTarget( self.m_Scene.m_object, self.m_object, self.m_targetType, self.m_targetHint );
		-- self.m_target = self.m_Scene:findCreature( targettype, targetid );
		self.m_target = self:searchTarget();
	end
	
	if self.m_target ~= nil and CCoreImpl_Monster.targetInSight( self.m_object, self.m_target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( self.m_object );
		if( attackTimeSpan > 1+(math.random(1,100)%3) ) then
			-- 优先选择技能攻击
			if CCoreImpl_Monster.skillAttackTarget(self.m_object, self.m_target.m_object) == false then
				-- 技能攻击失败，换成普通攻击
				if CCoreImpl_Monster.normalAttackTarget(self.m_object, self.m_target.m_object) == false then
					if	math.random(1,100) % 100 < 60  then
						CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
					else
						self.m_target = nil;
					end						
				end
			end
			return 0;
		else
			-- 没有到时间攻击，让核心层驱动怪物在目标附近巡逻游弋
			--CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
			return 0;
		end
	elseif self.m_target ~= nil then
		-- 调用C++核心模块追击目标
		if	math.random(1,100) % 100 < 40  then
			CCoreImpl_Monster.pursuitTarget( self.m_object, self.m_target.m_object );
		else
			self.m_target = nil;			
		end
	else
		-- 没有目标，四处游弋
		CCoreImpl_Monster.roam( self.m_object );
	end
	return 0;
end

-- 场景层引擎释放宠物的时候调用
function Pet:cb_onRelease()
end

function Pet:cb_onHited()
end

function Pet:cb_onDead()
end

function Pet:cb_onUpdate()
end

function Pet:cb_onBeAttacted()
end

function Pet:cb_onBeSkilled()
end

function Pet:cb_onUpdate( millisecond )
	if self.m_dead == false then 
		self:func_OnUpdate_1( millisecond );
		-- self:func_OnUpdate_2();
		-- self:func_OnUpdate_3();
		-- self:func_OnUpdate_4();
		-- self:func_OnUpdate_5();
	end
end

--[[ 一些工具函数 ]]--

function Pet:setPosition( posx, posy )
	ret = CCoreImpl_Scene.setPosition( self.m_Scene.m_object, self.m_object, posx, posy );
end

function Pet:getPosition()
	return CCoreImpl_Scene.getPosition( self.m_Scene.m_object, self.m_object );
end

function Pet:getName()
	return CCoreImpl_Player.getName( self.m_owner.m_object );
end

function Pet:getOwner()
	return self.m_owner;
end

function Pet:getAtk()
	return CCoreImpl_Monster.getAtk();
end

function Pet:getHit()
	return CCoreImpl_Monster.getHit();
end

function Pet:getDodge()
	return CCoreImpl_Monster.getDodge();
end

function Pet:getMaxHp()
	return CCoreImpl_Monster.getMaxHp();
end

function Pet:getHp()
	return CCoreImpl_Monster.getHp();
end

function Pet:getCri()
	return CCoreImpl_Monster.getCri();
end
