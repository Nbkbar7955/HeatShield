var xml;
var ports = [];
var screenItems = [];

var currentBreakIndex = 1;
var chartInit = false;
var output;
var $xmlDoc;
var useDebugger = false;
var maxDisplayChartItems = 40;    // Updated to 40 as more useful
var maxDisplaySerialItems = 1000; // Ones visible in Serial Monitor

var isNoPref = window.matchMedia("(prefers-color-scheme: no-preference)");
var isLightScheme = window.matchMedia("(prefers-color-scheme: light)");
var isDarkScheme = window.matchMedia("(prefers-color-scheme: dark)");

var dev_useMuuri = false;
var dev_collapseCardsByDefault = false;

var vmboard = { totalPins: 20, totalAnalogPins: 6 };

class SerialPort {
    constructor(Address, COMPort, Name) {
        this.PORT = COMPort;
        this.ADDRESS = Address;
        this.NAME = Name;
        this.WEBSOCKET = undefined;
    }
}

class tracking {
    constructor() {
        this.min = 0;
        this.max = 0;
        this.lastUpdated = 0;
    }
    dataUpdated(dataValue) {
        if (this.min > dataValue) { this.min = dataValue; }
        if (this.max < dataValue) { this.max = dataValue; }
        this.lastUpdated = (new Date).valueOf();
    }
}

class screenItem {
    constructor(itemName, itemType, parentElementId, className) {


        this.itemName = itemName;
        this.itemType = itemType;
        this.elemId = "vms_" + itemName.replace(/ /g, "_"); // Strip whitespace
        this.elemId = this.elemId.replace(/\W*/g, "")     // Remove any non-basic chars
        this.data = [[]]; // 0 is raw data for normal tiles, "name" is series for multi tracked items
        // Can be backed onto LS and Exported as CSV etc
        this.tracking = [];
        this.tracking[0] = new tracking();
        
        var chartType = itemType.replace("Chart", "");


        var displayScaleLabels = true;
        var displayGridLines = true;
        var displayTicks = true;
        var gague = false;
        switch (itemType) {
            case "doughnut":
            case "gagueChart":
                gague = true;
                chartType = "doughnut";

            case "pieChart":
                displayGridLines = false;
                displayScaleLabels = false;
                displayTicks = false;
            case "barChart":
            case "lineChart":

                var chartTileOuter = document.createElement("div");
                chartTileOuter.className = "card";

                var chartTile = document.createElement("div");
                chartTile.className = ui_card_getClasses(); //className;
                chartTile.id = this.elemId + "_cardBody";
                var chartTileTitle = document.createElement("div");
                var title = document.createElement("span");
                chartTileTitle.className = "card-header";
                title.className = "vmCardHeading";
                title.innerHTML = itemName;
                chartTileTitle.appendChild(title);
                
                chartTileTitle.appendChild(create_downloadButton(this.elemId));
                chartTileTitle.appendChild(create_collapseButton(this.elemId));
                //chartTile.style = "display: inline-block;"
            
                var canv = document.createElement("canvas");
                canv.id = this.elemId;
              // canv.setAttribute("width", 300);
              //  canv.setAttribute("height", 200);
                chartTile.appendChild(canv);
                chartTileOuter.appendChild(chartTileTitle);
                chartTileOuter.appendChild(chartTile);
                
                document.getElementById(parentElementId).appendChild(ui_WrapTile(chartTileOuter, className));
                
                // labels on x
                // data in data -- correlates easy
                // Colours if needed too
                var chartDat = {
                    type: chartType,
                    data: {
                        labels: [],
                        datasets: []
                    },
                    responsive: true,
                    options: {
                        
                        title: {
                            display: false, /* Now in Frame Title */
                            text: itemName
                        },
                        scales: {
                            xAxes: [{
                                scaleLabel: {
                                    display: displayScaleLabels
                                },
                                gridLines: {
                                    display: displayGridLines,
                                    drawBorder: displayGridLines,
                                    drawOnChartArea: displayGridLines
                                },
                                ticks: { display: displayTicks }
                            }],
                            yAxes: [{
                                scaleLabel: {
                                    display: displayScaleLabels
                                },
                                gridLines: {
                                    display: displayGridLines,
                                    drawBorder: displayGridLines,
                                    drawOnChartArea: displayGridLines
                                },
                                ticks: { display: displayTicks }
                            }]
                        }
                    }
                };

                if (gague) {
                    chartDat.options.rotation = -1.0 * Math.PI;
                    chartDat.options.circumference = Math.PI;
                }
                this.chartObj = new Chart(canv.getContext('2d'), chartDat);
                break;

            case "debugExpressions":
                // Refactor into table functions - so easier entry points for users
                var debugTable = document.createElement("table");
                debugTable.id = "debugExpr_table";
                debugTable.style.width = "100%";
                //debugTable.style.display = "none"; // Hide until data back
                var header = document.createElement("thead");
                var headerRow = document.createElement("tr");
                var headerCol = document.createElement("th");
                headerCol.innerHTML = "Name";
                headerRow.appendChild(headerCol);
                var headerVal = document.createElement("th");
                headerVal.innerHTML = "Value";
                headerRow.appendChild(headerVal);
                var headerAt = document.createElement("th");
                headerAt.innerHTML = "At";
                headerRow.appendChild(headerAt);

                header.appendChild(headerRow);


                var body = document.createElement("tbody");

                var breaks = $xmlDoc.find("BreakPoint");
                for (var b = 0; b < breaks.length; b++) {
                    var msgVars = $(breaks[b]).find("MessageVariables");

                    for (var v = 0; v < msgVars.find("MessageVariable").length; v++) {
                        var msgVariable = $(msgVars.find("MessageVariable")[v]);

                        var bodyRow = document.createElement("tr");
                        bodyRow.id = "dbgRowIdx_" + $(breaks[b]).attr("Index") + "_" + msgVariable.attr("Index");
                        bodyRow.className = "dbgRow";
                        var bodyName = document.createElement("td");
                        bodyName.innerHTML = msgVariable.attr("Name");
                        bodyRow.appendChild(bodyName);

                        var bodyCol = document.createElement("td");
                        var bodyDat = document.createElement("span");
                        if (msgVariable.attr("CanWriteVar") == "1") {
                            var editable = document.createElement("input");
                            editable.id = "dbgValIdx_edit_" + $(breaks[b]).attr("Index") + "_" + msgVariable.attr("Index");
                            editable.onblur = function () { sendDebugDataValue(this, 0) };
                            bodyCol.append(editable);
                        } else {
                            bodyDat.id = "dbgValIdx_" + $(breaks[b]).attr("Index") + "_" + msgVariable.attr("Index");
                            bodyDat.innerHTML = " - ";
                            bodyCol.appendChild(bodyDat);
                        }
                        bodyRow.appendChild(bodyCol);

                        var bodyAt = document.createElement("td");
                        bodyAt.innerHTML = $(breaks[b]).attr("Name");

                        bodyRow.appendChild(bodyAt);
                        body.appendChild(bodyRow);
                    }
                }
                debugTable.appendChild(header);
                debugTable.appendChild(body);
                try {
                    document.getElementById("debugExpr").appendChild(debugTable);
                } catch { }
                break;

            case "slider":               
                var chartTile = createBaseTile("card bg-light mb-3 slidecontainer", "18rem", this.elemId); // "item-content"

                var chartTileBody = document.createElement("div");
                chartTileBody.className = ui_card_getClasses();
                chartTileBody.id = this.elemId + "_cardBody";   

                var e = document.createElement("input");
                e.id = this.elemId;
                // Some Basic Defaults for now
                e.setAttribute("type", "range");
                e.setAttribute("min", "1");
                e.setAttribute("max", "100");
                e.setAttribute("value", "1");
                e.className = "slider";
                chartTileBody.appendChild(e);

                var l = document.createElement("span");
                l.id = this.elemId + "_value";
                l.innerHTML = "..."; // Waiting for data
                chartTileBody.appendChild(l);

                chartTile.appendChild(chartTileBody);

                try {
                    document.getElementById(parentElementId).appendChild(ui_WrapTile(chartTile, className));
                } catch { }

              //  addItemToGrid(parentElementId, this.elemId);

                break;

            case "tile":
                var chartTile = createBaseTile("card bg-light mb-3 slidecontainer", "18rem", this.elemId);
                var chartTileBody = document.createElement("div");
                chartTileBody.className = ui_card_getClasses();
                chartTileBody.id = this.elemId + "_cardBody";
                var v = document.createElement("p");
                v.id = this.elemId + "_value";
                v.innerHTML = "..."; // Waiting for data
                chartTileBody.appendChild(v);
                chartTile.appendChild(chartTileBody);
                try {
                    document.getElementById(parentElementId).appendChild(ui_WrapTile(chartTile, className));
                } catch {
                    
                }
                addItemToGrid(parentElementId, this.elemId);
                break;


            case "xyztile":
                var chartTile = createBaseTile("card bg-light mb-3 slidecontainer", "18rem", this.elemId);
                var chartTileBody = document.createElement("div");
                chartTileBody.className = ui_card_getClasses();
                chartTileBody.id = this.elemId + "_cardBody";
                var tilexyz = document.createElement("div");
                tilexyz.id = this.elemId + "_transform";
                tilexyz.style.width = "100px";
                tilexyz.style.height = "120px";
                tilexyz.className = "cubespinner";
                // Create the cube
                for (var f = 1; f < 7; f++) {
                    var face = document.createElement("div");
                    face.className = "face" + f;
                    face.style.backgroundImage = ("images/face" + f + ".png");
                    var faceimg = document.createElement("img");
                    faceimg.setAttribute("src", ("images/face" + f + ".png"));
                    face.appendChild(faceimg);
                    tilexyz.appendChild(face);
                }
                chartTileBody.appendChild(tilexyz);
                var v = document.createElement("p");
                v.id = this.elemId + "_value";

                var vr = document.createElement("span");
                vr.id = this.elemId + "_x_value";
                vr.setAttribute("style", "padding-right:5px;");
                v.appendChild(vr);
                var vg = document.createElement("span");
                vg.id = this.elemId + "_y_value";
                vg.setAttribute("style", "padding-right:5px;");
                v.appendChild(vg);
                var vb = document.createElement("span");
                vb.id = this.elemId + "_z_value";
                v.appendChild(vb);
   
                chartTileBody.appendChild(v);
                chartTile.appendChild(chartTileBody);
                try {
                    document.getElementById(parentElementId).appendChild(ui_WrapTile(chartTile, className));
                } catch { }
                addItemToGrid(parentElementId, this.elemId);
                break;

            case "rgbtile":
                var chartTile = createBaseTile("card bg-light mb-3 slidecontainer", "18rem", this.elemId);
                var chartTileBody = document.createElement("div");
                chartTileBody.className = ui_card_getClasses();
                chartTileBody.id = this.elemId + "_cardBody";
                var tileColor = document.createElement("div");
                tileColor.id = this.elemId + "_color";
                tileColor.style.width = "20%";
                tileColor.style.height = "40px";
                tileColor.style.backgroundColor = "rgba(0, 0, 0, 1)";
                tileColor.style.display = "inline-block";
                tileColor.innerHTML = "&nbsp;";
                chartTileBody.appendChild(tileColor);

                var v = document.createElement("p");
                v.id = this.elemId + "_value";

                var vr = document.createElement("span");
                vr.id = this.elemId + "_r_value";
                vr.setAttribute("style","padding-right:5px;");
                v.appendChild(vr);
                var vg = document.createElement("span");
                vg.id = this.elemId + "_g_value";
                vg.setAttribute("style", "padding-right:5px;");
                v.appendChild(vg);
                var vb = document.createElement("span");
                vb.id = this.elemId + "_b_value";
                vb.setAttribute("style", "padding-right:5px;");
                v.appendChild(vb);
                var va = document.createElement("span");
                va.id = this.elemId + "_a_value";
                v.appendChild(va);
                chartTileBody.appendChild(v);
                chartTile.appendChild(chartTileBody);
                try {
                    
              
                    document.getElementById(parentElementId).appendChild(ui_WrapTile(chartTile, className));
                } catch { }
                addItemToGrid(parentElementId, this.elemId);
                break;

            case "panel":
                var panelId = "vms_" + itemName.replace(/ /g, "_");
                var panelNavId = panelId + "_Nav";
                var panelNavTarget = panelId + "_Target";
                var panelTabLi = document.createElement("li");
                panelTabLi.className = "nav-item";

                var panelTab = document.createElement("a");
                panelTab.className = "nav-link";
                panelTab.id = panelNavId;
                panelTab.setAttribute("panel", panelId);
                panelTab.setAttribute("data-toggle", "pill")
                panelTab.setAttribute("href", "#" + panelNavTarget)
                panelTab.setAttribute("role", "tab")
                panelTab.setAttribute("aria-controls", panelNavTarget)
                panelTab.setAttribute("aria-selected","false")
                panelTab.innerHTML = itemName;

                panelTabLi.appendChild(panelTab);

                document.getElementById("v-pills-tab").appendChild(panelTabLi);

                // Content Tab
                var panelTabWrap = document.createElement("div");
                panelTabWrap.id = panelNavTarget;
                panelTabWrap.className = "tab-pane container fade";
                panelTabWrap.setAttribute("role", "tabpanel")
                panelTabWrap.setAttribute("aria-labelledby", panelNavId)

                // Inner Content Tab
                var panelTabRow = document.createElement("div");
                panelTabRow.id = panelId;
                panelTabRow.className = "row";
                panelTabWrap.appendChild(panelTabRow);

                // TODO: Add Rows into Panels = check MUURI First to see what Grid Sytem it uses as may be a repacement
                try {
                    document.getElementById("userPanels").appendChild(panelTabWrap);
                } catch { }

              

                break;

            default:	// Tile
                // "<div id="" class="one-column"></div>
                break;
        }
        
    }
}



function init() {
    grabThemeColours();



    // Handle Browser Theming
    handleDarkBrowserScheme(isDarkScheme); // Call listener function at run time
    handleLightBrowserScheme(isLightScheme);
    handleLightBrowserScheme(isNoPref);

    // Listen out for the theme changing while we are loaded too
    isLightScheme.addListener(handleLightBrowserScheme); // Attach listener function on state changes
    isNoPref.addListener(handleLightBrowserScheme);
    isDarkScheme.addListener(handleDarkBrowserScheme);
    

    $xmlDoc = $($($("#vmProgramData").html())[0]); // Load in Debug Data if present
    var wsUrl = "ws://" + $xmlDoc.find("Board").attr("vmwebdebug.webserver.address") + ":" + $xmlDoc.find("Board").attr("vmwebdebug.websocket.port");
    if ($xmlDoc.find("Board").attr("vmwebdebug.websocket.remote.address")) {
        wsUrl = "ws://" + $xmlDoc.find("Board").attr("vmwebdebug.websocket.remote.address")
        if ($xmlDoc.find("Board").attr("vmwebdebug.websocket.remote.port")) {
            wsUrl += ":" + $xmlDoc.find("Board").attr("vmwebdebug.websocket.remote.port");
        }
    }

    
    var tp = $xmlDoc.find("Board").attr("vmboard.totalpins");
    if (tp) {
        vmboard.totalPins = tp;
    }
    var tap = $xmlDoc.find("Board").attr("vmboard.totalanalogpins");
    if (tap) {
        vmboard.totalAnalogPins = tap;
    }

    var maxDisp = $xmlDoc.find("Board").attr("vmwebdebug.ui.chartDisplayMax");
    if (maxDisp) {
        maxDisplayChartItems = maxDisp;
    }

    // Populate Internal COM Struct
    addPort(wsUrl, $xmlDoc.find("Board").attr("serial.port.file"), $xmlDoc.find("Board").attr("serial.Caption"));
    // Create Screen Elements and Bindings
    document.getElementById("SerialPort_0_Address").value = wsUrl;
    document.getElementById("SerialPort_0_COM").value = $xmlDoc.find("Board").attr("serial.port");
    document.getElementById("SerialPort_0_Name").value = $xmlDoc.find("Board").attr("serial.Caption");
    document.getElementById("SerialPort_0_Apply").value = $xmlDoc.find("Board").attr("serial.Caption");

    // Connect to socket using its portArrayId - always 0 for multi-instance
    connectWebSocket(0);

    // Review Debugger Information
    if ($xmlDoc.find("BreakPoint").length > 0) {

        useDebugger = true;

        /*
        $xmlDoc.find("Program").attr("name"); 
        $xmlDoc.find("Program").attr("Ext");
        $xmlDoc.find("Board").attr("Description");
        $xmlDoc.find("Board").attr("serial.port.file");
        */

        document.getElementById("comPort").innerHTML = $xmlDoc.find("Board").attr("serial.Caption");

        // Debugger
        /* screenItems.push((new screenItem("myChart", "lineChart", "debugCharts", "")));*/
        screenItems.push((new screenItem("debugExpr", "debugExpressions", "debugCharts", "")));

        output = document.getElementById("output");
        document.getElementById("send").onclick = function () { doSend(0); };
        document.getElementById("step").onclick = function () { doStep(0); };

    } else {
        /* TODO: Load in any storage locally or via config file (append JSON or XML doc as before on build) here */
        $('#userPanel').html("Waiting for &quot;@vm_&quot; triggers from board...");
    /* All other user actions are triggered from board events */
        $('#debugPanel').hide();
        $('#debugPanelbtn').hide();
    }


    
}

// Add Event Handlers to Websockets
function connectWebSocket(portId) {
    ports[portId].WEBSOCKET = new WebSocket(ports[portId].ADDRESS);
    ports[portId].WEBSOCKET.onopen = function (evt) { onOpen(evt) };
    ports[portId].WEBSOCKET.onclose = function (evt) { onClose(evt) };
    ports[portId].WEBSOCKET.onmessage = function (evt) { onMessage(evt) };
    ports[portId].WEBSOCKET.onerror = function (evt) { onError(evt) };
}

function onOpen(evt) {
    if (useDebugger) {
        writeToScreen("CONNECTED", "green");
    }
}

function onClose(evt) {
    if (useDebugger) {
        writeToScreen("DISCONNECTED", "red");
    }
}

var SereBuf = "";
var dataFixed = false;

function onMessage(evt) {
    var userData = evt.data;

    if (useDebugger) {
        // Catch Split messages... need to look @ Why these occurr.....
        if (userData.substring(0, 3) == "VMD" && (evt.data).indexOf("_VMD") == -1) {
            SereBuf += userData;
            dataFixed = false;
            return;// Wait for next half.....
        } else
            if (userData.substring(0, 3) != "VMD" && (evt.data).indexOf("_VMD") != -1) {
            userData = SereBuf + userData;
            SereBuf = "";
            dataFixed = true;
        }


        if (userData.substring(0, 6) == "VMDPC_") {
            userData = userData.replace(userData.substring(userData.indexOf("VMDPC_"), userData.indexOf("_VMDPC")) + "_VMDPC", "");
            writeToScreen('Sys: Debug Continue', "green");
            userData = "";
        } else
            if (userData.substring(0, 6) == "VMDPV_") {
                userData = userData.replace(userData.substring(userData.indexOf("VMDPV_"), userData.indexOf("_VMDPV")) + "_VMDPV", "");
                writeToScreen("Sys: Debug Started", "green");
                userData = "";
            } else
                if (userData.substring(0, 6) == "VMDPE_") {
                    userData = userData.replace(userData.substring(userData.indexOf("VMDPE_"), userData.indexOf("_VMDPE")) + "_VMDPE", "");
                    // VMDPE_1:2:169065:0:9999|34_VMDPE  -- 34 is the value, 169065 is uptime
                    // [VMDPE_1:2:107528:98517:9011|2|0_VMDPE] -- here we are tracing 2 vars so split by | into array of watches
                   
                    var allWatches = (evt.data).substring(0, (evt.data).length - 7).split("|");
                    currentBreakIndex = +((evt.data).split(":")[0].split("_")[1]);

                    // Reset all Debug Rows' colouring....
                    $(".dbgRow").css("font-weight", "normal");
                    $(".dbgRow").css("background-color", "");

                    var breakpoint = $xmlDoc.find("Breakpoint[index=" + currentBreakIndex + "]");
                    if ($(breakpoint).attr("BreakWhenHit") == 1) {
                        $("#step").removeClass("btn-outline-success");  // Only if we can stop at it!
                        $("#step").addClass("btn-success");  // Only if we can stop at it!

                        if ("<pre  class=\"prettyprint lang-cc\">" + breakpoint[0].innerText.trimStart() + "</pre>" != $("#debugSrc").html()) {
                            $("#debugSrc").html("<pre  class=\"prettyprint lang-cc\">" + breakpoint[0].innerText.trimStart() + "</pre>");			// Which is when its' Attribute is BreakWhenHit
                            $("#debugSrcCard").show();
                        }
                    }
                     
                    var plotWinName = "DebugWin";
                    var plotSeriesName = "Series1";
                    var plotValName = "Val";

                    // May want other options to show/hide debug / expressions windows (put on bar @ Top?)
                    var MessageVariables = $(breakpoint).find("MessageVariable");
                    for (var mv = 0; mv < MessageVariables.length; mv++) {
                        var MessageString = $($(MessageVariables)[mv]).attr("Name");
                        if (MessageString.substring(0,5).toLowerCase() == "@plot") {
							var data = (evt.data).substring(0, (evt.data).length - 7).split("|")[mv+1]; // Just Plotting First Value Here....



                            // Split it up!
                            plotWinName = MessageString.split(".")[1];
                            plotSeriesName = MessageString.split(".")[2].split(" ")[0];
                            plotValName = MessageString.split(" ")[1];
                            // Add own Tab for each Debug Chart
                            createPanelIfNotExists("vmPlots", "PlotCharts");
                            var screenIdx = addScreenItem(plotWinName, "vmPlots", "line", "col-md-12");
                            addSeriesValue(screenIdx, plotSeriesName, (new Date).toLocaleTimeString(), data);
                            addLineChartItem(plotSeriesName, screenIdx, (new Date).toLocaleTimeString(), data);
                            screenItems[screenIdx].chartObj.update();  
                        }
                    }
                                                           
                    //BreakPoint Table Manipulation
                    // Change Debug Table Row to suit
                    var uptime = (evt.data).split(":")[2];
                    for (var w = 1; w < allWatches.length; w++) {
                        $("#debugExpr").show();
                        var data = (evt.data).substring(0, (evt.data).length - 7).split("|")[w];
			            // Need to know if can Write (check Elem Type)
			            // < !--$(breakpoint).attr("BreakWhenHit") CanWriteVar-- >
			            try {
                            if (document.activeElement.id != "dbgValIdx_edit_" + currentBreakIndex + "_" + w) {
                                document.getElementById("dbgValIdx_edit_" + currentBreakIndex + "_" + w).value = data;
                            }
                            //$("#dbgValIdx_edit_" + currentBreakIndex + "_" + w).attr("readonly","");
                        } catch { }
                        try {
                            document.getElementById("dbgValIdx_" + currentBreakIndex + "_" + w).innerHTML = data;
                        } catch { }
                        $("#dbgRowIdx_" + currentBreakIndex + "_" + w).css("font-weight", "bold");
                        $("#dbgRowIdx_" + currentBreakIndex + "_" + w).css("background-color", "lightblue");
                        //$("#dbgRowIdx_" + currentBreakIndex + "_" + w + ">input").attr("readonly","");
                        //console.log('Sys: Debug Data ['+evt.data+']');
                    }
                    userData = "";
                } else if (userData.substring(0, 6) == "VMDPR_") {
                    userData = userData.replace("VMDPR_", "").replace("_VMDPR", "");

                    // Show Panel for Digital Pins
                    var hasDigiData = false;
                    var hasAnalaogData = false;
                    var analogData = userData.split("|")[2];                   
                    userData = userData.split("|")[1]; // Digital
                    

                    // Now we can extract the bytes' of pins
                    var bytes = userData.split(":");
                    for (var b = 0; b < bytes.length; b++)
                    {
                        if (bytes[b] != "") {
                            var n = (bytes[b] >>> 0).toString(2); // Get in Binary
                            n = "00000000".substr(n.length) + n; // Pad
                            bytes[b] = n.split("").reverse().join(""); // Reverses Binary order...
                            // Now we can iterate these bytes in order  to work out pins
                           
                            // Need to work out the Rows as well so wraps nicely for responsive etc....
                            // Below just dumps into single row but looks odd.......
                            

                            for (var tb = 0; tb < bytes[b].length; tb++) { 
                                var pinNum = ((b * 8) + tb);
                                if (pinNum < vmboard.totalPins) {
                                    var pinRowId = "digitalPins_r" + (pinNum - (pinNum % 6)) / 6;
                                    if ($("#" + pinRowId).length == 0) {
                                        var pinRow = document.createElement("div");
                                        pinRow.id = pinRowId;
                                        pinRow.className = "row digiPins";
                                        document.getElementById("digitalPins").appendChild(pinRow);
                                        $('#digitalPinsCard').show();
                                    }

                                    if ($("#digitalPins_" + pinNum).length == 0) {
                                        var pinTile = document.createElement("div");
                                        pinTile.id = "digitalPins_" + pinNum;
                                        pinTile.className = "col-md-2 btn digiPin";
                                        pinTile.style.textAlign = "center";
                                        document.getElementById(pinRowId).appendChild(pinTile);
                                    }

                                    if (bytes[b][tb] == "1") {
                                        $("#digitalPins_" + pinNum).html(pinNum + ": ON  ");
                                        $("#digitalPins_" + pinNum).removeClass("bg-light");
                                        $("#digitalPins_" + pinNum).addClass("bg-success");                                        
                                    } else {
                                        $("#digitalPins_" + pinNum).html(pinNum + ": OFF");
                                        $("#digitalPins_" + pinNum).removeClass("bg-success");
                                        $("#digitalPins_" + pinNum).addClass("bg-light");
                                    }
                                    hasDigiData = true;
                                }
                            }
                        }
                    }
                    if (hasDigiData) {
                        $(".digitalPinItems").show();
                    }
                    // We can also extract the Analog Pins
                    // Now we can extract the bytes' of pins
                    var bytes = analogData.split(":");
                    for (var b = 0; b < bytes.length; b++) {
                        if (bytes[b] != "") {
                            var pinNum = b;
                            var pinRowId = "analogPins_r" + (pinNum - (pinNum % 6)) / 6;
                            if ($("#" + pinRowId).length == 0) {
                                var pinRow = document.createElement("div");
                                pinRow.id = pinRowId;
                                pinRow.className = "row";
                                document.getElementById("analogPins").appendChild(pinRow);
                                $('#analogPinsCard').show();
                            }
                            if ($("#analogPins_" + pinNum).length == 0) {
                                var pinTile = document.createElement("div");
                                pinTile.id = "analogPins_" + pinNum;
                                pinTile.className = "col-md-2 btn analogPin";
                                pinTile.style.textAlign = "center";
                                document.getElementById(pinRowId).appendChild(pinTile);
                            }

                            $("#analogPins_" + pinNum).html("A" + pinNum + " : " + bytes[b]);
                            console.log("Pin A" + pinNum + " : " + bytes[b]);
                            // Cant Color Analog Pins really without knowing Maximums -- Could plot on Chart instead?
                            //$("#digitalPins_" + pinNum).css("background", "linear-gradient(to bottom, #33ccff 0%, #ff99cc 100%);")        
                            $("#analogPins_" + pinNum).addClass("bg-light");
                            hasAnalaogData = true;
                        }
                    }
                    if (hasAnalaogData) {
                        $(".analogPinItems").show();
                    }
                    userData = "";
                } else {
                    userData = evt.data;
                }
    }


    // Also Listen for @vm_ commands (@vm_chart_line|........|vm_chart_line@)
    // Expect it to be on one line as dealt with in lib
    // Will have assistive Lib for Arduino to go with it to reduce all text down to simple cases

    // Expect User to write out a setup type command, and a data update command for complex options
    // Simple ones mean @vm_chart_line|name|01-01-1900|1 [|seriesName] -- to plot x 1900-01-01, y 1 on chart "name" (optional Seriesname)
    // 			Optional overrides for colours etc
    var data = false;


    //var userData = evt.data;
    if (userData.indexOf("@vm_panel") != -1) {
        data = userData.substring(evt.data.indexOf("@vm_panel"), evt.data.indexOf("vm_panel@"));
        userData = userData.replace(data + "vm_panel@", "");
        data = data.split("|");
        var dataName = data[2];
        var dataLabel = data[3];
        var dataValue = data[3];
        createPanelIfNotExists(data[1], dataName);
    } else
        if (userData.indexOf("@vm_slider") != -1) {
            data = userData.substring(evt.data.indexOf("@vm_slider"), evt.data.indexOf("vm_slider@"));
            userData = userData.replace(data + "vm_slider@", "");
            data = data.split("|");
            var dataName = data[2];
            var dataLabel = data[2];
            var dataValue = data[3];
            if (data.length > 4) {
                var dataMin = data[4];
                var dataMax = data[5];
            }
            var screenIdx = getIndexOfScreenItemByName(dataName);
            if (screenIdx < 0 || (screenIdx >= 0 && !document.getElementById(screenItems[screenIdx].elemId))) {
                screenIdx = screenItems.length;
                screenItems.push((new screenItem(dataName, "slider", getPanelParentFromId(data[1]), "col-md-4")));
            }
            document.getElementById(screenItems[screenIdx].elemId).value = dataValue;
            document.getElementById(screenItems[screenIdx].elemId).min = dataMin;
            document.getElementById(screenItems[screenIdx].elemId).max = dataMax;
            screenItems[screenIdx].data.push(rawDataPoint(dataLabel, dataValue));
            document.getElementById(screenItems[screenIdx].elemId + "_value").innerHTML = dataValue;
            document.getElementById(screenItems[screenIdx].elemId + "_label").innerHTML = dataLabel;

        } else
            if (userData.indexOf("@vm_tile") != -1) { // Basically same as slider
                data = userData.substring(evt.data.indexOf("@vm_tile"), evt.data.indexOf("vm_tile@"));
                userData = userData.replace(data + "vm_tile@", "");
                data = data.split("|");
                var dataName = data[2];
                var dataLabel = data[2];
                var dataValue = data[3];
                var screenIdx = getIndexOfScreenItemByName(dataName);
                if (screenIdx < 0) {
                    screenIdx = screenItems.length;
                    screenItems.push((new screenItem(dataName, "tile", getPanelParentFromId(data[1]), "col-md-4")));
                }
                screenItems[screenIdx].data.push(rawDataPoint(dataLabel, dataValue));
                try {
                    document.getElementById(screenItems[screenIdx].elemId + "_value").innerHTML = dataValue;
                    document.getElementById(screenItems[screenIdx].elemId + "_label").innerHTML = dataLabel;
                } catch { }
            } else
                if (userData.indexOf("@vm_rgbtile") != -1) { // Basically same as slider
                    data = userData.substring(evt.data.indexOf("@vm_rgbtile"), evt.data.indexOf("vm_rgbtile@"));
                    userData = userData.replace(data + "vm_rgbtile@", "");
                    data = data.split("|");
                    var dataName = data[2];
                    var datar = data[3];
                    var datag = data[4];
                    var datab = data[5];
                    var dataa = data[6];

                    // Convert a from 0-100 to 0-1
                    dataa = Math.round((dataa/100) * 100) / 100; // 2.dp rounding, and div by 100

                    var screenIdx = getIndexOfScreenItemByName(dataName);
                    if (screenIdx < 0) {
                        screenIdx = screenItems.length;
                        screenItems.push((new screenItem(dataName, "rgbtile", getPanelParentFromId(data[1]), "col-md-4")));
                    }
                    screenItems[screenIdx].data.push(rawDataPoint(dataName, datar + "," + datag + "," + datab + "," + dataa));
                    try {
                        document.getElementById(screenItems[screenIdx].elemId + "_label").innerHTML = dataName;
                        document.getElementById(screenItems[screenIdx].elemId + "_color").style.backgroundColor = "rgba(" + datar + "," + datag + "," + datab + "," + dataa + ")";
                        document.getElementById(screenItems[screenIdx].elemId + "_r_value").innerHTML = datar;
                        document.getElementById(screenItems[screenIdx].elemId + "_g_value").innerHTML = datag;
                        document.getElementById(screenItems[screenIdx].elemId + "_b_value").innerHTML = datab;
                        document.getElementById(screenItems[screenIdx].elemId + "_a_value").innerHTML = datab;
                    } catch { }
                } else
                    if (userData.indexOf("@vm_xyztile") != -1) { // Basically same as slider
                        data = userData.substring(evt.data.indexOf("@vm_xyztile"), evt.data.indexOf("vm_xyztile@"));
                        userData = userData.replace(data + "vm_xyztile@", "");
                        data = data.split("|");
                        var dataName = data[2];
                        var datax = data[3];
                        var datay = data[4];
                        var dataz = data[5];

                        var screenIdx = getIndexOfScreenItemByName(dataName);
                        if (screenIdx < 0) {
                            screenIdx = screenItems.length;
                            screenItems.push((new screenItem(dataName, "xyztile", getPanelParentFromId(data[1]), "col-md-4")));
                        }
                        screenItems[screenIdx].data.push(rawDataPoint(dataName, datax + "," + datay + "," + dataz));
                        try {
                        document.getElementById(screenItems[screenIdx].elemId + "_label").innerHTML = dataName;
                        document.getElementById(screenItems[screenIdx].elemId + "_transform").style.transform = "rotateX(" + datax + "deg) rotateY(" + datay + "deg) rotateZ(" + dataz + "deg)";
                        document.getElementById(screenItems[screenIdx].elemId + "_x_value").innerHTML = datax;
                        document.getElementById(screenItems[screenIdx].elemId + "_y_value").innerHTML = datay;
                        document.getElementById(screenItems[screenIdx].elemId + "_z_value").innerHTML = dataz;
                        } catch { }
                    }
                else
                if (userData.indexOf("@vm_chart_") != -1) {
                    data = userData.substring(evt.data.indexOf("@vm_chart_line"), evt.data.indexOf("vm_chart_line@"));
                    userData = userData.replace(data + "vm_chart_line@", "");
                    if (!data) {
                        data = userData.substring(evt.data.indexOf("@vm_chart_pie"), evt.data.indexOf("vm_chart_pie@"));
                        userData = userData.replace(data + "vm_chart_pie@", "");
                    }
                    if (!data) {
                        data = userData.substring(evt.data.indexOf("@vm_chart_bar"), evt.data.indexOf("vm_chart_bar@"));
                        userData = userData.replace(data + "vm_chart_bar@", "");
                    }
                    if (!data) {
                        data = userData.substring(evt.data.indexOf("@vm_chart_gague"), evt.data.indexOf("vm_chart_gague@"));
                        userData = userData.replace(data + "vm_chart_gague@", "");
                    }
                    
                    data = data.split("|");
                    var panelId = data[1];
                    var dataName = data[2];
                    var dataLabel = data[3];
                    var dataValue = data[4];
                    var chartType = data[0].split("_")[2];

                    var screenIdx = addScreenItem(dataName, panelId, chartType, "col-md-12");
                    //var seriesName = ;
                    var seriesName = dataName;
                    if (data.length > 5) {
                        seriesName = data[5];
                    }
                    addSeriesValue(screenIdx, seriesName, dataLabel, dataValue);

                    if (chartType == "line") {
                        addLineChartItem(seriesName, screenIdx, dataLabel, dataValue);
                    } else { // Gague, Pie, Bar - all have one label per series, one value per series, in single dataset, with labels holding labels
                        // Check has 1 dataset!
                        var dataSetSet = false;
                        if (chartType == "gague") {
                            seriesName = "Data";
                        }
                        for (var d = 0; d < screenItems[screenIdx].chartObj.data.datasets.length; d++) {
                            if (screenItems[screenIdx].chartObj.data.datasets[d].label == "dataset1") {
                                dataSetSet = true;
                            }
                        }

                        if (!dataSetSet) {
                            addDataSet("dataset1", screenItems[screenIdx].chartObj.data.datasets.length, screenIdx);
                        }
                        // Add a label as needed
                        var seriesSet = false;
                        if (screenItems[screenIdx].chartObj.data.labels.indexOf(seriesName) == -1) {
                            if (!Array.isArray(screenItems[screenIdx].chartObj.data.datasets[0].backgroundColor)) {
                                var baseCol = screenItems[screenIdx].chartObj.data.datasets[0].backgroundColor;
                                screenItems[screenIdx].chartObj.data.datasets[0].backgroundColor = [];
                                screenItems[screenIdx].chartObj.data.datasets[0].backgroundColor.push(baseCol);
                            }
                            
                            screenItems[screenIdx].chartObj.data.labels.push(seriesName);
                            screenItems[screenIdx].chartObj.data.datasets[0].backgroundColor.push(colours[screenItems[screenIdx].chartObj.data.labels.length]);
                            seriesSet = true;
                        }
                        
                        // Push Data into correct array slot
                        var seriesIndex = screenItems[screenIdx].chartObj.data.labels.indexOf(seriesName);
                        screenItems[screenIdx].chartObj.data.datasets[0].data[seriesIndex] = dataValue;

                        // Gagues need an additional series forcing in for MAX Value really like sliders
                        if (chartType == "gague") {
                            if (data.length > 5) {
                                var dataMin = data[5];
                                var dataMax = data[6];
                            
                                if (screenItems[screenIdx].chartObj.data.labels.indexOf("Remaining") == -1) {
                                    screenItems[screenIdx].chartObj.data.datasets[0].backgroundColor.push("rgba(171,171,171,0.3);");
                                    screenItems[screenIdx].chartObj.data.labels.push("Remaining");
                                }
                                var maxIndex = screenItems[screenIdx].chartObj.data.labels.indexOf("Remaining");
                                screenItems[screenIdx].chartObj.data.datasets[0].data[maxIndex] = dataMax - dataValue;
                            }
                        }
                    } 
                /* --> END SPLIT FOR CHART TYPE HERE <-- */

                    screenItems[screenIdx].chartObj.update();
                }


    writeToScreen(userData,"blue");
}

function addSeriesValue(screenIdx, seriesName, dataLabel, dataValue) {
    if (screenItems[screenIdx].hasOwnProperty("data")) {
        if (!screenItems[screenIdx].data[seriesName]) {
            screenItems[screenIdx].data[seriesName] = [];
            screenItems[screenIdx].tracking[seriesName] = new tracking();
        }

        screenItems[screenIdx].data[seriesName].push({ label: dataLabel, value: dataValue });
        screenItems[screenIdx].tracking[seriesName].dataUpdated(dataValue);
    }
}

function addScreenItem(dataName, panelId, chartType, tileClass) {
    var screenIdx = getIndexOfScreenItemByName(dataName);
    if (screenIdx < 0) {
        screenIdx = screenItems.length;
        screenItems.push((new screenItem(dataName, chartType + "Chart", getPanelParentFromId(panelId), tileClass)));
    }
    return screenIdx;
}

function addLineChartItem(seriesName, screenIdx, dataLabel, dataValue) {

    /* --> SPLIT FOR CHART TYPE HERE <-- */
        var seriesSet = false;

        for (var d = 0; d < screenItems[screenIdx].chartObj.data.datasets.length; d++) {
            if (screenItems[screenIdx].chartObj.data.datasets[d].label == seriesName) {
                seriesSet = true;
            }
        }

        if (!seriesSet) {
            addDataSet(seriesName, screenItems[screenIdx].chartObj.data.datasets.length, screenIdx);
        }

        var labelCount = 0;
        for (var d = 0; d < screenItems[screenIdx].chartObj.data.datasets.length; d++) {
            if (screenItems[screenIdx].chartObj.data.datasets[d].label == seriesName) {
                screenItems[screenIdx].chartObj.data.datasets[d].data.push(dataValue);
                // Pop Out an old data Value once limit reached
                if (screenItems[screenIdx].chartObj.data.datasets[d].data.length > maxDisplayChartItems) {
                    screenItems[screenIdx].chartObj.data.datasets[d].data.shift();
                    screenItems[screenIdx].chartObj.data.labels.shift();
                }
                labelCount = screenItems[screenIdx].chartObj.data.datasets[d].data.length;
            }
        }

        // Work out if label missing
        if (screenItems[screenIdx].chartObj.data.labels.length < labelCount) {
            screenItems[screenIdx].chartObj.data.labels.push(dataLabel);
        }
}

function getIndexOfScreenItemByName(name) {
    for (var s = 0; s < screenItems.length; s++) {
        if (screenItems[s].itemName == name) {
            return s;
        }
    }
    return -1;
}

function getIndexOfScreenItemElemId(elemId) {
    for (var s = 0; s < screenItems.length; s++) {
        if (screenItems[s].elemId == elemId) {
            return s;
        }
    }
    return -1;
}

function getIndexOfPanelById(elemId) {
    for (var s = 0; s < screenItems.length; s++) {
        if (screenItems[s].hasOwnProperty("panelId")) {
            if (screenItems[s].panelId == elemId) {
                return s;
            }
        }
    }
    return -1;
}

function addPort(Address, ComPort, Name) {
    var basePort = new SerialPort(Address, ComPort, Name);
    var nextIndex = ports.length;
    for (var p = 0; p < ports.length; p++) {
        if (ports[p].ADDRESS == Address) {
            ports[p].NAME = Name;
            ports[p].PORT = COMPort;
            return p;
        }
    }
    ports.push(basePort);
    return nextIndex;
}


function onError(evt) {
    writeToScreen('ERROR:' + evt.data, "red");
    try {
        websocket.close();
    } catch { }
}

function doSend(portId) {
    var message = document.getElementById("userInput").value;
    writeToScreen("SENT: " + message, "purple");
    sendMessage(portId, message);
    document.getElementById("userInput").value = "";
}

function doStep(portId) {
    $("#step").removeClass("btn-success");  
    $("#step").addClass("btn-outline-success");  
    sendMessage(portId, "c");
}

function sendDebugDataValue(el, portId) {
    var res = "v" + String.fromCharCode(2) + String.fromCharCode(0) + String.fromCharCode(el.id.split("_")[1]) /*bpid*/ + String.fromCharCode($("#" + el.id).val().length) /*len*/ + $("#" + el.id).val();
    sendMessage(portId, res);
}

function sendMessage(portId, message) {
    ports[portId].WEBSOCKET.send(message);
}

function writeToScreen(message, color) {
    var pre = document.createElement("p");
    pre.style.wordWrap = "break-word";
    var span = document.createElement("span");
    span.style.color = color;
    span.innerHTML = message;
    pre.appendChild(span);

    var objDiv = document.getElementById("output");
    if (output) {
        output.appendChild(pre);
		// Ensure we have a limited amount of Serial shown on screen
		if (output.children.length > maxDisplaySerialItems) {
			for (var i = 0; i < (output.children.length - maxDisplaySerialItems); i++) {
				output.removeChild(output.childNodes[0]);
			}
		}
		// Stick to bottom
        objDiv.scrollTop = objDiv.scrollHeight;
    }
}



window.addEventListener("load", init, false);

/*  JS Charting ------------------------------------------------------------------------------------------------------------------------------ */
var lightColours = [];
var lightBorders = [];
var darkColours = [];
var darkBorders = [];

var colours = [];
colours.push('rgba(169, 68, 66, 0.2)');
colours.push('rgba(94, 54, 143, 0.2)');
colours.push('rgba(51, 122, 183, 0.2)');
colours.push('rgba(60, 118, 61, 0.2)');
colours.push('rgba(138, 109, 59, 0.2)');
lightColours = colours;

var borders = [];
borders.push('rgba(169, 68, 66, 1)');
borders.push('rgba(94, 54, 143, 1)');
borders.push('rgba(51, 122, 183, 1)');
borders.push('rgba(60, 118, 61, 1)');
borders.push('rgba(138, 109, 59, 1)');
lightBorders = borders;

var darkColours = [];
darkColours.push('rgba(50, 205, 50, 1)');
darkColours.push('rgba(111, 54, 143, 0.2)');
darkColours.push('rgba(111, 122, 183, 0.2)');
darkColours.push('rgba(111, 118, 61, 0.2)');
darkColours.push('rgba(111, 109, 59, 0.2)');

var darkBorders = [];
darkBorders.push('rgba(169, 68, 66, 1)');
darkBorders.push('rgba(94, 54, 143, 1)');
darkBorders.push('rgba(51, 122, 183, 1)');
darkBorders.push('rgba(60, 118, 61, 1)');
darkBorders.push('rgba(138, 109, 59, 1)');

function addDataSet(seriesLabel, index, screenItemId) {
    var newDataset = {
        label: seriesLabel,
        backgroundColor: colours[index],
        borderColor: borders[index],
        borderWidth: 1,
        data: [0],
    }

    // You add the newly created dataset to the list of `data`
    screenItems[screenItemId].chartObj.data.datasets.push(newDataset);
}


function rawDataPoint(dataLabel, dataValue) {
    return { label: dataLabel, value: dataValue, timestamp: (new Date).toLocaleTimeString() };
}


//function addTabEventHandlers() {
//    var tabButtons = [].slice.call(document.querySelectorAll('ul.tab-nav li a.button'));

//    tabButtons.map(function (button) {
//        addTabEvent(button);
//    })
//}

//function addTabEvent(button) {
//    button.addEventListener('click', function () {
//        document.querySelector('li a.active.button').classList.remove('active');
//        button.classList.add('active');
//        try {
//            document.querySelector('.tab-pane.active').classList.remove('active');
//        } catch { }
//        document.getElementById(button.getAttribute('panel')).classList.add('active');
//        button.classList.add('active');
//    })
//}

//if (document.readyState !== 'loading') {
//    addTabEventHandlers();
//} else {
//    document.addEventListener('DOMContentLoaded', addTabEventHandlers);
//}


function getPanelParentFromId(panelIndex) {
    for (var p = 0; p < screenItems.length; p++) {
        if (screenItems[p].hasOwnProperty("panelId")) {
            if (screenItems[p].panelId == panelIndex) {
                return screenItems[p].elemId;
            }
        }
    }
    return -1;
}

function createPanelIfNotExists(panelId, panelName) {
    // Check if Panel Exists by name else create
    var screenIdx = getIndexOfScreenItemByName(panelName);
    if (screenIdx < 0) {
        screenIdx = screenItems.length;
        screenItems.push((new screenItem(panelName, "panel", "userPanels", "")));
        screenItems[screenIdx].panelId = panelId;
    }
}



/* File Saving ---------------------------------------------------------------------------------------------- */
// e.g saveTextFile("Boo!", "Scary.txt")
function saveTextFile(content, filename) {
    var blob = new Blob([content], { type: "text/plain;charset=utf-8" });
    saveAs(blob, filename);
}


/* 3D Animations --------------------------------------------------------------------------------------------- */
function play(el, x, y, z) {
    $(el).resetKeyframe(function () {
        deg += 10;
        $.keyframe.define(
            {
                name: 'ball-spin',
                'from/to': {
                    'transform': ' rotateX(0deg) rotateY(0deg) rotateZ(0deg)',
                    '-moz-transform': ' rotateX(0deg) rotateY(0deg) rotateZ(0deg)',
                    '-ms-transform': ' rotateX(0deg) rotateY(0deg) rotateZ(0deg)'

                },
                '100%': {
                    'transform': ' rotateX(' + deg + 'deg) rotateY(' + deg + 'deg) rotateZ(' + deg + 'deg)',
                    '-moz-transform': ' rotateX(' + deg + 'deg) rotateY(' + deg + 'deg) rotateZ(' + deg + 'deg)',
                    '-ms-transform': ' rotateX(' + deg + 'deg) rotateY(' + deg + 'deg) rotateZ(' + deg + 'deg)'
                },

            });

        // run spin keyframes using the shorthand method.
        $(el).playKeyframe({
            name: 'ball-spin',
            timingFunction: 'linear'
            //,complete: increment
        });
    });
}

function pause(el) {
    // freeze keyframe animation and kill callback
    $(el).pauseKeyframe();
}
function resume(el) {
    // resume keyframe animation
    $(el).resumeKeyframe();
}
// example callback function
function increment() {
    // $('#cb').html(parseInt($('#cb').html()) + 1);
}


/* -------------------- THEMING --------------------------------------------- */
// Using getComputedStyle we can access the rendered colour after CSS applied for our Pallette elements
function grabThemeColours() {
    darkColours = [];
    $('.ChartThemePallette_dark').each(function (index, el) {
        darkColours.push(getComputedStyle(el, null).getPropertyValue("background-color"));
    });
    darkBorders = [];
    $('.ChartThemePallette_Border_dark').each(function (index, el) {
        darkBorders.push(getComputedStyle(el, null).getPropertyValue("background-color"));
    });

    lightColours = [];
    $('.ChartThemePallette_light').each(function (index, el) {
        lightColours.push(getComputedStyle(el, null).getPropertyValue("background-color"));
    });
    lightBorders = [];
    $('.ChartThemePallette_Border_light').each(function (index, el) {
        lightBorders.push(getComputedStyle(el, null).getPropertyValue("background-color"));
    });

}

function handleLightBrowserScheme(scheme) {
    if (scheme.matches) {
        // Handle Charts!
        colours = lightColours;
        borders = lightBorders;
        // Update and Re-Render all Charts
        for (var si = 0; si < screenItems.length; si++) {
            if (screenItems[si].hasOwnProperty("chartObj")) {
                for (var ds = 0; ds < screenItems[si].chartObj.data.datasets.length; ds++) {
                    screenItems[si].chartObj.data.datasets[ds].backgroundColor = colours[ds];
                }
                screenItems[si].chartObj.update();
            }
        }
    }
}
    function handleDarkBrowserScheme(scheme) {
        if (scheme.matches) {
            colours = darkColours;
            borders = darkBorders;
            // Update and Re-Render all Charts
            for (var si = 0; si < screenItems.length; si++) {
                if (screenItems[si].hasOwnProperty("chartObj")) {
                    for (var ds = 0; ds < screenItems[si].chartObj.data.datasets.length; ds++) {
                        screenItems[si].chartObj.data.datasets[ds].backgroundColor = colours[ds];
                    }
                    screenItems[si].chartObj.update();
                }
            }
        }
    }


function create_downloadButton(downloadItemName) {
    var dla = document.createElement("a");
    dla.onclick = function () {
        // Find the Item Index
        var idx = getIndexOfScreenItemElemId(downloadItemName);
        if (idx > -1) {
            saveTextFile(dataArrayToCSV(idx), screenItems[idx].itemName + ".csv");
        }
    };
    var dl = document.createElement("img");
    dl.src = "images/cloud_download-24px.png"
    dl.className = "vmdownload-data-image float-right";
    dla.appendChild(dl);
    return dla;
}

// Converts our array of data objects back to a CSV (uses data props to perform translation)
function dataArrayToCSV(idx) {
    // Obtain the data Object
    var returnString = "";
    var dlData = screenItems[idx].data;
    if (screenItems[idx].hasOwnProperty("chartObj")) {
        // Multi Series Download - Segmented File for now....
        var seriesAvailable = Object.keys(dlData);
        
        for (var s = 0; s < Object.keys(dlData).length; s++) {
            if (seriesAvailable[s] != "0") {
                var seriesDataArr = screenItems[idx].data[seriesAvailable[s]];
                if (returnString.length == 0) {
                    returnString += turnSeriesToCSV(seriesDataArr, seriesAvailable[s], true);
                } else {
                    returnString += turnSeriesToCSV(seriesDataArr, seriesAvailable[s], false);
                }
            }
        }
    } else {
        // Single Series Download
        returnString = turnSeriesToCSV(dlData, undefined, true);
    }

    
    return returnString;
}

function turnSeriesToCSV(seriesArray, seriesName, withHeaders) {
    var seriesString = "";
    var cols = Object.keys(seriesArray[1]);


    if (withHeaders) {
        if (seriesName) {
            seriesString = '"SeriesName",';
        }
        for (var c = 0; c < cols.length; c++) {
            seriesString += '"' + cols[c] + '"';
            if (c + 1 != cols.length) {
                seriesString += ',';
            }
        }
        seriesString += "\n";
    }

    for (var d = 1; d < seriesArray.length - 1; d++) {
        if (seriesName) {
            seriesString += '"' + seriesName + '",';
        }

        for (var c = 0; c < cols.length; c++) {
            seriesString += '"' + seriesArray[d][cols[c]] + '"';
            if (c + 1 != cols.length) {
                seriesString += ',';
            }
        }
        seriesString += "\n";
    }
    return seriesString;
}

function create_collapseButton(elemId) {
    var collapsible = document.createElement("button");
    collapsible.type = "button";
    collapsible.className = "btn btn-default float-right";
    collapsible.style.marginRight = "5px";
    collapsible.innerHTML = "+/-";
    collapsible.setAttribute("data-toggle", "collapse");
    collapsible.setAttribute("data-target", "#" + elemId + "_cardBody");
    if (dev_collapseCardsByDefault) {
        collapsible.setAttribute("aria-expanded", "false");
    } else {
        collapsible.setAttribute("aria-expanded", "true");
    }
    return collapsible;
}

// Create Tile - returns modified element
function createBaseTile(className, maxWidth, elemId) {

    var chartTile = document.createElement("div");
    chartTile.className = className;
    chartTile.style.maxWidth = maxWidth;

    var chartTileHead = document.createElement("div");
    chartTileHead.className = "card-header";
    // chartTileHead.innerHTML = "Download Data"; -- Glue In Filesaver JS Link for Data

    var l = document.createElement("span");
    l.className = "vmCardHeading";
    l.id = elemId + "_label";
    l.innerHTML = "..."; // Waiting for data
    chartTileHead.appendChild(l);
    
    chartTileHead.appendChild(create_downloadButton(elemId));
    chartTileHead.appendChild(create_collapseButton(elemId));
    chartTile.appendChild(chartTileHead);
    return chartTile;
}

// Wrap Moveable Elements in required Divs
function ui_WrapTile(thisTile, tileClass) {
    var elwrap = document.createElement("div");
    if (dev_useMuuri) {
        elwrap.className = "item";
        var muuriInner = document.createElement("div");
        muuriInner.className = "item-content";
        muuriInner.appendChild(thisTile);
        elwrap.appendChild(muuriInner);
        return elwrap;
    } else {
        elwrap.className = tileClass;
        elwrap.appendChild(thisTile);
        return elwrap;
    }   
}

// Add on Panel Create as well as each time a widget is added
function createPanelGrid(screenItemId) {
    if (!dev_useMuuri) { return }
    // Muuri Grid Binding -- Will Hive Object Off into Panel if working
    Muuri.defaultOptions.dragEnabled = true;
    Muuri.defaultOptions.dragPlaceholder.enabled = true;
    return new Muuri('#' + screenItems[screenItemId].elemId); 
}

function addItemToGrid(panelElemId, elementToAdd) {
    if (!dev_useMuuri) { return }

    var pid = getIndexOfScreenItemElemId(panelElemId);
    if (screenItems[pid]) {
        if (!screenItems[pid].hasOwnProperty("grid")) {
            Muuri.defaultOptions.dragEnabled = true;
            Muuri.defaultOptions.dragPlaceholder.enabled = true;
            screenItems[pid].grid = new Muuri('#' + panelElemId);
        }
        else if (pid >= 0) {
            try {
                ////screenItems[pid].grid.refreshItems().layout();
                //screenItems[pid].grid.destroy()
                //if (screenItems[pid].hasOwnProperty("uiTimer")) {
                //    clearTimeout(screenItems[pid].uiTimer);
                //}
                //screenItems[pid].uiTimer = setTimeout(function () {
                screenItems[pid].grid.destroy();
                Muuri.defaultOptions.dragEnabled = true;
                Muuri.defaultOptions.dragPlaceholder.enabled = true;
                screenItems[pid].grid = new Muuri('#' + panelElemId);
                screenItems[pid].grid.add(document.getElementById(elementToAdd), { layout: "instant" });
                screenItems[pid].grid.refreshItems();
                screenItems[pid].grid.refreshItems().layout();
                //}, 1000);
                //
                //
            } catch (e) {
                console.log(e);
                console.log("GridAddErr:" + elementToAdd + "  PanelId:" + panelElemId);
            }
        }
    }
}



function ui_card_getClasses(){
    var classes = "card-body collapse";
    if (dev_collapseCardsByDefault) {
        return classes;
    } 
    return classes + " show";
}