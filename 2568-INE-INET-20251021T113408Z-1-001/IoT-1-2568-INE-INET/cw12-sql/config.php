<?php
define('DB_HOST', 'fdb33.awardspace.net');
define('DB_USERNAME', '4684267_pakindht');
define('DB_PASSWORD', '1qazXSW@3edc');
define('DB_NAME', '4684267_pakindht');

define('POST_DATA_URL', 'http://pakinlab.atwebpages.com/sensordata.php');

// PROJECT_API_KEY must match the one in your NodeMCU sketch
define('PROJECT_API_KEY', 'pakinsqldht');

// Set time zone
date_default_timezone_set('Asia/Bangkok');

// Connect to the database
$db = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

// Check connection
if ($db->connect_errno) {
    echo "Connection to database failed: " . $db->connect_error;
    exit();
}
?>