<?php

	include_once('config.php');
	include_once('linkbar.php');
	
function localiddropdown($name)
{
	global $dblocation;
	
	$db=new PDO('sqlite:'.$dblocation);
	
	$st=$db->prepare("SELECT LocalIdentityID, Name FROM tblLocalIdentity ORDER BY Name;");	
	$st->execute();
	
	print "<select name=\"".$name."\">";
	while($record=$st->fetch())
	{
		print "<option value=\"".$record[0]."\">".$record[1]."</option>";
	}
	print "</select>";
}
	
function content()
{
	global $dblocation;
	
	$db=new PDO('sqlite:'.$dblocation);
	
	if(isset($_REQUEST["formaction"]) && $_REQUEST["formaction"]=="announce" && $_REQUEST["localidentityid"]!="")
	{
		$st=$db->prepare("INSERT INTO tblIdentityIntroductionInserts(LocalIdentityID,Day,UUID,Solution) VALUES(?,?,?,?);");
		
		for($i=0; $i<count($_REQUEST["uuid"]); $i++)
		{
			if($_REQUEST["solution"][$i]!="")
			{
				$st->bindParam(1,$_REQUEST["localidentityid"]);
				$st->bindParam(2,$_REQUEST["day"][$i]);
				$st->bindParam(3,$_REQUEST["uuid"][$i]);
				$st->bindParam(4,$_REQUEST["solution"][$i]);
				$st->execute();	
			}
		}
	}
	
	?>
	<h2>Announce Identity</h2>
	<form name="frmannounce" method="POST">
	<input type="hidden" name="formaction" value="announce">
	Select Identity to announce
	<?php
	localiddropdown("localidentityid");
	print "<br>Type answers for a few puzzles and submit<br>";
	
	
	$st=$db->prepare("SELECT UUID,Day FROM tblIntroductionPuzzleRequests WHERE UUID NOT IN (SELECT UUID FROM tblIdentityIntroductionInserts) AND Day>='".gmdate('Y-m-d',strtotime('-1 day'))."' AND Found='true';");
	$st->execute();
	
	while($record=$st->fetch())
	{
		print "<img src=\"showcaptcha.php?UUID=".$record[0]."\">";
		print "<input type=\"hidden\" name=\"uuid[]\" value=\"".$record[0]."\">";
		print "<input type=\"hidden\" name=\"day[]\" value=\"".$record[1]."\">";
		print "<input type=\"text\" name=\"solution[]\">";
		print "<br>";
	}
	?>
	<input type="submit">
	</form>	
	<?php	
}
	
	include_once('template.php');

?>