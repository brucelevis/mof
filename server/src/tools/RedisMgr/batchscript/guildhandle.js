var redisHelper = require('./redishelper.js');

var dbCfg = {
	ip : '',
	port : 6379,
	index : 0,
	pwd : ''
}

var dbCtx = redisHelper.connect(dbCfg);


/*dbCtx.send_command("keys", ["guildlog:*"], function(err, data){

	if (err){
		return;
	}

	for(var k in data){
		console.log(data[k]);

		var backup = data[k] + ":backup";
		dbCtx.send_command("rename", [data[k], backup], function(err, data){
			if (err){
				console.log("fail" + err);
				return;
			}

			console.log("success" + data);
		})
	}
})*/

dbCtx.send_command("keys", ["guild:*"], function(err, data){
	if (err){
		return;
	}

	var command = "hmset";
	var params = ["guildallid:0"];
	for (var k in data){

		var arr = data[k].split(":");

		params.push(arr[1]);
		params.push(arr[1]);
	}

	console.log(params);

	dbCtx.send_command(command, params, function(err, data){
		if (err){
			console.log("fail");
			return;
		}

		console.log(data);
	})
})