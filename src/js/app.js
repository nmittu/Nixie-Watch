Pebble.addEventListener('appmessage',
  function(e){
		console.log('AppMessage recived!');
	}
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://pebbleconf-remote.mittudev.com/nixiewatch.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));
  console.log('Config window returned: ', JSON.stringify(config_data));
	
	var dictionary ={
		VIBE_TIME: config_data.minutes
	};
	
	Pebble.sendAppMessage(dictionary,
		function(e){
			console.log('Sent');
		},
		function(e){
			console.log('Error Sending minutes To Pebble!');
		}
	);
	
});