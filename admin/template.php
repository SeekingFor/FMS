<html xml:lang="en" xmlns="http://www.w3.org/1999/xhtml">
<head>
	<title>FMS : Freenet Message System</title>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<link rel="stylesheet" href="layout_deneva.css" type="text/css" media="screen" />
	<link rel="stylesheet" href="style_deneva.css" type="text/css" media="screen" />
</head>

<body>
	
	<div class="banner">

	FMS : Freenet Message System
	</div>
	
	<div class="content_main">

	<div class="content_left">

		<?php
		if(function_exists('linkbar'))
		{
			linkbar();	
		}
		?>
	
	</div>
	
	<div class="content_right">
	
		<?php
		if(function_exists('content'))
		{
			content();	
		}
		?>
		
	</div>
	
	</div>


</body>
</html>
