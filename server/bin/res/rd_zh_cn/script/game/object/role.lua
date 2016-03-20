--role

--[[
RawRole = class()
function RawRole:Ctor()
	self.mRoleId = 10000
	self.mLuaRole = LuaRole:new(self)
	self.mLuaRole:loadData()
end

function RawRole:getLuaRole()
	return self.mLuaRole
end

function RawRole:getInstID()
	return 1000000
end
--]]

LuaRole = class()
function LuaRole:Ctor(role)
	print("LuaRole:Ctor")
	self.mRawRole = role
	self.mComponents = {}
	self.mAbility = {mBase = {}, mBattle = {}}
	self:initComponents();
end

function LuaRole:loadData()
	local data = self.mRawRole:loadLuaRole();
	for name, component in pairs(self.mComponents) do
		local componentName = component:getName()
		local componentData = data[componentName]
		if componentData ~= nil then
			local tData = jsonDecode(componentData)
			component:loadData(tData)
		else 
			component:firstLoadData();
		end
	end
end

function LuaRole:initComponents()
	self:addComponent(TestComponent:new(self))
end

function LuaRole:addComponent(component)
	local componentName = component:getName()
	if self.mComponents[componentName] ~= nil then
		assert(false)
	end
	self.mComponents[componentName] = component
end

function LuaRole:saveComponent(component)
	local componentName = component:getName()
	local componentData = component:saveData()
	local data = {}
	data[componentName] = jsonEncode(componentData)
	self.mRawRole:saveLuaRole(data);

	print(componentName, data[componentName]);
end

function LuaRole:saveComponents(components)
	for index, component in ipairs(components) do
		self:saveComponent(component)
	end
end

function LuaRole:calcAbility()
	local base = self.mAbility.mBase
	local battle = self.mAbility.mBattle

	base.mStre = 0
	base.mInte = 0
	base.mPhys = 0
	base.mCapa = 0

	battle.mAtk = 0
	battle.mDef = 0
	battle.mDodge = 0
	battle.mHit = 0
	battle.mCri = 0
	battle.mMaxHp = 0

	return self.mAbility.mBase, self.mAbility.mBattle
end

function createLuaRole(role)
	local luaRole = LuaRole:new(role)
	return luaRole
end

--[[
function getRawRole(sessionId)
	if (g_RawRole == nil) then
		g_RawRole = RawRole:new()
		assert(g_RawRole ~= nil)

	end
	return g_RawRole
end
--]]
