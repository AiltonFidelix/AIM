/* 
    Author: Ailton Fidelix 
    Date: 30-08-2022
    Description: AIM device web page script
*/

$(document).ready(function () {
    startSensorInterval();
    getStorageInterval();
    $("#save").on("click", function () {
        setStorageInterval();
    });
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
    setInterval(getSensorValues, 2000);
}

function getStorageInterval() {
    $.getJSON('/getInterval.json', function (data) {
        $("#interval").val(data["interval"]);
    });
}

function setStorageInterval() {
    interval = $("#interval").val();

    $.ajax({
        url: '/setInterval.json',
        dataType: 'json',
        method: 'POST',
        cache: false,
        headers: { 'interval': interval },
        data: { 'timestamp': Date.now() }
    });

    window.alert('Successful save interval!');
}

function getUpdateStatus() {
    var xhr = new XMLHttpRequest();
    var requestURL = "/OTAstatus";
    xhr.open('POST', requestURL, false);
    xhr.send('ota_update_status');

    if (xhr.readyState == 4 && xhr.status == 200) {
        var response = JSON.parse(xhr.responseText);

        document.getElementById("latest_firmware").innerHTML = response.compile_date + " - " + response.compile_time;

        if (response.ota_update_status == 1) {
            seconds = 10;
            otaRebootTimer();
        }
        else if (response.ota_update_status == -1) {
            document.getElementById("ota_update_status").innerHTML = "!!! Upload Error !!!";
        }
    }
}