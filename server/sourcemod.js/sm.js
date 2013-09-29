(function(){
	require = function(file){
		if(require.cache.hasOwnProperty(file)) return require.cache[file];
		return __require(file);
	}
	
	require.cache = {};
	
})();

