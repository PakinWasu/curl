<?php
require 'config.php';

// Query to get the latest record
$sql = "SELECT * FROM tbl_temperature ORDER BY id DESC LIMIT 1";
$result = $db->query($sql);

if (!$result) {
    echo json_encode(["error" => "Query failed: " . $db->error]);
    exit();
}

$row = $result->fetch_assoc();

// Set response type to JSON
header('Content-Type: application/json');

// Output the row as JSON
echo json_encode($row);
?>