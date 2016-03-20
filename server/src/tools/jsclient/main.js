//main.js

var client = require('./client.js');
var questmgr = require('./questmgr.js')

//服务器ip和端口
var serverconfig = {
	//host: '192.168.160.201',
	//host: '121.9.235.39',
	//host: '192.168.110.91',
	//port: 9999,
	host: "192.168.110.92",
	port: 18081
};

/*
@1
connect server
 |
login --------success-- selectrole ------- do game logic
		 |
		fail
		 |
		register ---- get random name ---- createrole ---- select role --- do game logic

@2
login account: gametest + idx 
for (var i = 0; i < clientnum; i++){
	idx = start + i;
}
*/
//机器人数量
var clientnum = 1;
var clients = [];

var cur = 0;
function runClients()
{
	if (cur >= clientnum)
	{
		return;
	}

	clients[cur].run();
	cur++;
	setTimeout(runClients, 50);
}

function loadIniCfg()
{
	questmgr.load();
}

(function(){
	loadIniCfg();

	for (var i = 0; i < clientnum; i++)
	{
		var c = client.createClient(serverconfig, i);
		clients.push(c);
	}

	runClients();
})();

