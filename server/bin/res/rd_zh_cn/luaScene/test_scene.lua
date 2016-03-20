require "config"
require "base_scene";
require "base_player";
require "base_monster";

--声明了新的属性Z
TestScene = {m_var1=0};

--设置类型是Scene,继承自Scene
setmetatable(TestScene, Scene);

--还是和类定义一样，表索引设定为自身
TestScene.__index = TestScene;

--这里是构造体，加上了一个新的参数
function TestScene:new( sceneInstId, testNumber )
   local self;				  		--初始化对象自身
   
   self = Scene:new(sceneInstId); 	--将对象自身设定为父类，这个语句相当于其他语言的super
   
   setmetatable(self, TestScene); 	--将对象自身元表设定为Main类
   self.m_var1= testNumber; 		--新的属性初始化，如果没有将会按照声明=0
   
   _debug( "TestScene:new() called" );
   return self;
end

-- 场景初始化接口，在这里我们可以布置整个场景，由场景对象的 create 调用
function TestScene:func_onInit()
	-- 调用父类的方法,需要将self重新设置回父类，然后再调用
	setmetatable(self, Scene);
	self:func_onInit();

	-- 调用完毕，再设置回来
	setmetatable(self, TestScene);
	-- 自己的实现如下
	-- todo:

	local mon1, mon2, mon4, mon5, mon6, mon7;

	-- 在［200，300］位置增加一个 10001 号模版怪物,并设定它的行军路线
	mon1 = self:addMonster(123096, 700, 300, false);
	mon1.m_group = 1;
	mon1:addRoutinePoint(1,1500,400);
	mon1:addRoutinePoint(2,1500,50);
	mon1:addRoutinePoint(3,500,50);
	mon1:addRoutinePoint(4,500,400);


	-- 在［600，500］位置增加一个 10002 号模版的怪物,并设定它的行军路线
	mon2 = self:addMonster(132046, 1200, 600, false);
	mon2.m_group = 4;
	mon2:addRoutinePoint(0,500,400);
	mon2:addRoutinePoint(1,500,50);
	mon2:addRoutinePoint(2,1500,50);
	mon2:addRoutinePoint(3,1500,400);
	
	-- 在［600，500］位置增加一个 10002 号模版的怪物,并设定它的行军路线
	mon3 = self:addMonster(19432, 900, 600, false);
	mon3.m_group = 4;
	mon3:addRoutinePoint(0,500,400);
	mon3:addRoutinePoint(1,500,50);
	mon3:addRoutinePoint(2,1500,50);
	mon3:addRoutinePoint(3,1500,400);


	-- 在［600，500］位置增加一个 10002 号模版的怪物,并设定它的行军路线
	mon4 = self:addMonster(33104, 1200, 600, false);
	mon4.m_group = 4;
	mon4:addRoutinePoint(0,500,400);
	mon4:addRoutinePoint(1,500,50);
	mon4:addRoutinePoint(2,1500,50);
	mon4:addRoutinePoint(3,1500,400);


	-- 在［600，500］位置增加一个 10002 号模版的怪物,并设定它的行军路线
	mon5 = self:addMonster(19432, 900, 200, false);
	mon5.m_group = 4;
	mon5:addRoutinePoint(0,500,400);
	mon5:addRoutinePoint(1,500,50);
	mon5:addRoutinePoint(2,1500,50);
	mon5:addRoutinePoint(3,1500,400);


	-- 在［600，500］位置增加一个 10002 号模版的怪物,并设定它的行军路线
	mon6 = self:addMonster(33108, 900, 200, false);
	mon6.m_group = 4;
	mon6:addRoutinePoint(0,500,400);
	mon6:addRoutinePoint(1,500,50);
	mon6:addRoutinePoint(2,1500,50);
	mon6:addRoutinePoint(3,1500,400);


   _debug( "TestScene:func_onInit() called" );
	-- end
end

-- 是否允许玩家进入场景
-- 比如是否符合场地条件
-- 再如可以记录失败者名单，失败者不能再次进入本场地等等ß
function TestScene:func_canPlayerEnter( roleName, lvl, roleJob, vipLvl, honorId, guildName, guildPosition, weaponFasion, bodyFasion, maxHp, atk, def, hit, dodge, cri )
	_debug("TestScene:func_canPlayerEnter() called");
	return true;
end

-- 玩家进入场景时的定制处理
function TestScene:func_onPlayerEnter( roleInstId, player )
	-- 默认让初进入的玩家都出现在 100,100 位置
	_debug("TestScene:func_onPlayerEnter -- "..roleInstId);
	if player ~= nil then
		player:setPosition(840,220);
	end
end

-- 玩家宠物进入场景时的定制处理
function TestScene:func_onPetEnter( ownerPlayer, instId, newPet )
	-- 默认让初进入的宠物都出现在 100,100 位置
	if newPet ~= nil then
		_debug("TestScene:func_onPetEnter called");
		newPet:setPosition(880,260);
	end
end

-- 怪物进入场景时的定制处理
function TestScene:func_onMonsterEnter( monsterInstId, monster )

end

-- 决定每位玩家最多携带几只宠物进场，如果返回0则表示不允许携带宠物进场
function TestScene:func_getMaxPetCount()
	return 1;
end

-- 有生物挂掉时调用.这里可以提供逻辑处理,不需要返回值
function TestScene:func_onCreatureDead( victim, killer )
	if victim.m_type == ObjDefine.t_player then
		-- 是某玩家挂了
		-- todo:
	elseif victim.m_type == ObjDefine.t_pet then
		-- 是某宠物挂了
		-- todo:
	elseif victim.m_type == ObjDefine.t_monster then
		-- 是某怪物挂了
		-- todo:
	end
end

-- 有玩家离开场景了,这里我们不需要处理任何清理工作，底层会去做
-- 我们只需要做一些游戏逻辑相关的操作，如果没有事情可以直接返回
function TestScene:func_onPlayerLeave( player )
	if self.m_player_number <= 1 then
		-- 最后一个玩家正要离开，我们可以在这里关闭场景
		self:close();
	end
end

-- 敌人目标判断，这里可以实现敌我判断逻辑
function TestScene:func_onEnemyTargetJudge( creature, target )
	--_debug("TestScene:func_onEnemyTargetJudge");
	
	if (target == nil) or target:isDead() then
		return false;
	end

	if (creature == nil) or creature:isDead() then
		return false;
	end
	
	if creature.m_Id == target.m_Id then
		-- 自己不能打自己
		return false;
	end


	if creature.m_type == ObjDefine.t_pet then
		if target.m_type == ObjDefine.t_player then
			if creature.m_owner == target then
				return false;
			end
		elseif target.m_type == ObjDefine.t_pet then
			if creature.m_owner == target.m_owner then
				return false;
			end
		end
	elseif creature.m_type == ObjDefine.t_player then
		if target.m_type == ObjDefine.t_pet then
			if target.m_owner == creature then
				return false;
			end
		end
	end

	if creature.m_group ~= target.m_group then
		--_debug("creature.group="..creature.m_group.."   target.group="..target.m_group);
		return true;
	else 
		return false;
	end

	return true;
end

function TestScene:func_getPetCtrlMode()
	return PetCtrlMode.t_ctrlserver;
end

