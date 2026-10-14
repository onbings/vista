#!/usr/bin/python
#
# Copyright 2017 Otto Seiskari
# Licensed under the Apache License, Version 2.0.
# See http://www.apache.org/licenses/LICENSE-2.0 for the full text.
#
# This file is based on
# https://github.com/swagger-api/swagger-ui/blob/4f1772f6544699bc748299bd65f7ae2112777abc/dist/index.html
# (Copyright 2017 SmartBear Software, Licensed under Apache 2.0)
#
"""
Usage:

    python3 swagger-yaml-to-html.py --input <path/to/api.yaml> --output <path/to/output.html>

"""
import yaml, json, sys, argparse

TEMPLATE = """
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Swagger UI</title>
  <link href="https://fonts.googleapis.com/css?family=Open+Sans:400,700|Source+Code+Pro:300,600|Titillium+Web:400,600,700" rel="stylesheet">
  <link rel="stylesheet" type="text/css" href="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/3.24.2/swagger-ui.css" >
  <style>
    html
    {
      box-sizing: border-box;
      overflow: -moz-scrollbars-vertical;
      overflow-y: scroll;
    }
    *,
    *:before,
    *:after
    {
      box-sizing: inherit;
    }

    body {
      margin:0;
      background: #fafafa;
    }
  </style>
</head>
<body>

<div id="swagger-ui"></div>

<script src="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/3.24.2/swagger-ui-bundle.js"> </script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/3.24.2/swagger-ui-standalone-preset.js"> </script>
<script>
window.onload = function() {

  var spec = %s;

  // Build a system
  const ui = SwaggerUIBundle({
    spec: spec,
    dom_id: '#swagger-ui',
    deepLinking: true,
    presets: [
      SwaggerUIBundle.presets.apis,
      SwaggerUIStandalonePreset
    ],
    plugins: [
      SwaggerUIBundle.plugins.DownloadUrl
    ],
    layout: "StandaloneLayout"
  })

  window.ui = ui
}
</script>
</body>

</html>
"""

def main():
    # Set up the argument parser
    parser = argparse.ArgumentParser(description='Convert a Swagger YAML file to a standalone HTML file with Swagger UI.')
    parser.add_argument('--input', type=str, required=True, help='Path to the input YAML file.')
    parser.add_argument('--output', type=str, help='Path to the output HTML file. If not specified, prints to stdout.')
    args = parser.parse_args()
    print(f"Convert the Swagger YAML file {args.input} to a standalone HTML file {args.output}")
    # Read the YAML file from the specified input path
    try:
        with open(args.input, 'r') as infile:
            spec = yaml.load(infile, Loader=yaml.FullLoader)
    except FileNotFoundError:
        print(f"Error: The file '{args.input}' was not found.", file=sys.stderr)
        sys.exit(1)
    except yaml.YAMLError as e:
        print(f"Error: Failed to parse YAML from '{args.input}': {e}", file=sys.stderr)
        sys.exit(1)

    # Convert the YAML to JSON and insert it into the HTML template
    html_content = TEMPLATE % json.dumps(spec)

    # Write the HTML content to the specified output file or stdout
    if args.output:
        try:
            with open(args.output, 'w') as outfile:
                outfile.write(html_content)
            print(f"Successfully generated HTML file at '{args.output}'", file=sys.stderr)
        except IOError as e:
            print(f"Error: Failed to write to file '{args.output}': {e}", file=sys.stderr)
            sys.exit(1)
    else:
        sys.stdout.write(html_content)

if __name__ == '__main__':
    main()
