
<?php require 'config.php'; ?>
<!DOCTYPE html>
<html>
<head>
    <title>AhmadLogs - Test POST Data</title>
</head>
<body>
    <h2>AhmadLogs - Test POST Data</h2>

    <form method="POST" action="<?php echo POST_DATA_URL; ?>">
        <label for="api_key">API Key:</label><br>
        <input type="text" id="api_key" name="api_key" value="<?php echo PROJECT_API_KEY; ?>"><br><br>

        <label for="temperature">Temperature (°C):</label><br>
        <input type="text" id="temperature" name="temperature" value="16.53"><br><br>

        <label for="humidity">Humidity (%):</label><br>
        <input type="text" id="humidity" name="humidity" value="55.67"><br><br>

        <label for="resis">Resis (Ω):</label><br>
        <input type="text" id="resis" name="resis" value="1000"><br><br>

        <label for="lux">Lux (lx):</label><br>
        <input type="text" id="lux" name="lux" value="250"><br><br>

        <input type="submit" value="Submit">
    </form>
</body>
</html>