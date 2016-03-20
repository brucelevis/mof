//http.js
var httpcfg = {
	port: 8080
};

var rediscfg = [
	{port:6379, ip:'192.168.160.37', password:'', dbindex:1}
];

process.defrediscfg = rediscfg[0];

var http = require('http');
var url = require('url');
var querystring = require('querystring');
var redisctx = require('./redisctx.js');
var fs = require('fs');


(function(){
	for (var i = 0; i < rediscfg.length; i++){
		redisctx.createRedisCtx(rediscfg[i].port, rediscfg[i].ip, rediscfg[i].password, rediscfg[i].dbindex);
	}

	var server = http.createServer(function(request, response){
		var urlinfo = url.parse(request.url);
		var query = querystring.parse(urlinfo.query);

		var extfile = urlinfo.pathname.split('.');
		if (extfile.length > 1){
			var filepath = './' + urlinfo.pathname;
			fs.readFile(filepath, function(err, data){
				if (!err){
					console.log(data);
					response.write(data);
					response.end();
				}
			})
			return;
		}

		var modname = urlinfo.pathname ? '.' + urlinfo.pathname : './index.js';
		var handler;
		try {
			handler = require(modname);
		}  catch(e) {
			handler = null;
		}

		if (handler != null){
			handler.handle(query, response);
		}
	});

	server.listen(httpcfg.port);
	console.log('server start, listen port:' + httpcfg.port);

})();