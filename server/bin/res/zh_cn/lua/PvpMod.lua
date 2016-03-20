
--竞技场每10分钟发的积分返回， rank:玩家的排名
function pvpMod_pvpRankSettlement(rank)
	if rank < 5 then
		return (1000 - rank * 50)
	elseif rank < 10 then
		return (880 - rank * 30)
	elseif rank < 100 then
		return (580 - rank * 2)
	elseif rank < 300 then
		return (380 - rank)
	elseif rank < 800 then
		return 70
	elseif rank < 1500 then
		return 60
	elseif rank < 2500 then
		return 50
	elseif rank < 3500 then
		return 40
	elseif rank < 4500 then
		return 30
	else 
		return 20
	end
end