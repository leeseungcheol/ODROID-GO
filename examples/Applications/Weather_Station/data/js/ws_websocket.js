var SET_DEFAULT_VOLTAGE = 'v';
var SET_VOLTAGE = 'w';
var SET_AUTORUN = 'a';
var SAVE_NETWORKS = 'n';
var MEASUREWATTHOUR = 'm';
var CMD_ONOFF = 'o';
var PAGE_STATE = 'p';
var DATA_PVI = 'd';
var SAVE_SSID = 's';
var SAVE_IPADDR = 'i';
var SAVE_PASSWD = 'x';
var FW_VERSION = 'f';

var wsUriC = "ws://" + ipaddr + ":81";
var ws;
var pageState = 0;
var connected = 0;
var onoff = 1;
var setVolt;
var temperature = 0;
var humidity = 0;
var pressure = 0;
var altitude = 0;
var ssid = 0;
var ipaddr = 0;
var passwd = 0;

function wsConnectC() {
    console.log("WebSocket", wsUriC);

    ws = new WebSocket(wsUriC);
    ws.onmessage = function(msg) {
        var data = msg.data;
        console.log(msg.data);

        switch (data.charAt(0)) {
            case SAVE_NETWORKS:
                networkInfo = data.substring(1).split(",");
                ssid = networkInfo[0];
                ipaddr = networkInfo[1];
                passwd = networkInfo[2];
                $("#text_net_ssid").val(ssid).textinput("refresh");
                $("#text_net_ipv4").val(ipaddr).textinput("refresh");
                $("#text_net_passwd").val(passwd).textinput("refresh");
                break;
            case DATA_PVI:
                dataInfo = data.substring(1).split(",");
                if (temperature != dataInfo[0]) {
                    temperature = dataInfo[0];
                    $("#sevenseg_temperature").sevenSeg({
                        value: dataInfo[0]
                    });
                }
                if (pressure != dataInfo[1]) {
                    pressure = dataInfo[1];
                    $("#sevenseg_pressure").sevenSeg({
                        value: dataInfo[1]
                    });
                }
                if (humidity != dataInfo[2]) {
                    humidity = dataInfo[2];
                    $("#sevenseg_humidity").sevenSeg({
                        value: dataInfo[2]
                    });
                }
                if (altitude != dataInfo[3]) {
                    altitude = dataInfo[3];
                    $("#sevenseg_altitude").sevenSeg({
                        value: altitude
                    });
                }
                if (humidity != dataInfo[4]) {
                    humidity = dataInfo[4];
                    $("#sevenseg_uvindex").sevenSeg({
                        value: dataInfo[4]
                    });
                }
                if (humidity != dataInfo[5]) {
                    humidity = dataInfo[5];
                    $("#sevenseg_visible").sevenSeg({
                        value: dataInfo[5]
                    });
                }
                if (humidity != dataInfo[6]) {
                    humidity = dataInfo[6];
                    $("#sevenseg_ir").sevenSeg({
                        value: dataInfo[6]
                    });
                }
                break;
            case FW_VERSION:
                $("#fwversion").html(data.substring(1));
                break;
        }
    }

    ws.onopen = function() {
        $("#label_connect").html("Connected");
        ws.send(PAGE_STATE + pageState);

        connected = 1;
    }

    ws.onclose = function() {
        $("#label_connect").html("Not Connected");

        connected = 0;
        setTimeout(wsConnectC, 1000);
    }
}