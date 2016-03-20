--排名奖励的个人贡献值
function treasurefight_getGuildManorAward(rank)
	local award = ""
	local constrib = 0
	local exploit = 0
	if rank == 1 then
		award = "constrib 4000*1;"
		
	elseif rank == 2 then
		award = "constrib 3000*1;"
		
		elseif rank==3 then
		award = "constrib 2000*1;"
		elseif rank==4 then
		award = "constrib 1500*1;"
		elseif rank==5 then
		award = "constrib 1000*1;"
		else
		award = "constrib 500*1;"
		
	end
	

	return award
end


--排名奖励的公会财富和建设度
function treasurefight_getGuildWealthAward(rank)

local fortune = 0
local construction = 0
if rank == 1 then
		fortune=50000
		construction=10000
		
	elseif rank == 2 then
		fortune=40000
		construction=8000
		
		elseif rank==3 then
		
		fortune=30000
		construction=6000
		
		elseif rank==4 then
		fortune=25000
		construction=5000
		
		elseif rank==5 then
		fortune=20000
		construction=4000
		
		else
		fortune=5000
		construction=1000
		
	end


return fortune,construction

end




-- 个人积分奖励

function treasurefight_getPlayerAward(points,lvl)

	local award = ""
	local constrib = 1*points
	local exploit = 0.05*points
	if constrib >30*lvl^1.3 then
		constrib=30*lvl^1.3
	end
    if exploit >30*0.05*lvl^1.3 then
	    exploit=30*0.05*lvl^1.3
	end

	constrib = math.floor(constrib)
	exploit=math.floor(exploit)
	
	award = "constrib "..tostring(constrib).."*1;".."exploit "..tostring(exploit).."*1"
	return award
end

function treasurefight_getManorCityName(weekday)

	local cityname = "";

	if weekday == 0 then
		cityname = "维托里"
	elseif weekday == 1 then
		cityname = "风车村"
	elseif weekday == 3 then
		cityname = "莱布瑞尔"
	elseif weekday == 5 then
		cityname = "塔格拉"
	elseif weekday == 6 then
		cityname = "亚门洲"
	else 
		cityname = "没有配置"
	end

	return cityname;
end

-- 返还被杀的基础积分
function treasurefight_produceScoreByRole(lvl)
    return lvl^1.3;
end

-- 返还杀人的等级衰减，victimlvl被杀者等级，killerlvl杀人者的等级
--被杀者等级与杀人者等级相差5级以内，随着被杀者等级提高每级增加5%奖励
--被杀者等级大于杀人者5级以上，随着被杀者等级提高每级减少10%奖励
--被杀者等级小于杀人者5级以下，随着被杀者等级降低每级减少15%奖励
--奖励衰减最小值为5%
function treasurefight_getKillRoleReduce(victimlvl, killerlvl)
	local rate=0
	if (killerlvl-victimlvl)<=5 and (killerlvl-victimlvl)>=-5 then
	   rate=1-(killerlvl-victimlvl)*0.05
	    elseif killerlvl-victimlvl>5 then
	       rate=1.5-(killerlvl-victimlvl)*0.15
		   elseif killerlvl-victimlvl<5  then
		     rate=1.75+(killerlvl-victimlvl)*0.1
	     
	end
	
	if rate<0.05 then
		rate=0.05
		
	end
     return rate
end

