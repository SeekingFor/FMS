<?php

	include_once('config.php');
	include_once('linkbar.php');
	
function content()
{
	
	global $dblocation;
	
	$db=new PDO('sqlite:'.$dblocation);
	
	if(isset($_REQUEST['formaction']) && $_REQUEST['formaction']=='updatetrust')
	{
		$st=$db->prepare("UPDATE tblIdentity SET LocalMessageTrust=?, LocalTrustListTrust=? WHERE IdentityID=?;");
		for($i=0; $i<count($_REQUEST['identityid']); $i++)
		{
			if($_REQUEST['oldlocalmessagetrust'][$i]!=$_REQUEST['newlocalmessagetrust'][$i] || $_REQUEST['oldlocaltrustlisttrust']!=$_REQUEST['newlocaltrustlisttrust'])
			{
				$st->bindParam(1,$_REQUEST['newlocalmessagetrust'][$i]);
				$st->bindParam(2,$_REQUEST['newlocaltrustlisttrust'][$i]);
				$st->bindParam(3,$_REQUEST['identityid'][$i]);
				$rval=$st->execute();
			}
		}
	}
	
	$st=$db->prepare("SELECT IdentityID,Name,LocalMessageTrust,PeerMessageTrust,LocalTrustListTrust,PeerTrustListTrust FROM tblIdentity ORDER BY Name;");
	$st->execute();
	?>
	<h2>Identity Trust</h2>
	<form name="frmtrust" method="post">
	<input type="hidden" name="formaction" value="updatetrust">
	<table>
		<tr>
			<th>Peer</th>
			<th>Local Message Trust</th>
			<th>Peer Message Trust</th>
			<th>Local Trust List Trust</th>
			<th>Peer Trust List Trust</th>
		</tr>
		<?php
		while($record=$st->fetch())
		{
		?>
		<tr>
			<td>
				<?php print($record[1]) ?>
				<input type="hidden" name="identityid[]" value="<?php print($record[0]) ?>">
			</td>
			<td>
				<input type="hidden" name="oldlocalmessagetrust[]" value="<?php print($record[2]) ?>">
				<input type="text" name="newlocalmessagetrust[]" size="2" maxlength="3" value="<?php print($record[2]) ?>">
			</td>
			<td><?php print($record[3]) ?></td>
			<td>
				<input type="hidden" name="oldlocaltrustlisttrust[]" value="<?php print($record[4]) ?>">
				<input type="text" name="newlocaltrustlisttrust[]" size="2" maxlength="3" value="<?php print($record[4]) ?>">
			</td>
			<td><?php print($record[5]) ?></td>
		</tr>
		<?php	
		}
		?>
		<tr>
			<td colspan="5">
				<input type="submit" value="Update Trust">
			</td>
		</tr>
	</table>
	</form>
	<?php
}
	
	include_once('template.php');

?>