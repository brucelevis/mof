require "config"
require "base_define"
require "base_scene"

Routine = {{x=0,y=0},{x=1,y=1},};

Monster = {	m_type=ObjDefine.t_monster,
			m_Id=0,
			m_object=nil,

			m_Scene=nil,

			m_modId= -1,

			m_target=nil,
			m_targets={},		-- 所有的可攻击目标,对提升怪物进攻策略有帮助

			m_dead=false,
			m_deadTime=0,		-- 死亡时间
			m_deadPosx=0,		-- 死亡地点
			m_deadPosy=0,

			-- 战斗阵营相关
			m_targetType=ObjDefine.t_monster or ObjDefine.t_player,
			m_targetHint=0,
			m_group=0,


			-- 预设的运动轨迹
			m_routine=nil,
			m_currentPosition=0,
			m_routine_length=0,

			-- function new(scene);
			-- function create(monster_tplid, posx, posy);
			
			-- function release();

			-- function cb_onHited();
			-- function cb_onDead();
			-- function cb_onUpdate();
			-- function cb_onBeAttacted();
			-- function cb_onBeSkilled();
		};

--这句是重定义元表的索引，就是说有了这句，这个才是一个类。具体的解释，请百度。
Monster.__index = Monster;

function Monster:new( monster_modId )
    local self = {};  				--初始化self，如果没有这句，那么类所建立的对象改变，其他对象都会改变
    setmetatable(self, Monster);  	--将self的元表设定为Class
    self.m_modId = monster_modId;
  	return self;  					--返回自身
end

function Monster:create( scene, posx, posy, bNotifyClient )
	-- 调用C++核心模块创建场景
	
	-- bNotifyClient = true 表示要向当前场景动态增加一个怪物，核心层需要通知客户端（用在游戏中间加入），而 ＝ false 表示只创建，核心层不需要即时通知
	-- 客户端（一般用在场景刚创建时加入，通知工作由场景引擎集中做） 
	self.m_object, self.m_Id = CCoreImpl_Scene.createMonster( scene.m_object, self.m_modId, posx, posy, bNotifyClient );
	self.m_Scene = scene;

	if self.m_object == nil or self.m_Id < 0 then
		return false;
	end
	return true;
end

-- 设置怪物的预设行进路径点
function Monster:addRoutinePoint( index, posx, posy )
	if self.m_routine == nil then
		self.m_routine = {};
	end

	if self.m_routine[index] == nil then
		self.m_routine[index] = {};
	end

	self.m_routine[index].x = posx;
	self.m_routine[index].y = posy;

	if index >= self.m_routine_length then
		self.m_routine_length = index+1;
	end

end

function Monster:relife( posx, posy )
	-- 怪物复活
	-- 由于逻辑较复杂，这里我们就通知核心层释放掉老object,然后创建一个新的
	if self.m_dead == true then
		--根据目前的核心层逻辑和 onCreatureDead 逻辑，核心层已经在怪物死亡时释放了对象，无需这里处理
		--if self.m_object ~= nil then
		--	CCoreImpl_Monster.invalidSelf( self.m_object );
		self.m_object = nil;

		self.m_Scene.m_monsterMap[self.m_Id] = nil;
		self.m_Scene.m_objectMap[self.m_Id] = nil;

		-- 通知核心层重新创建新怪，加入到当前对象中	
		if self:create( self.m_Scene, posx, posy, true ) == false then
			return false;
		end	

		self.m_Scene.m_monsterMap[self.m_Id] = self;
		self.m_Scene.m_objectMap[self.m_Id] = self;

		_debug("Monster:relife");

		self.m_dead = false;
		return true;
	end
	return false;
end

function Monster:release()
	self.m_Scene:releaseMonster(self);
end

-- 判断怪物是否处于战斗空闲状态，如果是，则可以顺利执行ai，不然就等待
function Monster:inIdleState()
	return CCoreImpl_Monster.inIdleState( self.m_object );
end

-- 释放对象时，被Scene引擎调用
function Monster:cb_onRelease()
end

function Monster:cb_onHited()
end

function Monster:cb_onDead()
end

function Monster:cb_onUpdate()
end

function Monster:cb_onBeAttacted()
end

function Monster:cb_onBeSkilled()
end

function Monster:cb_onUpdate( millisecond )
	if self.m_dead == false then 
		-- self:func_OnUpdate_1( millisecond );
		self:func_OnUpdate_2();
		-- self:func_OnUpdate_3();
		-- self:func_OnUpdate_4();
		-- self:func_OnUpdate_5();
	else
		self:func_OnUpdate_dead( millisecond );
	end
end


function Monster:setPosition( posx, posy )
	ret = CCoreImpl_Scene.setPosition( self.m_Scene.m_object, self.m_object, posx, posy );
end

function Monster:getPosition()
	return CCoreImpl_Scene.getPosition( self.m_Scene.m_object, self.m_object );
end

function Monster:getAtk()
	return CCoreImpl_Monster.getAtk();
end

function Monster:getHit()
	return CCoreImpl_Monster.getHit();
end

function Monster:getDodge()
	return CCoreImpl_Monster.getDodge();
end

function Monster:getMaxHp()
	return CCoreImpl_Monster.getMaxHp();
end

function Monster:getHp()
	return CCoreImpl_Monster.getHp();
end

function Monster:getCri()
	return CCoreImpl_Monster.getCri();
end

function Monster:isDead()
	if (self.m_dead == true) or (self.m_object == nil) then
		return true;
	end

	if self.m_object ~= nil then
		self.m_dead = CCoreImpl_Scene.creatureIsDead(self.m_Scene.m_object, self.m_object);
		return self.m_dead;
	end
	return false;
end

-- 帮助函数，能够实现一个默认的寻找（攻击，救援）目标的方法
function Monster:searchTarget()
	
	-- local targets = {};
	local targetNum = 0;
	local key, value;
	local player, monster, pet, target;

	local count = 0;

	-- 先清空整张表
	self.m_targets = {};

	-- 首先遍历场景内所有的玩家
	--if self.m_targetType and TargetDefine.t_player then
		for key,player in pairs(self.m_Scene.m_players) do
			-- print(key, value);
			if player ~= nil and (player:isDead() == false) then
				if self.m_Scene:func_onEnemyTargetJudge( self, player ) == true then 
					if CCoreImpl_Monster.targetInSight( self.m_object, player.m_object) and (player:isDead() == false) then
						self.m_targets[count] = player;
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
						self.m_targets[count] = monster;
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
						self.m_targets[count] = pet;
						count = count + 1;
					end
				end
			end
		end
	--end
end

function Monster:getNearestTarget()

	-- 最后我们寻找一个距离我们最近的 (将核心层的逻辑搬进来，看左右两边哪边人多，就找那边最近的)
	local leftMinDistance = 100000000;
    local rightMinDistance = 100000000;
    local leftTarget = nil;
    local rightTarget = nil;
    local leftCount = 0;
    local rightCount = 0;

    local key, value;

    if self:isDead() or (self.m_object == nil) then
    	return nil;
    end

    local self_posx, self_posy = CCoreImpl_Scene.getPosition( self.m_Scene.m_object, self.m_object );

	for key,target in pairs(self.m_targets) do
		repeat
        	if target.m_Id == self.m_Id then
        		-- 是自己：跳出repeat,直接判断下一个
            	break;
        	end

        	if target:isDead() or (target.m_object == nil) then
        		-- 已挂
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

--[[
-- 获取场地内所有的可攻击目标，返回目标组
function Monster:searchTarget()
	
	local targets = {};
	local targetNum = 0;
	local key, value;
	local player, monster, pet, target;

	local count = 0;

	-- 首先遍历场景内所有的玩家
	--if self.m_targetType and TargetDefine.t_player then
		for key,player in pairs(self.m_Scene.m_players) do
			-- print(key, value);
			if player ~= nil then
				if self.m_Scene:func_onEnemyTargetJudge( self, player ) == true then 
					if CCoreImpl_Monster.targetInSight( self.m_object, player.m_object) and (CCoreImpl_Scene.creatureIsDead(self.m_Scene.m_object, player.m_object) == false) then
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
			if monster ~= nil then 
				if self.m_Scene:func_onEnemyTargetJudge( self, monster ) == true then 
					if CCoreImpl_Monster.targetInSight( self.m_object, monster.m_object) and (CCoreImpl_Scene.creatureIsDead(self.m_Scene.m_object, monster.m_object) == false) then
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
			if pet ~= nil then 
				if self.m_Scene:func_onEnemyTargetJudge( self, pet ) == true then 
					if CCoreImpl_Monster.targetInSight( self.m_object, pet.m_object) and (CCoreImpl_Scene.creatureIsDead(self.m_Scene.m_object, pet.m_object) == false)  then
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

]]--

-- 具体的怪物逻辑可以放到上层交给脚本程序员编写
function Monster:func_OnUpdate_1( millisecond )
	
	
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
		self:searchTarget();
		self.m_target = self:getNearestTarget();
	end
	
	if self.m_target ~= nil and CCoreImpl_Monster.targetInSight( self.m_object, self.m_target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( self.m_object );
		if( attackTimeSpan > 1+(math.random(1,100)%3) ) then

			local skillAttack = math.random(1,100);
			-- 优先选择技能攻击
			if skillAttack<35 or CCoreImpl_Monster.skillAttackTarget(self.m_object, self.m_target.m_object) == false then

				if skillAttack >= 35 then
					for key,target in pairs(self.m_targets) do
						if target.m_object ~= nil then
							if self.m_Scene:func_onEnemyTargetJudge(self, target) then
								if CCoreImpl_Monster.skillAttackTarget(self.m_object, target.m_object) == true then
									self.m_target = target;
									if target.m_type == ObjDefine.t_monster or target.m_type == ObjDefine.t_pet then
										target.m_target = self;
									end
									return 0;
								end
							end
						end
					end
				end

				-- 技能攻击失败，换成普通攻击
				if CCoreImpl_Monster.normalAttackTarget(self.m_object, self.m_target.m_object) == false then

					for key,target in pairs(self.m_targets) do
						if target.m_object ~= nil then
							if self.m_Scene:func_onEnemyTargetJudge(self, target) then
								if CCoreImpl_Monster.normalAttackTarget(self.m_object, target.m_object) == true then
									self.m_target = target;
									if target.m_type == ObjDefine.t_monster or target.m_type == ObjDefine.t_pet then
										target.m_target = self;
									end
									return 0;
								end
							else
								if target.m_type == ObjDefine.t_monster or target.m_type == ObjDefine.t_pet then
									target.m_target = self;
								end
							end
						end
					end

					if	math.random(1,100) % 100 < 60  then
						CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
					else
						self.m_target = nil;
					end						
				else
					if self.m_target.m_type == ObjDefine.t_monster or self.m_target.m_type == ObjDefine.t_pet then
						self.m_target.m_target = self;
					end
				end
			else
				if self.m_target.m_type == ObjDefine.t_monster or self.m_target.m_type == ObjDefine.t_pet then
					self.m_target.m_target = self;
				end
			end
			return 0;
		else
			-- 没有到时间攻击，让核心层驱动怪物在目标附近巡逻游弋
			CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
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

-- 死亡状态下的遍历，这种状态一定要注意，m_object 已经无效或者为nil, 不要使用它请求核心层
function Monster:func_OnUpdate_dead( millisecond )

	if self.m_dead == false then 
		return;
	end

	local currentTime = CCoreImpl_Scene.getCurrentTime( self.m_Scene.m_object );
	if (currentTime - self.m_deadTime) > 30 then
		-- 30秒后怪物复活
		self:relife(self.m_deadPosx, self.m_deadPosy);
	end
end

function Monster:walkRoutine()
	if self.m_routine_length <= 0 then
		return;
	end

	--_debug("self.m_currentPosition = "..self.m_currentPosition);

	if self.m_routine[self.m_currentPosition] == nil or CCoreImpl_Scene.atPosition(self.m_Scene.m_object, self.m_object, self.m_routine[self.m_currentPosition].x, self.m_routine[self.m_currentPosition].y) ~= 0 then
		-- 已经运动到目的地或者目的地不合适到达，可以去向下一站

		self.m_currentPosition = self.m_currentPosition + 1;
		if self.m_currentPosition >= self.m_routine_length then
			self.m_currentPosition = 0;
		end

		if self.m_routine[self.m_currentPosition] ~= nil then
			CCoreImpl_Monster.walkRoutine(self.m_object, self.m_routine[self.m_currentPosition].x, self.m_routine[self.m_currentPosition].y );
			return;
		end
	end

	if self.m_routine[self.m_currentPosition] ~= nil then
		--_debug("m_routine[m_currentPosition] = ["..self.m_routine[self.m_currentPosition].x..","..self.m_routine[self.m_currentPosition].y.."]");
		CCoreImpl_Monster.walkRoutine(self.m_object, self.m_routine[self.m_currentPosition].x, self.m_routine[self.m_currentPosition].y );
		return;
	end		
end

function Monster:func_OnUpdate_2( millisecond )
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
		self:searchTarget();
		self.m_target = self:getNearestTarget();
	end
	
	if self.m_target ~= nil and CCoreImpl_Monster.targetInSight( self.m_object, self.m_target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( self.m_object );
		if( attackTimeSpan > 1+(math.random(1,100)%3) ) then

			local skillAttack = math.random(1,100);
			-- 优先选择技能攻击
			if skillAttack<35 or CCoreImpl_Monster.skillAttackTarget(self.m_object, self.m_target.m_object) == false then

				if skillAttack >= 35 then
					for key,target in pairs(self.m_targets) do
						if target.m_object ~= nil then
							if self.m_Scene:func_onEnemyTargetJudge(self, target) then
								if CCoreImpl_Monster.skillAttackTarget(self.m_object, target.m_object) == true then
									self.m_target = target;
									if target.m_type == ObjDefine.t_monster or target.m_type == ObjDefine.t_pet then
										target.m_target = self;
									end
									return 0;
								end
							end
						end
					end
				end

				-- 技能攻击失败，换成普通攻击
				if CCoreImpl_Monster.normalAttackTarget(self.m_object, self.m_target.m_object) == false then

					for key,target in pairs(self.m_targets) do
						if target.m_object ~= nil then
							if self.m_Scene:func_onEnemyTargetJudge(self, target) then
								if CCoreImpl_Monster.normalAttackTarget(self.m_object, target.m_object) == true then
									self.m_target = target;
									if target.m_type == ObjDefine.t_monster or target.m_type == ObjDefine.t_pet then
										target.m_target = self;
									end
									return 0;
								end
							else
								if target.m_type == ObjDefine.t_monster or target.m_type == ObjDefine.t_pet then
									target.m_target = self;
								end
							end
						end
					end

					if	math.random(1,100) % 100 < 60  then
						--CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
						self:walkRoutine();
					else
						self.m_target = nil;
					end						
				else
					if self.m_target.m_type == ObjDefine.t_monster or self.m_target.m_type == ObjDefine.t_pet then
						self.m_target.m_target = self;
					end
				end
			else
				if self.m_target.m_type == ObjDefine.t_monster or self.m_target.m_type == ObjDefine.t_pet then
					self.m_target.m_target = self;
				end
			end
			return 0;
		else
			-- 没有到时间攻击，让核心层驱动怪物在目标附近巡逻游弋
			--CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
			self:walkRoutine();
			return 0;
		end
	elseif self.m_target ~= nil then
		-- 调用C++核心模块追击目标
		if	math.random(1,100) % 100 < 40  then
			self:walkRoutine();
			--CCoreImpl_Monster.pursuitTarget( self.m_object, self.m_target.m_object );
		else
			self.m_target = nil;			
		end
	else
		-- 没有目标，四处游弋
		self:walkRoutine();
		--CCoreImpl_Monster.roam( self.m_object );
	end
	return 0;

end


function Monster:func_OnUpdate_3( millisecond )
--
	-- 再如一个活动怪物（要追击头顶锦标的VIP玩家）
	-- 
	if self.m_target == nil then
		-- 怪物没有攻击目标, 我们不将target决定权交给核心层，我们自行处理
		-- 寻找具有10550这个vip腰牌物品的玩家
		for i=1,g_MaxScenePlayers,1 do
			if self.m_Scene.m_players[i] ~= nil and CCoreImpl_Player.hasItem( self.m_Scene.m_players[i].m_object, 10550 ) then
				self.m_target = self.m_Scene.m_players[i];
				break;
			end 
		end
		-- monster.target = self.searchTarget();
	elseif self.m_target ~= nil and CCoreImpl_Player.hasItem( self.m_target, 10550 ) == false then
		-- 目标玩家已经没有这件物品，可能转移给了别人，放弃它，重新寻找
		for i=1,g_MaxScenePlayers,1 do
			if self.m_Scene.m_players[i] ~= nil and CCoreImpl_Player.hasItem( self.m_Scene.m_players[i].m_object, 10550 ) then
				self.m_target = self.m_Scene.m_players[i];
				break;
			end 
		end
	end
	
	if self.m_target ~= nil and CCoreImpl_Monster.targetInSight( self.m_object, self.m_target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( self.m_object );
	
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget( self.m_object, self.m_target.m_object );
			return 0;
		else
			CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
			return 0;
		end
	elseif self.m_target ~= nil then
		-- 调用C++核心模块追击目标
		CCoreImpl_Monster.pursuitTarget( self.m_object, self.m_target.m_object );
	end
	return 0;
end


function Monster:func_OnUpdate_4( millisecond )
--
	-- 设计一种活动怪物（它们是分成两派互相对战的，挂掉的还会自动再次刷出来）
	-- 
	if self.inIdleState() == false then
		return 0;
	end

	if self.m_target == nil then
		-- 怪物没有攻击目标, 我们不将target决定权交给核心层，我们自行处理
		-- 寻找跟自己分组不一样的其他怪物
		for i=1,g_MaxSceneMonsters,1 do
			if self.m_Scene.m_monsters[i] ~= nil and self.m_target.m_group ~= self.m_group then
				self.m_target = self.m_Scene.m_monsters[i];
				break;
			end 
		end
	end
	
	if self.m_target ~= nil and CCoreImpl_Monster.targetInSight( self.m_object, self.target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( self.m_object );
	
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget(sel.m_object, self.m_target.m_object);
			return 0;
		else
			CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
			return 0;
		end
	elseif self.m_target ~= nil then
		-- 调用C++核心模块追击目标
		CCoreImpl_Monster.purchTarget( self.m_object, self.m_target.m_object );
	end
	return 0;
end


function Monster:func_OnUpdate_5( millisecond )
--
	-- 设计一种活动怪物（它们是从场景的一个地方出发，到另一个地方去采矿的，采完矿会回到出发地，过一段时间会再次重复）
	-- 
	if self.m_target == nil then
		-- 怪物没有攻击目标, 我们不将target决定权交给核心层，我们自行处理
		-- 寻找跟自己分组不一样的其他怪物
		for i=1,g_MaxSceneMonsters,1 do
			if self.m_Scene.m_monsters[i] ~= nil and self.m_target.m_group ~= self.m_group then
				self.m_target = self.m_Scene.m_monsters[i];
				break;
			end 
		end
	end
	
	if self.m_target ~= nil and CCoreImpl_Monster.targetInSight( self.m_object, self.target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( self.m_object );
	
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget(self.m_object, self.target.m_object);
			return 0;
		else
			CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
			return 0;
		end
	elseif self.m_target ~= nil then
		-- 追击目标
		CCoreImpl_Monster.purchTarget( self.m_object, self.m_target.m_object );
	end
	return 0;
end
