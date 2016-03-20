//proxy.js

var net = require('net');
var buffer = require('./buffer.js');

var NetProxy = function()
{
	var self = this;

	this.sock = null;
	this.handler = null;
	this.client = null;
	this.buffer = new Buffer(0);;

	this.sendPacket = function(buf)
	{
		var pack = new Buffer(buf.offset);
		buf.buf.copy(pack, 0, 0, buf.offset);

		self.sock.write(pack);
	}

	this.sendRaw = function(raw)
	{
		self.sock.write(raw);
	}

	this.connectServer = function(serverconfig, callback)
	{
		var sock = net.createConnection(serverconfig, 
			function(){
				self.sock = sock;
				callback();
			});


		sock.on('data', self.onData);
		sock.on('end', self.onEnd);
		sock.on('error', self.onError);
	}

	this.disconnect = function(){
		self.sock.end();
	}


	this.onData = function(data)
	{
		if (self.buffer.length == 0)
		{
			self.buffer = data;
		}
		else
		{
			self.buffer = Buffer.concat([self.buffer, data]);
		}

		while (self.buffer.length > 4)
		{
			var packlen = self.buffer.readInt32LE(0);
			if (packlen + 4 <= self.buffer.length)
			{
				var packbuf = new Buffer(packlen);
				self.buffer.copy(packbuf, 0, 4, packlen + 4);

				try
				{
					var ack = buffer.initBuffer(packbuf);
					self.handler.handle(self.client, ack); 

				}
				catch(e)
				{
					console.log(e);
				}

				self.buffer = self.buffer.slice(4 + packlen);
			}
			else
			{
				break;
			}
		}
	}

	this.onEnd = function()
	{
		self.client.onDisconnect();
	}

	this.onError = function()
	{
		console.log('client socket on error, ignored');
	}
};

exports.createProxy = function(handler, client)
{
	var proxy = new NetProxy;
	proxy.handler = handler;
	proxy.client = client;

	return proxy;
}