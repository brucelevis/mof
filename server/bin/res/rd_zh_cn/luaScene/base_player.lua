require "config"
require "base_define"
require "base_scene"

--[[
	人物模型定义
	class Player{
		
		int m_Id;				// 人物的临时id
		Scene m_Scene;			// 人物所处场景对象
		Monster * m_object;		// 人物ß对应的C++对象LivingObject

		int m_roleId;
		int m_status;

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

Player = {	m_type=ObjDefine.t_player,
			m_Id=0,
			m_object=nil,

			m_Scene=nil,

			m_dead=false,
			m_deadTime=0,
			m_deadPosx=0,		-- 死亡地点，时间
			m_deadPosy=0,

			-- m_roleId=-1,

			-- 战斗阵营相关
			m_group=0,

			-- 玩家宠物列表
			m_pets={},
			m_petNum=0,

			-- function new(scene);
			-- function create(monster_tplid, posx, posy);
			
			--// 人物正在（技能）攻击目标对象，根据不同返回值C++核心做不同处理ß
			-- function cb_onAttackingTarget( LivingObject * pTarget );
			-- function cb_onSkillingTarget( LivingObject * pTarget, int skillId );

			-- // 人物挂掉的通知
			-- function cb_onDead();

			-- // 人物被打中时的钩子调用，根据不同返回值C++核心做不同处理
			-- function cb_onBeAttacked( LivingObject * pAttacker )
			-- function cb_onBeSkilled( LivingObject * pAttacker, int skillid );

			-- function Release(this);
		};

--这句是重定义元表的索引，就是说有了这句，这个才是一个类。具体的解释，请百度。
Player.__index = Player;

function Player:new( roleInstId )
    local self = {};  				--初始化self，如果没有这句，那么类所建立的对象改变，其他对象都会改变
    setmetatable(self, Player);  	--将self的元表设定为Class
  	self.m_Id = roleInstId;
  	return self;  					--返回自身
end

function Player:create( scene, playerObj )
	-- 调用C++核心模块创建场景
	-- self.m_object, self.m_Id = CCoreImpl_Scene.createPlayer( scene.m_object, self.m_roleId, posx, posy );
	self.m_object = playerObj;
	self.m_Scene = scene;
	if self.m_object ~= nil  then
		return true;
	end
	return false;
end

function Player:isDead()
	if self.m_dead == true then
		return true;
	end
	if self.m_object ~= nil then
		self.m_dead = CCoreImpl_Scene.creatureIsDead(self.m_Scene.m_object, self.m_object);
		return self.m_dead;
	end
	return false;
end

function Player:addPet(pet)
	self.m_pets[self.m_petNum+1] = pet;
	self.m_petNum = self.m_petNum + 1;
end

function Player:setPosition( posx, posy )
	ret = CCoreImpl_Scene.setPosition( self.m_Scene.m_object, self.m_object, posx, posy );
end

function Player:getPosition()
	return CCoreImpl_Scene.getPosition( self.m_Scene.m_object, self.m_object );
end

function Player:release()
	-- 通知场景层释放自己
	self.m_Scene.releasePlayer(self);
	-- 这里的释放是脚本发起的，所以还需要通知核心层释放
end

-- 会被场景引擎调用, 用于释放资源
function Player:cb_onRelease()
	-- 释放所有宠物
	for i=1,self.m_petNum,1 do
		if self.m_pets[i] ~= nil then
			self.m_Scene:releasePet( self.m_pets[i] );
		end
		self.m_pets[i] = nil;
	end
	self.m_petNum = 0;
end

function Player:cb_onHited()
end

function Player:cb_onDead()
end

function Player:cb_onUpdate()
end

function Player:cb_onBeAttacted()
end

function Player:cb_onBeSkilled()
end


--[[ 一些工具函数 ]]--

function Player:getName()
	return CCoreImpl_Player.getName( self.m_object );
end

function Player:getSex()
	return CCoreImpl_Player.getSex( self.m_object );
end

function Player:getJob()
	return CCoreImpl_Player.getJob( self.m_object );
end

function Player:getVipLvl()
	return CCoreImpl_Player.getVipLvl( self.m_object );
end

function Player:getGuildName()
	return CCoreImpl_Player.getGuildName( self.m_object );
end

function Player:getGuildPosition()
	return CCoreImpl_Player.getGuildPosition( self.m_object );
end

function Player:getAtk()
	return CCoreImpl_Player.getAtk();
end

function Player:getDef()
	return CCoreImpl_Player.getDef();
end

function Player:getHit()
	return CCoreImpl_Player.getHit();
end

function Player:getDodge()
	return CCoreImpl_Player.getDodge();
end

function Player:getMaxHp()
	return CCoreImpl_Player.getMaxHp();
end

function Player:getHp()
	return CCoreImpl_Player.getHp();
end

function Player:getCri()
	return CCoreImpl_Player.getCri();
end




