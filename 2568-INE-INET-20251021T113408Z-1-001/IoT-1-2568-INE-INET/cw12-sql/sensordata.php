<?php
require 'config.php';

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = escape_data($_POST["api_key"]);

    if ($api_key === PROJECT_API_KEY) {
        $temperature = escape_data($_POST["temperature"]);
        $humidity    = escape_data($_POST["humidity"]);
        $resis       = isset($_POST["resis"]) ? escape_data($_POST["resis"]) : null;
        $lux         = isset($_POST["lux"]) ? escape_data($_POST["lux"]) : null;

        // Prepare and bind (prevents SQL injection)
        $stmt = $db->prepare("INSERT INTO tbl_temperature (temperature, humidity, resis, lux, created_date) VALUES (?, ?, ?, ?, NOW())");
        $stmt->bind_param("dddd", $temperature, $humidity, $resis, $lux);

        if ($stmt->execute()) {
            echo "OK. INSERT ID: " . $stmt->insert_id;
        } else {
            echo "Error: " . $stmt->error;
        }

        $stmt->close();
    } else {
        echo "Wrong API Key";
    }
} else {
    echo "No HTTP POST request found";
}

function escape_data($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
?>