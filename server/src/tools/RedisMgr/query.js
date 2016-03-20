//query.jsvar redisctx = require('./redisctx');
var redisctx = require('./redisctx.js')

exports.handle = function(query, response){
	var ip = query.ip ? query.ip : process.defrediscfg.ip;
	var port = query.port ? query.port : process.defrediscfg.port;
	var ctx = redisctx.getRedisCtx(port, ip);

	ctx.getKeys(function(err, keys){
		if (err != null){
			response.write("{\"error\": \"can't get data from db\"}");
			response.end();
		}

		response.write("{\"data\":\"" +　keys.toString() + "\"}");
		response.end();
	});
}