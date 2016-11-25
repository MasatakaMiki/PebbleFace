var debug_mode = false;

// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function cDateFromUnixTimestamp(unixTimestamp) {
  var date = new Date(unixTimestamp * 1000);
  return date;
}

function locationSuccess(pos) {
  // We will request the weather here
  // Construct URL
  var myAPIKey = 'ea89120ba85fc38f32f1eebc9063f758';
  var lat = pos.coords.latitude;
  var lon = pos.coords.longitude;
  if (debug_mode) {
    lat = 33.586597;
    lon = 130.396447;
  }
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      lat + '&lon=' + lon + '&appid=' + myAPIKey;
  console.log('Url is ' + url);

  var status_weather = '';
  var temperature_f = 0;
  var temperature_c = 0;
  var conditions = '';
  var icon = '';
  var name = '';
  var status_forecast = '';
  var forecasttime1 = '';
  var forecasttime2 = '';
  var forecasttime3 = '';
  var forecasttime4 = '';
  var forecasticon1 = '';
  var forecasticon2 = '';
  var forecasticon3 = '';
  var forecasticon4 = '';

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json_weather = JSON.parse(responseText);

      // Status
      var cod_weather = json_weather.cod;
      console.log('Cod is ' + cod_weather);

      var dt_weather = 0;
      if (cod_weather === 200) {
        status_weather = 'success';
        dt_weather = json_weather.dt;
        // Temperature in Kelvin requires adjustment
        temperature_f = Math.round((json_weather.main.temp - 273.15) * 1.8000 + 32.00);        
        temperature_c = Math.round(json_weather.main.temp - 273.15);
        // Conditions
        conditions = json_weather.weather[0].main;
        icon = json_weather.weather[0].icon;
        if (debug_mode) icon = '01d';
        name = json_weather.name;
      } else if (cod_weather === '404') {
        status_weather = 'ERR_' + cod_weather; //json_weather.message;
      } else {
        status_weather = 'ERR_' + cod_weather;
      }

      console.log('Status[W] is ' + status_weather);
      console.log('Temperature is ' + temperature_f + 'f');
      console.log('Temperature is ' + temperature_c + 'c');
      console.log('Conditions are ' + conditions);
      console.log('Icon is ' + icon);
      console.log('Name is ' + name);

      var url_forecast = 'http://api.openweathermap.org/data/2.5/forecast?lat=' +
        lat + '&lon=' + lon + '&appid=' + myAPIKey;
      console.log('Url is ' + url_forecast);

      // Send request to OpenWeatherMap
      xhrRequest(url_forecast, 'GET', 
        function(responseText) {
          // responseText contains a JSON object with weather info
          var json_forecast = JSON.parse(responseText);

          // Status
          var cod_forecast = json_forecast.cod;
          console.log('Cod is ' + cod_forecast);

          if (cod_forecast === '200') {
            status_forecast = 'success';

            // Replace weather date with unixTimestamp
            var date = new Date();
            dt_weather = Math.floor(date.getTime() / 1000);
            console.log('dt_weather is ' + dt_weather);

            var dt_forecast = 0;
            var cnt = json_forecast.cnt;
            var i = 0;
            for (i = 0; i < cnt; i++) {
              dt_forecast = json_forecast.list[i].dt;
              console.log('dt_forecast is ' + dt_forecast);
              if (dt_forecast > dt_weather) break;
            }
            if (i < cnt) {
              console.log('i is ' + i);
              console.log('dt_txt is ' + json_forecast.list[i].dt_txt);
              forecasttime1 = cDateFromUnixTimestamp(json_forecast.list[i].dt).getHours().toString();
              forecasticon1 = json_forecast.list[i].weather[0].icon;
            }
            if (i + 1 < cnt) {
              forecasttime2 = cDateFromUnixTimestamp(json_forecast.list[i + 1].dt).getHours().toString();
              forecasticon2 = json_forecast.list[i + 1].weather[0].icon;
            }
            if (i + 2 < cnt) {
              forecasttime3 = cDateFromUnixTimestamp(json_forecast.list[i + 2].dt).getHours().toString();
              forecasticon3 = json_forecast.list[i + 2].weather[0].icon;
            }
            if (i + 3 < cnt) {
              forecasttime4 = cDateFromUnixTimestamp(json_forecast.list[i + 3].dt).getHours().toString();
              forecasticon4 = json_forecast.list[i + 3].weather[0].icon;
            }
          } else if (cod_forecast === '404') {
            status_forecast = 'ERR_' + cod_forecast; //json_forecast.message;
          } else {
            status_forecast = 'ERR_' + cod_forecast;
          }

          console.log('Status[F] is ' + status_forecast);
          console.log('Forecast Time 1 is ' + forecasttime1);
          console.log('Forecast Time 2 is ' + forecasttime2);
          console.log('Forecast Time 3 is ' + forecasttime3);
          console.log('Forecast Time 4 is ' + forecasttime4);
          console.log('Forecast Icon 1 is ' + forecasticon1);
          console.log('Forecast Icon 2 is ' + forecasticon2);
          console.log('Forecast Icon 3 is ' + forecasticon3);
          console.log('Forecast Icon 4 is ' + forecasticon4);

          // Assemble dictionary using our keys
          var dictionary = {
            'TEMPERATURE_F': temperature_f,
            'TEMPERATURE_C': temperature_c,
            'ICONNAME': icon,
            'LOCALNAME': name,
            'FORECASTTIME1': forecasttime1,
            'FORECASTTIME2': forecasttime2,
            'FORECASTTIME3': forecasttime3,
            'FORECASTTIME4': forecasttime4,
            'FORECASTICONS1': forecasticon1,
            'FORECASTICONS2': forecasticon2,
            'FORECASTICONS3': forecasticon3,
            'FORECASTICONS4': forecasticon4
          };

          // Send to Pebble
          Pebble.sendAppMessage(dictionary,
            function(e) {
              console.log('Weather info sent to Pebble successfully!');
            },
            function(e) {
              console.log('Error sending weather info to Pebble!');
              console.log('Message failed: ' + JSON.stringify(e));
            }
          );
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