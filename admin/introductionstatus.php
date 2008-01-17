<?php

	include_once('config.php');
	include_once('linkbar.php');
		
function content()
{
	global $dblocation;
	
	$db=new PDO('sqlite:'.$dblocation);
	$st=$db->prepare("SELECT tblLocalIdentity.Name, COUNT(Inserted) FROM tblLocalIdentity LEFT JOIN tblIdentityIntroductionInserts ON tblLocalIdentity.LocalIdentityID=tblIdentityIntroductionInserts.LocalIdentityID WHERE (Inserted='true' OR Inserted IS NULL) GROUP BY tblLocalIdentity.LocalIdentityID;");
	$st->execute();
	
	?>
	<h2>Introduction Status</h2>
	<table>
		<tr>
			<th>Identity</th>
			<th>Successful introduction inserts</th>
		</tr>
		<?php
		while($record=$st->fetch())
		{
			?>
		<tr>
			<td>
				<?php print $record[0]; ?>
			</td>
			<td>
				<?php print $record[1]; ?>
			</td>
		</tr>
			<?php	
		}
		?>
	</table>
	<?php
}
	
	include_once('template.php');

?>