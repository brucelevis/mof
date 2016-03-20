
--竞技场每10分钟发的积分返回， rank:玩家的排名
function pvpMod_pvpRankSettlement(rank)
	if rank > 100 then
		return 1
	else
		return 2 * (100 - rank + 1)
	end

	return 0
end