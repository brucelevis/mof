var redisHelper = require('./redishelper.js');

var srcCfg = {
	ip : '192.168.110.91',
	port : 6379,
	index : 0,
	pwd : ''
}

var destCfg = {
	ip : '',
	port : 6379,
	index : 0,
	pwd : ''
}


var srcCtx = redisHelper.connect(srcCfg);
var bsrcCtx = redisHelper.connect(srcCfg, true);
var destCtx = redisHelper.connect(destCfg);
var bdestCtx = redisHelper.connect(destCfg, true);

backupKey(srcCtx, bsrcCtx, destCtx, bdestCtx, "paihang:jjc");


function backupKey(srcCtx, bsrcCtx, destCtx, bdestCtx, key){
	var backupKey = "backup:" + key;
	destCtx.send_command("rename", [key, backupKey], function(err, reply){
		if (err){
			console.log("rename failed with err:" + err);
		}

		redisHelper.copyTo(bsrcCtx, bdestCtx, key, true);
	})
}
