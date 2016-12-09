<?php
	$fields[0] = "lightStart";
	$fields[1] = $_POST["lightStart"];
	
	$fields[2] = "lightEnd";
	$fields[3] = $_POST["lightEnd"];
	
	$fields[4] = "foggerStart";
	$fields[5] = $_POST["foggerStart"];
	
	$fields[6] = "foggerEnd";
	$fields[7] = $_POST["foggerEnd"];
	
	$fields[8] = "plantTempMin";
	$fields[9] = $_POST["plantTempMin"];
	
	$fields[10] = "plantTempMax";
	$fields[11] = $_POST["plantTempMax"];
	
	$fields[12] = "rootTempMin";
	$fields[13] = $_POST["rootTempMin"];
	
	$fields[14] = "rootTempMax";
	$fields[15] = $_POST["rootTempMax"];
	
	$fields[16] = "pHMin";
	$fields[17] = $_POST["pHMin"];
	
	$fields[18] = "pHMax";
	$fields[19] = $_POST["pHMax"];
	
	$file = fopen("botny.csv","w");
	fputcsv($file,$fields);
	fclose($file);
	
	echo "Data submitted";
?>
