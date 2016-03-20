--main lua
print("lua cpath: "..package.cpath)
print("lua path: "..package.path)

require "luaClass"

require "object/role"
require "object/component"

require "msgHandler/msgHandler"

require "testComponent/testComponent"
require "testComponent/testHandler"

print(jsonEncode({a=1, b={2,4,5}}))
