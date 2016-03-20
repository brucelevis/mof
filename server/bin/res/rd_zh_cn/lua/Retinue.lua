

--根据星级，等级计算属性
function retinue_getLvlProp(lvl, star, modatk, moddef, modhp, modhit, moddodge, modcri)

	local atk = 0
	local def = 0
	local hp = 0

	local hit = 0
	local dodge = 0
	local cri = 0

	local b1 = 1
	local b2 = 1
	local b3 = 1
	local b4 = 1
	local b5 = 1
	local b6 = 1
	--写公式

	-----         等级属性		   +  星级属性
	atk   = modatk   * (1 + ((lvl - 1) / 10)) * (1 + star / 10)

	def   = moddef   * (1 + ((lvl - 1) / 10)) * (1 + star / 10)

	hp    = modhp    * (1 + ((lvl - 1)/ 10)) * (1 + star / 10)

	hit   = modhit   * (1 + ((lvl - 1)/ 10)) * (1 + star / 10)

	dodge = moddodge * (1 + ((lvl - 1)/ 10)) * (1 + star / 10)

	cri   = modcri   * (1 + ((lvl - 1)/ 10)) * (1 + star / 10)

	return atk, def, hp, hit, dodge, cri
end

--升级所需经验
function retinue_getLvlUpExp(lvl, modExp)

	local needExp = 0

	local n = 10;
	local m = 100;
	--计算所需经验
	needExp = modExp*(lvl / 2 + 1);

	return needExp
end

--当前等级总经验
function retinue_getTotalExp(lvl, modExp)

	local totalExp = 0

	--计算总经验
	for i=1,lvl do
		totalExp = totalExp + retinue_getLvlUpExp(i, modExp);
	end

	return totalExp
end

--获取升星所需经验
function retinue_getStarUpExp(lvl, modExp)

	local needExp = 0

	--计算所需经验

	local needExp = 0

	local n = 10;
	local m = 100;

	--计算所需经验
	needExp = modExp*lvl;
	return needExp
end

--一次性获取所需信息，减少lua调用
function retinue_getRetinueInfo( lvl, star, modexp, modstarExp, modatk, moddef, modhp, modhit, moddodge, modcri )
	-- body

	local nextLvlExp = retinue_getLvlUpExp(lvl + 1, modexp)
	local nextStarExp = retinue_getStarUpExp(star + 1, modstarExp)

	local atk, def, hp, hit, dodge, cri = retinue_getLvlProp(lvl, star, modatk, moddef, modhp, modhit, moddodge, modcri)

	return atk, def, hp, hit, dodge, cri, nextLvlExp, nextStarExp

end
