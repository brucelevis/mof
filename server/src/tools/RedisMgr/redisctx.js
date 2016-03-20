//redis
var redis = require('redis');
var os = require('os');
var ctxes = [];

var RedisContext = function(){
	var self = this;
	this.client = null;
	this.allkeys = [];
	this.lastupdate = null;

	this.connect = function(port, ip, password, dbindex){
		var client = redis.createClient(port, ip);

		client.on('error', function(err){
			console.log('redis error ' + err);
		})

		client.on('connect', function(){
			self.client = client;
			var addr = ip + ":" + port;
			ctxes[addr] = self;
			console.log('connect to redis server ' + addr);

		})
		
		client.auth(password);
		client.select(dbindex);
	}

	this.getKeys = function(callback){
		if (self.lastupdate != null){
			var now = os.uptime();
			if (now - self.lastupdate < 60){
				callback(null, self.allkeys);
			}
		}

		self.client.send_command("keys", ["*"], function(err, reply){
			if (err){
				callback(err);
				return;
			}

			self.allkeys = reply;
			self.allkeys.sort();
			callback(null, self.allkeys);
			self.lastupdate = os.uptime();
		})
	}

	this.get = function(key, val){
		self.client.send_command("get", [key], function(err, data){
			if (err){
				return;
			}
			val = data;
		});
	}

	this.set = function(key, val, callback){
		self.client.send_command("set", [key, val], function(err, data){
			callback(err, data);
		});
	}

	this.hget = function(key, hkey, callback){
		self.client.send_command("hget", [key, hkey], function(err, data){
			callback(err, data);
		});
	}

	this.hset = function(key, hkey, val, callback){
		console.log(key);
		console.log(hkey);
		console.log(val);
		self.client.send_command("hset", [key, hkey, val], function(err, data){
			callback(err, data);
		});
	}

	this.hgetall = function(key, callback){
		self.client.send_command("hgetall", [key], function(err, data){
			callback(err, data)
		});
	}

	this.smem = function(key, members){

	}

	this.getKeyDetail = function(key, callback){
		var detail = {};
		detail.type = 'none';
		detail.key = key;
		self.client.send_command("type", [key], function(err, data){
			if (data == 'hash'){
				detail.type = 'hash';
				detail.keyvals = {};
				self.client.send_command("hgetall", [key], function(err, data){
					if (err == null){
						detail.keyvals = data;
					}
					callback(detail);
				})
			}
			else if (data == 'set'){
				detail.type = 'set';
				detail.members = [];
				self.client.send_command("smembers", [key], function(err, data){
					if (err == null){
						detail.members = data;
					}
					callback(detail);
					console.log(err);
				})
			}
			else if (data == 'string'){
				detail.type = 'string';
				detail.val = '';
				self.client.send_command("get", [key], function(err, data){
					if (err == null){
						detail.val = data;
						console.log(data);
					}
					callback(detail);
				})
			}
		});
	}
}

exports.createRedisCtx = function(port, ip, password, callback){
	var ctx = new RedisContext;

	if (ip == undefined){
		ip = "127.0.0.1";
	}

	if (port == undefined){
		port = 6379;
	}

	var addr = ip + ":" + port;
	if (ctxes[addr] == undefined){
		ctx.connect(port, ip, password, callback);
	}
}

exports.getRedisCtx = function(port, ip){
	var addr = ip + ":" + port;
	return ctxes[addr];
}