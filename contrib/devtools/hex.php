<?php
function strToHex($string){
    $hex = '';
    for ($i=0; $i<strlen($string); $i++){
        $ord = ord($string[$i]);
        $hexCode = dechex($ord);
        $hex .= substr('0'.$hexCode, -2);
    }
    return strToUpper($hex);
}
function hexToStr($hex){
    $string='';
    for ($i=0; $i < strlen($hex)-1; $i+=2){
        $string .= chr(hexdec($hex[$i].$hex[$i+1]));
    }
    return $string;
}

$t=$argv[1];
$p=$argv[2];
if ($t == "dec") {
echo $p."\n";
echo hexToStr($p)."\n";
} else {
echo $p."\n";
echo strToHex($p)."\n";
}
?>
