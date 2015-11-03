<?php
// Put the same numbers you used when dumping.
$fRunNo = 954;
$fStart = 0;
$fEnd = 0;

//////////////////////////////////////////////////////////////////
//                                                              //
//    Please don't touch below if you don't know about them.    //
//                                                              //
//////////////////////////////////////////////////////////////////

$fRunNo = sprintf("%04d", $fRunNo);
if ($fStart == 0 && $fEnd == 0)
  $fEnd = iterator_count(new FilesystemIterator("run_".$fRunNo, FilesystemIterator::SKIP_DOTS))/2;
?>
<!DOCTYPE HTML5>
<html>
	<head>
		<title><?php echo $fRunNo; ?></title>
		<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css" integrity="sha512-dTfge/zgoMYpP7QbHy4gWMEGsbsdZeCXz7irItjcC3sPUFtf0kuFbDz/ixG7ArTxmDjLXDmezHubeNikyKGVyQ==" crossorigin="anonymous">
		<script>
			function change(number) {
				document.getElementById("top").src="run_<?php echo $fRunNo; ?>/event_" + number + "_top.png";
				document.getElementById("side").src="run_<?php echo $fRunNo; ?>/event_" + number + "_side.png";
			}
		</script>
	</head>
	<body>
	<div class="modal fade" id="myModal">
		<div class="modal-dialog modal-lg" role="document">
			<div class="modal-content">
				<div class="modal-body">
					<img id='top' src='run_<?php echo $fRunNo; ?>/event_0_top.png' width='100%'><br>
					<img id='side' src='run_<?php echo $fRunNo; ?>/event_0_side.png' width='100%'>
				</div>
			</div>
		</div>
	</div>
<?php
for ($iEvent = $fStart; $iEvent < $fEnd; $iEvent++) {
echo "<a data-toggle='modal' data-target='#myModal' onclick='change(".$iEvent.");'>";
echo "<img src='run_".$fRunNo."/event_".$iEvent."_top.png' width='280px' height='169px' /></a>";
}
?>
	<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
	<script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js" integrity="sha512-K1qjQ+NcF2TYO/eI3M6v8EiNYZfA95pQumfvcVrTHtwQVDG+aHRqLi/ETn2uB+1JqwYqVG3LIvdm9lj6imS/pQ==" crossorigin="anonymous"></script>
	</body>
</html>
