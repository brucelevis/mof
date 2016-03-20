require "config"
require "base_define"
require "base_scene"

Routine = {{x=0,y=0},{x=1,y=1},};

Monster = {	m_type=ObjDefine.t_monster,
			m_Id=0,
			m_object=nil,

			m_Scene=nil,

			m_tplId= -1,
			m_resId= -1,

			m_target=nil,

			m_dead=false,

			-- 战斗阵营相关
			m_targetType=ObjDefine.t_monster or ObjDefine.t_player,
			m_targetHint=0,
			m_group=0,


			-- 预设的运动轨迹
			m_routine=nil,
			m_currentPosition=0,

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

function Monster:new( monster_templateId, monster_resId )
    local self = {};  				--初始化self，如果没有这句，那么类所建立的对象改变，其他对象都会改变
    setmetatable(self, Monster);  	--将self的元表设定为Class
    self.m_tplId = monster_templateId;
    self.m_resId = monster_resId;
  	return self;  					--返回自身
end

function Monster:create( scene, posx, posy, bNotify )
		-- 调用C++核心模块创建场景
	
	self.m_object, self.m_Id = CCoreImpl_Scene.createMonster( scene.m_object, self.m_tplId, self.m_resId, posx, posy, bNotify );
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
end

function Monster:relifeMonster( posx, posy )
	-- 怪物复活
	-- 由于逻辑较复杂，这里我们就通知核心层释放掉老object,然后创建一个新的
	CCoreImpl_Monster.invalidSelf( self.m_object );
	self.m_object = nil;
	
	if self.m_Scene ~= nil then
		-- 在现有对象上创建新核心怪物
		self.m_Scene:addMonster(self.m_tplId, self.m_resId, posx, posy);
	end
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
		self:func_OnUpdate_1( millisecond );
		-- self:func_OnUpdate_2();
		-- self:func_OnUpdate_3();
		-- self:func_OnUpdate_4();
		-- self:func_OnUpdate_5();
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

-- 帮助函数，能够实现一个默认的寻找（攻击，救援）目标的方法
function Monster:searchTarget()
	
	local targets = {};
	local targetNum = 0;
	local key, value;
	local player, monster, pet, target;

	local count = 0;

	-- 首先遍历场景内所有的玩家
	if self.m_targetType and TargetDefine.t_player then
		for key,player in pairs(self.m_Scene.m_players) do
			-- print(key, value);
			if player ~= nil then 
				if CCoreImpl_Monster.targetInSight( self.m_object, player.m_object) and player:isDead() == false) then
					targets[count] = player;
					count = count + 1;
				end
			end
		end
	end

	-- 然后判断所有怪物
	if self.m_targetType and TargetDefine.t_monster then
		for key,monster in pairs(self.m_Scene.m_monsters) do
			-- print(key, value);
			if monster ~= nil then 
				if CCoreImpl_Monster.targetInSight( self.m_object, monster.m_object) and monster:isDead() == false) then
					targets[count] = monster;
					count = count + 1;
				end
			end
		end
	end

	-- 判断所有宠物
	if self.m_targetType and TargetDefine.t_pet then
		for key,pet in pairs(self.m_Scene.m_pets) do
			-- print(key, value);
			if pet ~= nil then 
				if CCoreImpl_Monster.targetInSight( self.m_object, pet.m_object) and pet:isDead() == false)  then
					targets[count] = pet;
					count = count + 1;
				end
			end
		end
	end

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
		self.m_target = self:searchTarget();
	end
	
	if self.m_target ~= nil and CCoreImpl_Monster.targetInSight( self.m_object, self.m_target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( self.m_object );
		if( attackTimeSpan > 3 ) then
			-- 优先选择技能攻击
			if CCoreImpl_Monster.skillAttackTarget(self.m_object, self.m_target.m_object) == false then
				-- 技能攻击失败，换成普通攻击
				if CCoreImpl_Monster.normalAttackTarget(self.m_object, self.m_target.m_object) == false then
					CCoreImpl_Monster.patrolAround( self.m_object, self.m_target.m_object );
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
		CCoreImpl_Monster.pursuitTarget( self.m_object, self.m_target.m_object );
	else
		-- 没有目标，四处游弋
		CCoreImpl_Monster.roam( self.m_object );
	end
	return 0;
end


function Monster:func_OnUpdate_2( millisecond )
--
	-- 比如一个活动怪物（要求按照自己的固定轨迹运动，如果玩家在身边就攻击一下，但不追踪
	-- 
	if self.m_target == nil then
		-- 怪物没有攻击目标, 告知引擎我们需要的怪物类型，让它去帮我们搜索
		-- local targettype, targetid=CCoreImpl_Monster.searchTarget( self.m_Scene.m_object, self.m_object, self.m_targetType, self.m_targetHint );
		-- self.m_target = self.m_Scene:findCreature( targettype, targetid );
		self.m_target = self:searchTarget();
	end
	
	if self.m_target ~= nil and CCoreImpl_Monster.targetInSight( self.m_object, self.m_target.m_object ) then 
	 	
		attackTimeSpan = CCoreImpl_Monster.getLastHitTimeSpan( self.m_object );
	
		if( attackTimeSpan > 3 ) then
			CCoreImpl_Monster.normalAttackTarget( self.m_object, self.m_target.m_object );
			return 0;
			-- return 0;
		else
			-- 没有到时间攻击，不能让核心层驱动怪物在目标附近巡逻游弋
			-- CCoreImpl_Monster.patrol( self.m_object, self.m_target.m_object );
			-- return 0;
		end
	else
		-- 不追击目标
		-- CCoreImpl_Monster.purchTarget( self.m_object, self.m_target.m_object );
	end
	
	-- 怪物不管是否攻击，依旧按照路线行驶
	CCoreImpl_Monster.walkRoutine( self.m_object, self.m_routine[self.m_currentPosition].x,  self.m_routine[self.m_currentPosition].y);
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
