Monster = {x=0,y=0};
--这句是重定义元表的索引，就是说有了这句，这个才是一个类。具体的解释，请百度。
Monster.__index = Monster

--构造体，构造体的名字是随便起的，习惯性改为new()
function Monster:new(x,y)
    local self = {}  --初始化self，如果没有这句，那么类所建立的对象改变，其他对象都会改变
    setmetatable(self, Monster)  --将self的元表设定为Class
    self.x = x   --属性值初始化
    self.y = y
  	return self  --返回自身
end
--这里定义类的其他方法
function Monster:test()
    print(self.x,self.y)
end
function Monster:plus()
    self.x = self.x + 1
    self.y = self.y + 1
end

a = Monster:new(10,20)
a:test()
b = Monster:new(11,23)
b:test()
b:plus()
b:test()


print "============================";

--声明了新的属性Z
FireMonster = {z=0, old_mon=nil};
--设置类型是Class
setmetatable(FireMonster, Monster);

--还是和类定义一样，表索引设定为自身
FireMonster.__index = FireMonster;

--这里是构造体，看，加上了一个新的参数
function FireMonster:new(x,y,z)
   local self = {} ;				--初始化对象自身
   self = Monster:new(x,y); 		--将对象自身设定为父类，这个语句相当于其他语言的super
   setmetatable(self, FireMonster); --将对象自身元表设定为Main类
   self.z= z; 						--新的属性初始化，如果没有将会按照声明=0
   self.old_mon = Monster:new(x,y);
   return self;
end

function FireMonster:test()

	-- 调用父类的实现，需要将self重新设置回父类，然后再调用
   	setmetatable(self, Monster); --将对象自身元表设定为父类  
	self:test();

	-- 父类方法调用完毕，这里重新设置回来
   	setmetatable(self, FireMonster); --将对象自身元表设定为父类  

	print("fire !! --->");
	print(self.x,self.y,self.z);
end

mon_1 = FireMonster:new(76,11,04);
mon_2 = FireMonster:new(82,10,26);
mon_1:test();
print "=============================";
mon_2:test();