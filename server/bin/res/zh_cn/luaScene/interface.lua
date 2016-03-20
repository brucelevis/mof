-- 将当前目录加入到 require 搜索路径中去
--package.path = string.format( '%s;%s', (arg[0]:gsub('[^\\/]+%.lua','?.lua')), package.path );
package.path = string.format( '%s;%s', CCoreConfig.require_path, package.path );

require "config";
require "base_scene";
require "base_monster";
require "base_player";
require "base_pet";
require "test_scene";


function script_inCall_onLoad( SceneManagerObject )
	g_SceneManagerObject = SceneManagerObject;
end 

function script_inCall_newScene( sceneInstId, sceneObj )
	-- 场景初始化, 将脚本函数变成对象的内部成员，调用的时候可以更加面向对象化
	
	-- 初始化随机数种子
	math.randomseed(os.time());

	for i=1,g_MaxScene,1 do
		if g_scenes[i] == nil then
			g_scenes[i] = TestScene:new( sceneInstId, 1976 );
			
			if g_scenes[i] == nil then
				return false;
			end

			if g_scenes[i]:create( sceneObj ) == true then
				return true;
			end

			return false;
		end
	end	
	return false;
end

-- 每位玩家最多携带几只宠物进场
function script_inCall_getMaxPetCount( scendId  )
	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return 0;
	end

	return scene:func_getMaxPetCount();
end

-- 主模块调用查看脚本是否允许增加玩家进场
function script_inCall_canPlayerEnter( sceneId, roleName, lvl, roleJob, vipLvl, honorId, guildName, guildPosition, weaponFasion, bodyFasion, maxHp, atk, def, hit, dodge, cri )

	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return false;
	end

	-- 以下几项可以放在一个接口中获取，加快速度
	-- local player_level = CCoreImpl_Player.getLevel( roleObj );
	-- local player_sex = CCoreImpl_Player.getSex( roleObj );
	-- local player_roleid = CCoreImpl_Player.getRoleId( roleObj );
	-- local player_guild = CCoreImpl_Player.getGuild( roleObj );

	-- 根据脚本要求判断是否要加入该玩家
	bCan = scene:func_canPlayerEnter( roleName, lvl, roleJob, vipLvl, honorId, guildName, guildPosition, weaponFasion, bodyFasion, maxHp, atk, def, hit, dodge, cri );
	return bCan;

end


function script_inCall_addPlayer( sceneId, roleInstId, playerObj )

	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return false;
	end

	return scene:cb_onAddPlayer( roleInstId, playerObj );
end

function script_inCall_addPet( sceneId, petInstId, petObj, ownerRoleInstId )

	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return false;
	end

	return scene:cb_onAddPet( petInstId, petObj, ownerRoleInstId );
end

function script_inCall_updateScene( sceneId, millisecond )
	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return;
	end

	scene:cb_onUpdate( millisecond );
end

function script_inCall_destroyScene( sceneId )

	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return false;
	end

	-- 清除场景中的所有资源ß
	scene:clearAll();

	g_sceneMap[sceneId] = nil;

	for i=1,g_MaxScene,1 do
		if g_scenes[i] == scene then
			_debug("scene is released !!");
			g_scenes[i] = nil;
		end
	end

	return true;
end

-- creature 被 killer 杀死了，这里通知到脚本
function script_inCall_onCreatureDead( sceneId, creatureInstId, killerInstId )
	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return false;
	end

	return scene:cb_onCreatureDead( creatureInstId, killerInstId );
end

function script_inCall_onPlayerLeave( sceneId, playerInstId )
	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return false;
	end

	return scene:cb_onPlayerLeave( playerInstId );	
end


function script_inCall_isEnemyTarget( sceneId, originInstId, targetInstId )
	local scene = g_sceneMap[sceneId];
	if scene == nil then
		return false;
	end

	return scene:cb_onEnemyTargetJudge( originInstId, targetInstId );	
end

function script_inCall_getPetCtrlMode( sceneId )
	local scene = g_sceneMap[sceneId];
	if scene ~= nil then
		return scene:func_getPetCtrlMode();
	end
	return PetCtrlMode.t_ctrlclientserver;
end

