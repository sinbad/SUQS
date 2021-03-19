# Editor Setup

Quest definitions are most easily set up using JSON. To make it even easier, a JSON schema 
for the quest format exists in the repository at docs/questschema.json. You can hook this up to your
editors to provide autocomplete and pop-up documentation for every element of the quest definition format.

In order to enable these features in your editor, you need to set up a JSON Schema mapping.

## Visual Studio Code

1. Open Settings
1. Go to Extensions > JSON
1. Ensure that Schema Download is Enabled
1. Under Schemas, click Edit in settings.json
1. Use the following setting or similar:

```json
"json.schemas": [
    {
        "fileMatch": [
            "*.suqs.json"
        ],
        "url": "https://raw.githubusercontent.com/sinbad/SUQS/master/docs/questschema.json"
    }
]
```

This means if you save your JSON quest files as Anything.suqs.json, they will automatically gain 
auto-complete and tooltips for all properties. You could link the schema from your local drive
as well, but this URL format means you're not dependent on that.

## JetBrains Rider

1. Open Settings 
1. Go to Languages & Frameworks > Schemas and DTDs > JSON Schema Mappings
1. Add a new entry, name it however you like
1. Set the URL to "https://raw.githubusercontent.com/sinbad/SUQS/master/docs/questschema.json"
1. Set the Schema Version to 7
1. Add a file pattern of *.suqs.json
1. Save


