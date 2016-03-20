//request

var buffer = require('./buffer.js');



function beginMsg(type, id)
{
	var netmsg = buffer.newBuffer(4096 * 4);
	netmsg.writeInt(0);
	netmsg.writeInt(type);
	netmsg.writeInt(id);
	return netmsg;
}

 function endMsg(netmsg)
{
	//netmsg.writeIntAt(0, 0);
	netmsg.writeIntAt(netmsg.offset - 4, 0);
}

function Requestor()
{
	var self = this;
	this.proxy = null;

	this.heartBeat = function(){
		var netmsg = beginMsg(0, 0);
		netmsg.writeInt(1);
		netmsg.writeInt64(1, 1);
		netmsg.writeInt(2);
		netmsg.writeInt64(0xFFFF, 0xFFFF);
		netmsg.writeInt64(0, 0x00FF);
		endMsg(netmsg);

		this.proxy.sendPacket(netmsg);
	}

	this.login = function(account, password)
	{
		var netmsg = beginMsg(1, 0);
		netmsg.writeStr(account);
		netmsg.writeStr(password);
		endMsg(netmsg);

		this.proxy.sendPacket(netmsg);
	}

	this.register = function(account, password)
	{
		var netmsg = beginMsg(0, 1);
		netmsg.writeStr(account);
		netmsg.writeStr(password);
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.getRandName = function()
	{
		var netmsg = beginMsg(0, 3);
		netmsg.writeInt(0);
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.selectRole = function(roleid)
	{
		var netmsg = beginMsg(0, 7);
		netmsg.writeInt(roleid);
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.getRoles = function()
	{
		var netmsg = beginMsg(0, 11);
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.createRole = function(type, name)
	{
		var netmsg = beginMsg(0, 5);
		netmsg.writeInt(type);
		netmsg.writeStr(name);
		netmsg.writeStr("");
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.enterCity = function(cityid)
	{
		var netmsg = beginMsg(2, 2);
		netmsg.writeInt(cityid);
		netmsg.writeInt(100);
		netmsg.writeInt(100);
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.registerOrder = function(){

		var netmsg = beginMsg(18, 21);
		netmsg.writeInt(1);
		netmsg.writeInt(100);
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.getSceneRoles = function()
	{
		var netmsg = beginMsg(8, 2);
		netmsg.writeInt(20);
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.getFriendList = function()
	{
		var netmsg = beginMsg(8, 4);
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.finishCopy = function(copyid)
	{
		var netmsg = beginMsg(6, 5);
		netmsg.writeInt(copyid);  //501401 - 501420  dixiacheng
		netmsg.writeInt(5);  //star
		netmsg.writeInt(2)  //state 1 fail 2 pass
		endMsg(netmsg);
		
		this.proxy.sendPacket(netmsg);
	}

	this.getGuildList = function()
	{
		var netmsg = beginMsg(14, 1);
		endMsg(netmsg);	

		this.proxy.sendPacket(netmsg);
	}


	this.chatWorld = function(str)
	{
		var netmsg = beginMsg(8, 18);
		netmsg.writeStr(str);
		endMsg(netmsg);	

		this.proxy.sendPacket(netmsg);
	}

	this.getQuestState = function()
	{
		var netmsg = beginMsg(5, 0);
		endMsg(netmsg);

		this.proxy.sendPacket(netmsg);
	}

	this.recvQuest = function(questid)
	{
		var netmsg = beginMsg(5, 2);
		netmsg.writeInt(questid);
		endMsg(netmsg);

		this.proxy.sendPacket(netmsg);
	}

	this.dialog = function(npcid, idx)
	{
		var netmsg = beginMsg(5, 10);
		netmsg.writeInt(npcid);
		netmsg.writeInt(idx);
		endMsg(netmsg);

		this.proxy.sendPacket(netmsg);
	}

	this.finishQuest = function(questid)
	{
		var netmsg = beginMsg(5, 8);
		netmsg.writeInt(questid);
		endMsg(netmsg);

		this.proxy.sendPacket(netmsg);
	}

	this.finishCopy = function(copyid)
	{
		var netmsg = beginMsg(6, 5);
		netmsg.writeInt(copyid);
		netmsg.writeInt(5); //star
		netmsg.writeInt(2); //1 fail, success 2
		endMsg(netmsg);

		this.proxy.sendPacket(netmsg);
	}

	this.enterWorldBoss = function(){

		console.log("request enter world boss");
		var netmsg = beginMsg(30, 1);
		netmsg.writeInt(0);
		endMsg(netmsg);
		this.proxy.sendPacket(netmsg);
	}

	this.attackWorldBoss = function(){



		console.log("attack")
		//self.chatWorld("why");

		var netmsg = beginMsg(30, 6);
		netmsg.writeInt(0);
		netmsg.writeInt(2);
		netmsg.writeInt(1);
		netmsg.writeInt(0);
		netmsg.writeInt(0);
		endMsg(netmsg);
		this.proxy.sendPacket(netmsg);
/*
    def_int(sceneid)
    def_int(objtype)                        // pet or role
    def_int(isCir)
    def_int(hurts)
    def_int_arr(args)                       // 附加参数
*/

	}
}




exports.createRequestor = function createRequestor(proxy)
{
	var requestor = new Requestor;
	requestor.proxy = proxy;
	return requestor;
}