--base component
LuaComponent = class()

function LuaComponent:Ctor(object)
	self.mObject = object
	self.mData = {}
end

function LuaComponent:getName()
	assert(false)
end

function LuaComponent:loadData(sData)
end

function LuaComponent:saveData()
	return nil
end

function LuaComponent:offline()
end

function LuaComponent:online()

end

function LuaComponent:processDailyRefresh()
	
end