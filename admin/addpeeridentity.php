<?php

	include_once('config.php');
	include_once('linkbar.php');
	
function content()
{
	global $dblocation;
	
	if(isset($_REQUEST["formaction"]) && $_REQUEST["formaction"]=="addpeer" && $_REQUEST["publickey"]!="")
	{
		$message="";
		$db=new PDO('sqlite:'.$dblocation);
		$st=$db->prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
		$st->bindParam(1,$_REQUEST["publickey"]);
		$st->execute();
		if($record=$st->fetch())
		{
			$message="This peer already exists";
		}
		else
		{
			$st2=$db->prepare("INSERT INTO tblIdentity(PublicKey,DateAdded) VALUES(?,?);");
			$st2->bindParam(1,$_REQUEST["publickey"]);
			$st2->bindParam(2,gmdate('Y-m-d H:i:s'));
			$st2->execute();
			$message="Peer added";	
		}
?>
	<h2><?php print($message) ?></h2>
<?php
	}
	else
	{
?>
	<h2>Add Peer</h2>
	<form name="frmaddpeer" method="POST">
	<input type="hidden" name="formaction" value="addpeer">
	Peer Public Key<input type="text" name="publickey" size="100">
	<br>
	The public key must be a valid SSK public key and include the / at the end
	<br>
	<input type="submit" value="Add">
	</form>
<?php
	}
}
	
	include_once('template.php');

?>