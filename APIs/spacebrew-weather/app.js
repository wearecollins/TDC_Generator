var tembooAuth = require("./auth/auth_temboo").tAuth
	, tsession = require("temboo/core/temboosession")
	, session = new tsession.TembooSession(tembooAuth.user, tembooAuth.app, tembooAuth.key)
	, Spacebrew = require("./Spacebrew.js").Spacebrew
  	, request = require('request');

// SPACEBREW OBJECT
var sb;

// RSS stuff
var FeedParser = require('feedparser');

var Yahoo = require("temboo/Library/Yahoo/Weather");

var getWeatherChoreo = new Yahoo.GetWeather(session);
var getWeatherByCoordinatesChoreo = new Yahoo.GetWeatherByCoordinates(session);

// Instantiate and populate the input set for the choreo
var getWeatherByCoordinatesInputs = getWeatherByCoordinatesChoreo.newInputSet();

// Instantiate and populate the input set for the choreo
var getWeatherInputs = getWeatherChoreo.newInputSet();

// cache coords
var lastLat = 0;
var lastLong = 0;
var currWOEID = 0;

function getWeather( lat, lon ){
	if ( lat == lastLat && lon == lastLong && currWOEID != 0 ){
		getWeatherByWOEID(currWOEID);
	} else {
		// Set inputs
		getWeatherByCoordinatesInputs.set_Latitude( lat );
		getWeatherByCoordinatesInputs.set_Longitude( lon );

		lastLong 	= lon;
		lastLat 	= lat;

		// Run the choreo, specifying success and error callback handlers
		getWeatherByCoordinatesChoreo.execute(
		    getWeatherByCoordinatesInputs,
		    function(results){
		    	currWOEID = results.get_WOEID();
		    	getWeatherByWOEID( currWOEID );
		    },
		    function(error){
		    	console.log(error.type); console.log(error.message);
		    }
		);
	}
}

function getWeatherByWOEID( id ){
	// Set inputs
	getWeatherInputs.set_WOEID( id );

	// Run the choreo, specifying success and error callback handlers
	getWeatherChoreo.execute(
	    getWeatherInputs,
	    function(results){
	    	// feedparser
	    	//sb.send("weather_raw", "xml", String(results.get_Response()));

	    	var feedparser = new FeedParser({normalize:false});
			feedparser.end(results.get_Response());
			// RSS FEED PARSE EVTS
			feedparser.on('error', function(error) {
				console.log("feed error");
				console.log( error );
			  // always handle errors
			});
			feedparser.on('readable', function() {
			  // This is where the action is!
			  var stream = this
			    , meta = this.meta // **NOTE** the "meta" is always available in the context of the feedparser instance
			    , item;

			  while (item = stream.read()) {
			  	if ( item['yweather:condition']){
			  		console.log(item['yweather:condition']['@']['text']);
			  		console.log(item['yweather:condition']['@']['temp']);
					sb.send("temperature", "range", parseInt(item['yweather:condition']['@']['temp']).toString() );
					sb.send("conditions", "string", item['yweather:condition']['@']['text'] );
					sb.send("condition_coded", "range", parseInt(item['yweather:condition']['@']['code']).toString() );
			  		break;
			  	}
			  }
			  

			});
	    },
	    function(error){console.log(error.type); console.log(error.message);}
	);
}


/**
 * [main description]
 * @return {[type]} [description]
 */
var main = function() {
	sb = new Spacebrew.Client("localhost", "weather", "");
	sb.addPublish("weather_raw", "xml", "");
	sb.addPublish("temperature", "range", 0 );
	sb.addPublish("condition_coded", "range", 0 );
	sb.addPublish("conditions", "string", 0 );
	// {"lat":0,"long":0}
	sb.addSubscribe("getWeather", "lat_long");
	sb.connect();

	sb.onCustomMessage = function(name, type, value){
		if ( name == "getWeather"){
			value = JSON.parse(value);
			getWeather(value.lat, value.long);
		}
	}

	// set interval 4ever
	var lat = 40.714352999999996;
	var lon = -74.00597299999998;
	setInterval(function(){
		getWeather(lat, lon);
	}, 120000 );
	getWeather(40.714352999999996, -74.00597299999998);
}

/**
 * Run the app
 */
main();