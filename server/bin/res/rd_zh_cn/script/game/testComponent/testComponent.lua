--test component

TestComponent = class(LuaComponent)


function TestComponent:Ctor(object)
	self.mObject = object
	self.mTestValue = 0
end

function TestComponent:getName()
	return "TestComponent"
end

function TestComponent:loadData(tData)
	self.mTestValue = tData.TestValue + 10000
	self.mObject:saveComponent(self)
end

function TestComponent:firstLoadData()
	self.mTestValue = 10010
	self.mObject:saveComponent(self)
end

function TestComponent:saveData()
	local tData = {}
	tData.TestValue = self.mTestValue
	tData.TestArray = {1, 2, 3, 4}
	tData.TestArray[100] = 5
	return tData;
end

function TestComponent:processDailyRefresh()
	
end

function TestComponent:getTestValue()
	return self.mTestValue
end

function TestComponent:setTestValue(val)
	self.mTestValue = val
end