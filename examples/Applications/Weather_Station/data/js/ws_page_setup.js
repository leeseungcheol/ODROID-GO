$(document).bind('mobileinit', function() {
    $.mobile.pushStateEnabled = false;
});

$(function() {
    $('#tf_ipv4').ipAddress();

    $("#sevenseg_temperature").sevenSeg({
        digits: 5,
        decimalPlaces: 2
    });
    $("#sevenseg_pressure").sevenSeg({
        digits: 5,
        decimalPlaces: 1
    });
    $("#sevenseg_humidity").sevenSeg({
        digits: 5,
        decimalPlaces: 2
    });
    $("#sevenseg_altitude").sevenSeg({
        digits: 5,
        decimalPlaces: 1
    });
    $("#sevenseg_uvindex").sevenSeg({
        digits: 5,
        decimalPlaces: 2
    });
    $("#sevenseg_visible").sevenSeg({
        digits: 5,
        decimalPlaces: 0
    });
    $("#sevenseg_ir").sevenSeg({
        digits: 5,
        decimalPlaces: 0
    });

    $("#navbar_home").on("pagebeforeshow", function() {
        pageState = 0;
        if (connected) {
            ws.send(PAGE_STATE + pageState);
        }
    });

    $("#navbar_settings").on("pagebeforeshow", function() {
        pageState = 1;
        if (connected) {
            ws.send(PAGE_STATE + pageState);
        }
    });

    $("#btn_save_network").click(function() {
        ssid = $("#tf_ssid").val();
        ipaddr = $("#tf_ipv4").val();
        passwd = $("#tf_passwd").val();
        ws.send(SAVE_NETWORKS + ssid + "," + ipaddr + "," + passwd);
    });

    $("#btn_reset_network").click(function() {
        $("#tf_ssid").val("ODROID_GO").textinput("refresh");
        $("#tf_ipv4").val("192.168.4.1").textinput("refresh");
        $("#tf_passwd").val("12345678").textinput("refresh");
    });
});