<?php

	include_once('config.php');
	
	if(isset($_REQUEST['formaction']) && $_REQUEST['formaction']=='saveoptions')
	{
		$db=new PDO('sqlite:'.$dblocation);
		$st=$db->prepare("UPDATE tblOption SET OptionValue=? WHERE Option=?;");
		foreach($_REQUEST as $key=>$val)
		{
			$st->bindParam(1,$val);
			$st->bindParam(2,$key);
			$st->execute();
		}
	}

function content()
{
	global $dblocation;
	$db=new PDO('sqlite:'.$dblocation);
	
	$rs=$db->query("SELECT Option, OptionValue, OptionDescription FROM tblOption;");
	
?>
	<h2 style="text-align:center;">Options</h2>
	<form name="frmoptions" method="post">
	<input type="hidden" name="formaction" value="saveoptions">
	<table>
		<tr>
			<th>Option</th>
			<th>Value</th>
			<th>Description</th>
		</tr>
		<?php
		while($record=$rs->fetch())
		{
			print '<tr>';
			print '<td valign="TOP">'.$record[0].'</td>';
			print '<td valign="TOP"><input type="text" name="'.$record[0].'" value="'.$record[1].'"></td>';
			print '<td valign="TOP">'.$record[2].'</td>';
			print '</tr>';
		}
		?>
		<tr>
			<td colspan="3">
				<center>
				<input type="submit" value="Save">
				</center>
			</td>
		</tr>
	</table>
<?php
}

	include_once('linkbar.php');
	include_once('template.php');

?>