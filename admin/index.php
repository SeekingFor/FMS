<?php
	include_once('config.php');

function content()
{
?>
		<h2>Home</h2>
		<p class="paragraph">
		Use these pages to administer your FMS installation.  Make sure to set $dblocation correctly in config.php, and that the database is writable to your web service user account.  While you should be able to safely use these pages while FMS is running, it is recommended that you shut down FMS prior to make any changes here.
		</p>
<?php
}

	include_once('linkbar.php');
	include_once('template.php');

?>