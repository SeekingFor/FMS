<?php

	include_once('config.php');
	include_once('linkbar.php');
	
function content()
{
	
	global $dblocation;
	
	$db=new PDO('sqlite:'.$dblocation);
	$st=$db->prepare("SELECT IdentityID,Name,LocalMessageTrust,PeerMessageTrust,LocalTrustListTrust,PeerTrustListTrust FROM tblIdentity ORDER BY Name;");
	$st->execute();
	?>
	<h2>Identity Trust</h2>
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
			<td><?php print($record[1]) ?></td>
			<td><?php print($record[2]) ?></td>
			<td><?php print($record[3]) ?></td>
			<td><?php print($record[4]) ?></td>
			<td><?php print($record[5]) ?></td>
		</tr>
		<?php	
		}
		?>
	</table>
	<?php
}
	
	include_once('template.php');

?>