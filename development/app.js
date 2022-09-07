/* 
    Author: Ailton Fidelix 
    Date: 30-08-2022
    Description: AIM device web page script
*/

var seconds = null;
var otaTimerVar = null;

$(document).ready(function () {
    startSensorInterval();
    getStorageInterval();
    getUpdateStatus();
    $("#save").on("click", function () {
        checkInterval();
    });
    $("#upload").on("click", function () {
        updateFirmware();
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

function checkInterval() {
    var interval = parseInt($("#interval").val());
    if (interval > 0 && interval <= 60000) {
        setStorageInterval();
    }
    else {
        window.alert('Interval need to be between 1 ~ 60000 seconds.');
    }
}

function updateFirmware() {
    var formData = new FormData();
    var fileSelect = document.getElementById("selected_file");

    if (fileSelect.files && fileSelect.files.length == 1) {
        var file = fileSelect.files[0];
        formData.set("file", file, file.name);
        document.getElementById("update_status").innerHTML = "Uploading " + file.name + ", Firmware Update in Progress...";

        var request = new XMLHttpRequest();

        request.upload.addEventListener("progress", updateProgress);
        request.open('POST', "/OTAupdate");
        request.responseType = "blob";
        request.send(formData);
    }
    else {
        window.alert('Select a file first!')
    }
}

function updateProgress(oEvent) {
    if (oEvent.lengthComputable) {
        getUpdateStatus();
    }
    else {
        window.alert('total size is unknown')
    }
}

function getUpdateStatus() {
    var xhr = new XMLHttpRequest();
    var requestURL = "/OTAstatus";
    xhr.open('POST', requestURL, false);
    xhr.send('ota_update_status');

    if (xhr.readyState == 4 && xhr.status == 200) {
        var response = JSON.parse(xhr.responseText);

        if (response.ota_update_status == 1) {
            seconds = 10;
            otaRebootTimer();
        }
        else if (response.ota_update_status == -1) {
            document.getElementById("update_status").className = "text-danger text-center";
            document.getElementById("update_status").innerHTML = "!!! Upload Error !!!";
        }
    }
}

function otaRebootTimer() {
    document.getElementById("update_status").className = "text-success text-center";
    document.getElementById("update_status").innerHTML = "Firmware Update Complete. This page will close shortly! Rebooting in: " + seconds;

    if (--seconds == 0) {
        clearTimeout(otaTimerVar);
        window.location.reload();
    }
    else {
        otaTimerVar = setTimeout(otaRebootTimer, 1000);
    }
}