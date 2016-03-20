//updatestring
var redisctx = require('./redisctx.js')

exports.handle = function(query, response){
	var ip = query.ip ? query.ip : process.defrediscfg.ip;
	var port = query.port ? query.port : process.defrediscfg.port;
	var ctx = redisctx.getRedisCtx(port, ip);
	
	if (query.key == undefined || query.val == undefined){
		response.write("err");
		response.end();
		return;
	}

	ctx.set(query.key, query.val, function(err, data){
		if (err == null){
			response.write("ok");
		}
		else{
			response.write("err");
		}
		response.end();
	});
}