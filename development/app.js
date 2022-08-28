$(document).ready(function () {
    startSensorInterval();
});

function getSensorValues() {
    $.getJSON('/sensorValues.json', function (data) {
        $("#pitch_reading").text(data["pitch"]);
        $("#roll_reading").text(data["roll"]);
        $("#yaw_reading").text(data["yaw"]);
        $("#temperature_reading").text(data["temperature"]);
    });
}

function startSensorInterval() {
    setInterval(getSensorValues, 1000);
}