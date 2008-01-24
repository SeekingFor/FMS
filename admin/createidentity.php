<?php
	
	include_once('config.php');
	include_once('linkbar.php');

function content()
{
	
	global $dblocation;
	
	if(isset($_REQUEST["formaction"]) && $_REQUEST["formaction"]=="create" && $_REQUEST["name"]!="")
	{
		$db=new PDO('sqlite:'.$dblocation);
		$st=$db->prepare("INSERT INTO tblLocalIdentity(Name,PublishTrustList) VALUES(?,'true');");
		$st->bindParam(1,$_REQUEST["name"]);
		$st->execute();
?>
	<h2>Identity Created</h2>
<?php
	}
	else
	{
?>
	<h2>Create Identity</h2>
	<form name="frmcreateidentity" method="POST">
	<input type="hidden" name="formaction" value="create">
	Name : <input type="text" name="name">
	<input type="submit" value="Create">
	</form>
<?php
	}
}
	
	include_once('template.php');

?>