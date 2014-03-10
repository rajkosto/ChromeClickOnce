<?php

if (!isset($_GET['jsonp']))
{
	http_response_code(400); //bad request, missing jsonp
	die('Missing JS function name parameter');
}

$jsFuncName = $_GET['jsonp'];

$request_headers = array();
if (!function_exists('getallheaders')) {
   foreach ($_SERVER as $name => $value) {
      /* RFC2616 (HTTP/1.1) defines header fields as case-insensitive entities. */
      if (strtolower(substr($name, 0, 5)) == 'http_') {
         $headers[str_replace(' ', '-', ucwords(strtolower(str_replace('_', ' ', substr($name, 5)))))] = $value;
      }
   }
   $request_headers=$headers;
} else {
   $request_headers = getallheaders();
}

$netVersions = 'null';
foreach($request_headers as $headerKey => $headerVal)
{
	if (!strcasecmp($headerKey,'X-ClickOnceSupport'))
	{
		$netVersions = "'$headerVal'";
		break;
	}
}

header('Content-Type: application/javascript');
echo "$jsFuncName($netVersions);";