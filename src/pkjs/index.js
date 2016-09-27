var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // We will request the weather here
  // Construct URL
  var myAPIKey = 'ea89120ba85fc38f32f1eebc9063f758';
  var lat = pos.coords.latitude;
  var lon = pos.coords.longitude;
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      lat + '&lon=' + lon + '&appid=' + myAPIKey;
  console.log('Url is ' + url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Status
      var cod = json.cod;
      console.log('Cod is ' + cod);
      
      var status = '';
      var temperature = 0;
      var conditions = '';
      var icon = '';
      var name = '';

      if (cod === 200) {
        status = 'success';
        // Temperature in Kelvin requires adjustment
        temperature = Math.round(json.main.temp - 273.15);
        // Conditions
        conditions = json.weather[0].main;
        icon = json.weather[0].icon;
        name = json.name;
      } else if (cod === '404') {
        status = json.message;
      } else {
        status = 'failure';
      }

      console.log('Status is ' + status);
      console.log('Temperature is ' + temperature);
      console.log('Conditions are ' + conditions);
      console.log('Icon is ' + icon);
      console.log('Name is ' + name);

      // Assemble dictionary using our keys
      var dictionary = {
        'REQSTATUS': status,
        'TEMPERATURE': temperature,
        'CONDITIONS': conditions,
        'ICONNAME': icon,
        'LOCALNAME': name
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }                     
);