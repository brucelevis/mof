--占城商店购买物品价格计算（配置表价格，玩家占城分）
function guildStore_occupy_buy(value, point)
	if point >= 100 then
		return value
	end
	return value * (100 - point) / 100
end