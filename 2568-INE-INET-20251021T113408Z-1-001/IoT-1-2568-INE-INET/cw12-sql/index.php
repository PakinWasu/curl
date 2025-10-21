<?php
//-------------------------------------------------------------------------------------------
require 'config.php';

$db;
$sql = "SELECT * FROM tbl_temperature ORDER BY id DESC LIMIT 30";
$result = $db->query($sql);
if (!$result) {
  echo "Error: " . $sql . "<br>" . $db->error;
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
  <title>Real Time Weather Station</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.6.1/dist/css/bootstrap.min.css">
  <script src="https://code.jquery.com/jquery-3.1.1.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/popper.js@1.16.1/dist/umd/popper.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@4.6.1/dist/js/bootstrap.bundle.min.js"></script>

  <style>
    .chart {
      width: 100%;
      height: 400px;
    }

    /* Fix for the flex container for two charts */
    .charts-container {
      display: flex;
      justify-content: space-between;
      gap: 20px; /* Add space between charts */
      margin-bottom: 30px;
    }

    .chart-container {
      width: 48%; /* Set a max-width for each chart */
    }

    .row {
      margin: 0 !important;
    }

    .col-md-6 {
      display: flex;
      justify-content: center;
      align-items: center;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="row">
      <div class="col-md-12 text-center">
        <h1>Real Time Weather Station</h1>
        <p>Created By: <a href="#">AhmadLogs</a></p>
      </div>
    </div>

    <!-- Flex container for temperature and humidity gauges -->
    <div class="charts-container">
      <div class="chart-container">
        <div id="chart_temperature" class="chart"></div>
      </div>
      <div class="chart-container">
        <div id="chart_humidity" class="chart"></div>
      </div>
    </div>

    <!-- Flex container for resis and lux gauges -->
    <div class="charts-container">
      <div class="chart-container">
        <div id="chart_resis" class="chart"></div>
      </div>
      <div class="chart-container">
        <div id="chart_lux" class="chart"></div>
      </div>
    </div>

    <div class="row">
      <div class="col-md-12">
        <table class="table">
          <thead>
            <tr>
              <th scope="col">#</th>
              <th scope="col">Temperature</th>
              <th scope="col">Humidity</th>
              <th scope="col">Resis (Ω)</th>
              <th scope="col">Lux (lx)</th>
              <th scope="col">Date Time</th>
            </tr>
          </thead>
          <tbody>
          <?PHP $i = 1; while ($row = mysqli_fetch_assoc($result)) {?>
            <tr>
              <th scope="row"><?php echo $i++;?></th>
              <td><?PHP echo $row['temperature'];?></td>
              <td><?PHP echo $row['humidity'];?></td>
              <td><?PHP echo isset($row['resis']) ? $row['resis'] : '';?></td>
              <td><?PHP echo isset($row['lux']) ? $row['lux'] : '';?></td>
              <td><?PHP echo date("Y-m-d h:i: A", strtotime($row['created_date']));?></td>
            </tr>
          <?PHP } ?>
          </tbody>
        </table>
      </div>
    </div>
  </div>

  <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
  <script>
    google.charts.load('current', {'packages':['gauge']});
    google.charts.setOnLoadCallback(initializeGauges);

    function toPercent(value, maxValue) {
      var v = parseFloat(value);
      if (isNaN(v) || isNaN(maxValue) || maxValue <= 0) return 0;
      var p = (v / maxValue) * 100;
      if (p < 0) p = 0;
      if (p > 100) p = 100;
      return parseFloat(p.toFixed(2));
    }

    var tempChart;
    var tempData;
    var tempOptions;
    function drawTemperatureChart() {
      tempData = google.visualization.arrayToDataTable([
        ['Label', 'Value'],
        ['Temperature', 0],
      ]);

      tempOptions = {
        width: '100%',
        height: 400,
        redFrom: 70, 
        redTo: 100,
        yellowFrom: 40, 
        yellowTo: 70,
        greenFrom: 0, 
        greenTo: 40,
        minorTicks: 5
      };

      tempChart = new google.visualization.Gauge(document.getElementById('chart_temperature'));
      tempChart.draw(tempData, tempOptions);
    }

    var humidityChart;
    var humidityData;
    var humidityOptions;
    function drawHumidityChart() {
      humidityData = google.visualization.arrayToDataTable([
        ['Label', 'Value'],
        ['Humidity', 0],
      ]);

      humidityOptions = {
        width: '100%',
        height: 400,
        redFrom: 80, 
        redTo: 100,
        yellowFrom: 60, 
        yellowTo: 80,
        greenFrom: 0, 
        greenTo: 60,
        minorTicks: 5
      };

      humidityChart = new google.visualization.Gauge(document.getElementById('chart_humidity'));
      humidityChart.draw(humidityData, humidityOptions);
    }

    var resisChart;
    var resisData;
    var resisOptions;
    function drawResisChart() {
      resisData = google.visualization.arrayToDataTable([
        ['Label', 'Value'],
        ['Resis', 0],
      ]);

      resisOptions = {
        width: '100%',
        height: 400,
        redFrom: 80,
        redTo: 100,
        yellowFrom: 60,
        yellowTo: 80,
        greenFrom: 0,
        greenTo: 60,
        minorTicks: 5
      };

      resisChart = new google.visualization.Gauge(document.getElementById('chart_resis'));
      resisChart.draw(resisData, resisOptions);
    }

    var luxChart;
    var luxData;
    var luxOptions;
    function drawLuxChart() {
      luxData = google.visualization.arrayToDataTable([
        ['Label', 'Value'],
        ['Lux', 0],
      ]);

      luxOptions = {
        width: '100%',
        height: 400,
        redFrom: 80,
        redTo: 100,
        yellowFrom: 60,
        yellowTo: 80,
        greenFrom: 0,
        greenTo: 60,
        minorTicks: 5
      };

      luxChart = new google.visualization.Gauge(document.getElementById('chart_lux'));
      luxChart.draw(luxData, luxOptions);
    }

    var refreshIntervalId = null;
    function initializeGauges() {
      drawTemperatureChart();
      drawHumidityChart();
      drawResisChart();
      drawLuxChart();

      function refreshAll() {
        $.ajax({
          url: 'getdata.php',
          dataType: 'json',
          cache: false,
          success: function (responseText) {
            var var_temperature = parseFloat(responseText.temperature);
            if (!isNaN(var_temperature)) {
              tempData.setValue(0, 1, parseFloat(var_temperature.toFixed(2)));
              tempChart.draw(tempData, tempOptions);
            }

            var var_humidity = parseFloat(responseText.humidity);
            if (!isNaN(var_humidity)) {
              humidityData.setValue(0, 1, parseFloat(var_humidity.toFixed(2)));
              humidityChart.draw(humidityData, humidityOptions);
            }

            var raw_resis = parseFloat(responseText.resis);
            if (!isNaN(raw_resis)) {
              var percent_resis = toPercent(raw_resis, 1023);
              resisData.setValue(0, 1, percent_resis);
              resisChart.draw(resisData, resisOptions);
            }

            var raw_lux = parseFloat(responseText.lux);
            if (!isNaN(raw_lux)) {
              var percent_lux = toPercent(raw_lux, 65535);
              luxData.setValue(0, 1, percent_lux);
              luxChart.draw(luxData, luxOptions);
            }
          },
          error: function(jqXHR, textStatus, errorThrown) {
            console.log(errorThrown + ': ' + textStatus);
          }
        });
      }

      if (refreshIntervalId) clearInterval(refreshIntervalId);
      refreshAll();
      refreshIntervalId = setInterval(refreshAll, 1000);
    }

    $(window).resize(function() {
      initializeGauges();
    });
  </script>
</body>
</html>
