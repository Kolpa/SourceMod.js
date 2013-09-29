(function(){
	require = function(file){
		if(require.cache.hasOwnProperty(file)) return require.cache[file];
		return require.cache[file] = __require(file);
	}
	
	require.cache = {};
	
	Array.prototype.remove = function(e){
		var i = this.indexOf(e);
		if(i != -1){
			this.splice(i, 1);
		}
	}
	
})();

