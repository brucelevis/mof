//buffer.js


var NetBuffer = function(){
	this.buf = null;
	this.offset = 0;

	this.writeIntAt = function(val, pos)
	{
		this.buf.writeInt32LE(val, pos);
	}

	this.writeInt = function(val)
	{
		this.buf.writeInt32LE(val, this.offset);
		this.offset += 4;
	}

	this.writeInt64 = function(low, high){
		this.buf.writeInt32LE(low, this.offset);
		this.buf.writeInt32LE(high, this.offset + 4);
		this.offset += 8;
	}

	this.writeStr = function(val)
	{
		var len = Buffer.byteLength(val);
		this.writeInt(len);
		this.buf.write(val, this.offset, len);
		this.offset += len;		
	}

	this.readInt = function()
	{
		var val = this.buf.readInt32LE(this.offset);
		this.offset += 4;
		return val;
	}

	this.readStr = function()
	{
		var len = this.readInt();	
			console.log(len);	
		if (len == 0)
		{
			return "";
		}
		var str = this.buf.toString('utf8', this.offset, this.offset + len);
		this.offset += len;
		return str;
	}
}

function newBuffer(len)
{
	var buffer = new NetBuffer;
	buffer.buf = new Buffer(len);
	return buffer;
}

function initBuffer(buf)
{
	var buffer = new NetBuffer;
	buffer.buf = buf;
	return buffer;
}

exports.newBuffer = newBuffer;
exports.initBuffer = initBuffer;
