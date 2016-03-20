//redis
var redisHelper = require('./redishelper.js');


var alldata =[]

for (var i = 0; i < alldata.length; i += 2){
	console.log("roleid:" + alldata[i] + " rmb:" + alldata[i + 1]);
}


var dbCfg = {
	//ip : '',
	//port : 6384,
	//index : 1,
	//pwd : ''
}

var dbCtx = redisHelper.connect(dbCfg);

function listData(roleid){
	var roleKey = "role:" + roleid;
	dbCtx.send_command("hget", [roleKey, "rmb"], function(err, data){

		if (err){
			console.log(roleKey + "error");
			return;
		}

		if (!data){
			console.log(roleKey + "error");
			return;
		}


		var newRmb = 0;

		var currmb = (data);
		console.log(roleKey + "'s rmb " + currmb);
	});
}

function opData(roleid, rmb){
	var roleKey = "role:" + roleid;
	dbCtx.send_command("hget", [roleKey, "rmb"], function(err, data){

		if (err){
			console.log(roleKey + "error");
			return;
		}

		if (!data){
			console.log(roleKey + "error");
			return;
		}


		var newRmb = 0;

		var currmb = (data);
		if (currmb > rmb){
			newRmb = currmb - rmb;
		}

		(function(){
			dbCtx.send_command("hset", [roleKey, "rmb", newRmb], function(err1, data1){
				if (err1){
					console.log(roleKey + "error");
					return;
				}

				console.log(roleKey + "'s prev is " + currmb + " and new rmb is " + newRmb + " and minus " + rmb + " and return " + data1);
			})
		})();
	} )
}

for (var i = 0; i < alldata.length; i += 2){
	var roleid = alldata[i];
	var rmb = (alldata[i + 1]);

	listData(roleid, rmb);
	opData(roleid, rmb);
	listData(roleid, rmb);
}