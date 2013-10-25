(function(){
	// Remove __require from the global scope
	var _require = __require;
	delete __require;
	
	require = function(file){
		if(require.cache.hasOwnProperty(file)) return require.cache[file];
		return require.cache[file] = _require(file);
	}
	
	require.cache = {};
	
	Math.randomPointInCircle = function(center, radius){
		// Returns a random point in a circle with uniform distribution
		var t = 2 * Math.PI * Math.random();
		var u = Math.random() + Math.random();
		var r = u > 1 ? 2 - u : u;
		return {x: center.x + r * Math.cos(t), y: center.y + r * Math.sin(t), z: center.z}; // z provided for convenience
	}
	
	Array.prototype.remove = function(e){
		var i = this.indexOf(e);
		if(i != -1){
			this.splice(i, 1);
		}
	}
	
	Array.prototype.last = function(){
		if(this.length == 0) throw new Error("Array is empty");
		return this[this.length - 1];
	}
	
	Array.prototype.random = function(){
		return this[~~(Math.random() * this.length)];
	}
	
	server.printToChatAll = function(str){
		for (var i = 0; i < server.clients.length; i++) {
			if(server.clients[i] == null || !server.clients[i].isInGame()) continue;
			server.clients[i].printToChat(str);
		}
	}
	
	server.formatToChatAll = function(str){
		server.printToChatAll(format(str, Array.prototype.slice.call(arguments, 1)));
	}
	
	
	;(function(){
		// String formatter
		
		var formatMap = {
			"none":   "\x06",
			"gray":   "\x06", 
			"grey":   "\x06", 
			
			"green":  "\x0C", 
			"dpurple": "\x0D", 
			"spink":  "\x0E",
			//"orange":  "\x0F",
			"dyellow":  "\x10",
			"pink":  "\x11",
			
			"red":    "\x12",
			//"orange":    "\x13",
			//"gold":    "\x14",
			"lgreen":  "\x15",
			"blue":  "\x16",
			"dgreen":   "\x18",
			"sblue":   "\x19",
			"purple": "\x1A", 
			"orange":    "\x1B",
			"lred":    "\x1C",
			"gold":    "\x1D"
		};
		
		format = function(str, arr){
			var indexing;
			var args;
			if(Array.isArray(arr)){
				args = arr;
				indexing = 1;
			} else{
				args = arguments;
				indexing = 0;
			}
			
			return str.replace(/\{([a-z0-9]+)\}/gi, function(all, id){
				if(/^[0-9]+$/.test(id)){
					var i = parseInt(id, 10);
					if(i <= 0 || i >= args.length + indexing) return all;
					return args[i - indexing];
				}
				
				if(formatMap.hasOwnProperty(id)){
					return formatMap[id];
				}
				
				return all;
			});
		}
		
		format.addAlias = function(key, value){
			formatMap[key] = value;
		}
		
		String.prototype.format = function(){
			return format(this, Array.prototype.slice.call(arguments));
		}
	})();
	
	
})();

