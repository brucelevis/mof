--跨服pvp胜利获得的积分
function winCsPvpScore(selfRank, targetRank)
	return 50
end

--跨服pvp失败获得的积分
function failCsPvpScore(selfRank, targetRank)
	return 10
end

--跨服pvp结算获得的积分
function finishCsPvpScore(targetRank)
	if targetRank < 5 then
		return (2100 - targetRank * 100)
	elseif targetRank < 10 then
		return (1750 - targetRank * 50)
	elseif targetRank < 100 then
		return (1250 - targetRank * 5)
	elseif targetRank < 200 then
		return (695 - targetRank * 2)
	elseif targetRank < 400 then
		return (495 - targetRank)
	elseif targetRank < 800 then
		return 90 
	elseif targetRank < 1500 then
		return 80 
	elseif targetRank < 2500 then
		return 70 
	elseif targetRank < 3500 then
		return 60 
	elseif targetRank < 4500 then
		return 50
	elseif targetRank < 5500 then
		return 40
	else 
		return 30
	end
end