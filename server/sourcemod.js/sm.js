(function(){
	require = function(file){
		if(require.cache.hasOwnProperty(file)) return require.cache[file];
		__require(file);
	}
	
	require.cache = {};
	
})();

