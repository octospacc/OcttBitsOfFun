<?php http_response_code(500);

if (false
|| $_SERVER['REQUEST_METHOD'] !== 'POST'
|| $_SERVER['HTTP_CONTENT_TYPE'] !== 'image/bmp'
|| !str_starts_with($_SERVER['HTTP_USER_AGENT'], 'LumaBmp2NinJpg/')
) {
	return;
}

$jpgFile = '/tmp/php.' . rand() . microtime();
$jpgQuality = 100;

$imageTmp = imagecreatefrombmp('php://input');
imagejpeg($imageTmp, $jpgFile, $jpgQuality);
imagedestroy($imageTmp);

header('Content-Type: image/jpg');
header('Content-Length: ' . filesize($jpgFile));
http_response_code(200);
fpassthru(fopen($jpgFile, 'rb'));
unlink($jpgFile);
