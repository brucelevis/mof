var redis = require('redis');

exports.connect = function(cfg, binary){
    var port = cfg.port || 6379;
    var ip = cfg.ip || '127.0.0.1';
    var pwd = cfg.pwd || '';
    var index = cfg.index || 0;

    var client = redis.createClient(port, ip, {return_buffers:binary});
    client.on('error', function(err){
        console.log("Error" + err);
        return null;
    });

    if (pwd.length > 0){
        client.auth(cfg.pwd);
    }

    if (index > 0){
        client.select(index);
    }

    return client;
}

exports.copyTo = function(srcCtx, destCtx, key, force){
	srcCtx.send_command("exists", [key], function(err, data){
		if (err){
			console.log(err);
			return;
		}

		if (data == 1 && !force){
			console.log('already exist ' + key);
			return;
		}

		srcCtx.send_command("dump", [key], function(err, srcData){
			if (err){
				console.log("dump " + key + " fail with " + err);
				return;
			}

			destCtx.send_command("restore", [key, 0, srcData], function(err, reply){
				if (err){
					console.log("restore " + key + " fail with " + err);
					return;
				}

				console.log("restore " + key + " success");
			})
		})
	});
}