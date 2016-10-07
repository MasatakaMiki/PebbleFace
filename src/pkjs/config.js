module.exports = [
  {
    "type": "heading",
    "defaultValue": "App Configuration"
  },
  {
    "type": "text",
    "defaultValue": "Please change as you like..."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "More Settings"
      },
      {
        "type": "radiogroup",
        "messageKey": "TEMPERATURE_SCALE",
        "label": "Temperature Scale",
        "options": [
          { 
            "label": "Fahrenheit", 
            "value": "fahrenheit" 
          },
          { 
            "label": "Celsius", 
            "value": "celsius" 
          }
        ]
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];