<?php
	
	include_once('config.php');
	include_once('linkbar.php');

function truefalsedropdown($name,$currentval)
{
	?>
	<select name="<?php print($name); ?>">
		<option value="true" <?php if($currentval=="true") print("SELECTED"); ?>>true</option>
		<option value="false" <?php if($currentval=="false") print("SELECTED"); ?>>false</option>
	</select>
	<?php
}

function content()
{
	global $dblocation;
	$db=new PDO('sqlite:'.$dblocation);
	
	if(isset($_REQUEST['formaction']))
	{
		if($_REQUEST['formaction']=='update' && isset($_REQUEST['update']))
		{
			$st=$db->prepare("UPDATE tblLocalIdentity SET SingleUse=?, PublishTrustList=? WHERE LocalIdentityID=?;");
			for($i=0; $i<count($_REQUEST['update']); $i++)
			{
				if($_REQUEST['update'][$i]!='')
				{
					$st->bindParam(1,$_REQUEST['singleuse'][$_REQUEST['update'][$i]]);
					$st->bindParam(2,$_REQUEST['publishtrustlist'][$_REQUEST['update'][$i]]);
					$st->bindParam(3,$_REQUEST['localidentityid'][$_REQUEST['update'][$i]]);
					$st->execute();
				}
			}
		}
		if($_REQUEST['formaction']=='delete' && isset($_REQUEST['update']))
		{
			$st=$db->prepare("DELETE FROM tblLocalIdentity WHERE LocalIdentityID=?;");

			for($i=0; $i<count($_REQUEST['update']); $i++)
			{
				if($_REQUEST['update'][$i]!='')
				{
					$st->bindParam(1,$_REQUEST['localidentityid'][$_REQUEST['update'][$i]]);
					$st->execute();
				}				
			}			
		}
	}
	
	$st=$db->prepare("SELECT LocalIdentityID,Name,PublicKey,PublishTrustList,SingleUse,PublishBoardList FROM tblLocalIdentity ORDER BY Name;");
	$st->execute();
	?>
	<h2>Local Identities</h2>
	<form name="frmlocalidentity" method="post">
	<input type="hidden" name="formaction" value="update">
	<table>
		<tr>
			<td></td>
			<th>Name</th>
			<th>Single Use</th>
			<th>Publish Trust List</th>
		</tr>
	<?php
	$row=0;
	while($record=$st->fetch())
	{
		?>
		<tr>
			<td>
				<input type="checkbox" name="update[]" value="<?php print($row); ?>">
			</td>
			<td title="<?php print($record[2]); ?>">
				<input type="hidden" name="localidentityid[]" value="<?php print($record[0]); ?>">
				<?php print($record[1]); ?>
			</td>
			<td>
				<?php
				truefalsedropdown("singleuse[]",$record[4]);
				?>
			</td>
			<td>
				<?php
				truefalsedropdown("publishtrustlist[]",$record[3]);
				?>
			</td>
		</tr>
		<?php
		$row++;
	}
	?>
	</table>
	<input type="submit" value="Update Selected">
	<input type="submit" value="Delete Selected" onClick="if(confirm('Delete Selected Identities?')){frmlocalidentity.formaction.value='delete';}else{return false;}">
	</form>
	<?php
}

	include_once('template.php');

?>