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

    $("#text_net_ipv4").prop("disabled", true);

    $("#text_net_ipv4").mask('0ZZ.0ZZ.0ZZ.0ZZ', {
        translation: {
            'Z': {
                pattern: /[0-9]/, optional: true
            }
        }
    });;

    $("#btn_save_network").click(function() {
        ssid = $("#text_net_ssid").val();
        ipaddr = $("#text_net_ipv4").val();
        passwd = $("#text_net_passwd").val();

        if (ipaddr.length == 0 || ssid.length == 0 || passwd.length == 0) {
            alert("All fields are required.");
        } else if (passwd.length < 8) {
            alert("Password length must be at least 8 characters long.");
        } else {
            ws.send(SAVE_NETWORKS + ssid + "," + ipaddr + "," + passwd);
            alert("ODROID GO will reboot.");
        }
    });

    $("#btn_reset_network").click(function() {
        $("#text_net_ipv4").val("192.168.4.1").textinput("refresh");
        $("#text_net_ssid").val("ODROID_GO").textinput("refresh");
        $("#text_net_passwd").val("12345678").textinput("refresh");
    });
});
