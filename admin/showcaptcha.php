<?php

	require_once('config.php');
	
	$db=new PDO('sqlite:'.$dblocation);
	
	if(isset($_REQUEST['UUID']))
	{
		$st=$db->prepare("SELECT MimeType,PuzzleData FROM tblIntroductionPuzzleRequests WHERE UUID=?;");
		$st->bindParam(1,$_REQUEST['UUID']);
		$st->execute();
		
		if($record=$st->fetch())
		{
			$data=base64_decode($record[1]);
			header("Content-type: ".$record[0]);
			header("Content-Length: ".strlen($data));
			print $data;
		}
	}

?>