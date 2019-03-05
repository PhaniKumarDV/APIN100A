/*
 * Copyright (c) 2015-2016 Qualcomm Atheros, Inc.
 *
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */

//////////////////////////////////////////////////////////////
/*
    translation routines - ported from flux
*/
//////////////////////////////////////////////////////////////
/*
    Function: getLanguage

    get the language the browser would like us to display

    Parameters:
      none.

    Returns:
      language code string (Example: EN-US)

    See also:
      nothing.
*/
////////////////////////////////////////////////////////////////////////
function getLanguage()
{
    //get the browsers language
    var language = window.navigator.userLanguage || window.navigator.language;

    //if we get something
    if(language != undefined)
    {
        //convert to lower
        language = language.toUpperCase();
    }

    //give it back
    return language;
}

/*
    Variable: g_Language

    global instance of the translation table

    file format for load translation is json:
    {
        "language": "name",
        trans
        [
            {"string 1" : "translation 2"},
            {"string 2" : "translation 2"},
        ]
    }
*/
var g_Language = null;

//the path of the externar cloud resources
var g_extPath = null;

//are we in a ext path testing mode?
var g_bExtTest = true;

////////////////////////////////////////////////////////////////////////
/*
    Function: extPath

    Get the external path for the cloud assets. This assembled from
    the model_information in ozker.

    Parameters:
      none

    Returns:
        g_extPath

    See also:
      nothing.
*/
////////////////////////////////////////////////////////////////////////
function extPath()
{
    if(g_extPath == null)
    {
        //try to load the language
        $.ajax(
                {
                    type: "GET",
                    async: false,
                    url: "/cgi-bin/lil-ozker/api/model_information",
                    dataType: "json",
                    //-------------------------------------------------------------------
                    // Issue callback with result data
                    success: function(data, status, request)
                    {
                        var strModel    = data.BOARD_MODEL;
                        var strMfg      = data.BOARD_MFG;
                        var strVersion  = data.BOARD_VERSION;
                        var strHost     = data.STREAMBOOST3_HOST;

                        if(g_bExtTest == true)
                        {
                            strModel = "welsh";
                        }

                        //set the global path to the cloud data
                        //like : static-qca-welsh-beryllium.streamboost.qualcomm.com
                        g_extPath = "static-"+strMfg+"-"+strModel+"-"+strVersion+"."+strHost;

                    },
                    error: function(data, status, request)
                    {
                       //no real recovery for this...means api/model_information failed to return
                       console.log("ozker's '/api/model_information' isn't responding!");
                    },

        });
    }

    return g_extPath;
}

////////////////////////////////////////////////////////////////////////
/*
    Function: loadTranslation

    loadTranslation from the web server

    Parameters:
      loadedCB - callback to execute when data is loaded
      strFile  - Web translation file to load

    Returns:
      false = failure
      true  = success

    See also:
      nothing.
*/
////////////////////////////////////////////////////////////////////////
function loadTranslation(loadedCB,strFile)
{
    //fail by default
    var bReturn = false;

    //init external cloud path
    extPath();

    //1st get the local language
    var strGetFile = "translate_"+getLanguage()+".js";
    var strGetType = "jsonp";
    var strURL = "http://"+g_extPath +"/trans/"+ strGetFile;

    //if we are loading a local file
    if(typeof strFile != 'undefined' && strFile != "")
    {
        //set it up to get json for local not jsonp
        strGetType = "json";
        strGetFile = strFile;
        strURL = g_path.lstrings + strGetFile;
    }

    //try to load the language
    $.ajax(
            {
                type: "GET",
                async: true,
                url: strURL,
                jsonpCallback: "jsonpCallback",
                dataType: strGetType,
                timeout: 3000,
                //-------------------------------------------------------------------
                // Issue callback with result data
                success: function(data, status, request)
                {
                    //set the global language to the returned data
                    g_Language = data;

                    //if we loaded the language return success
                    bReturn = true;

                    //callback return
                    loadedCB();
                },
                error: function(data, status, request)
                {
                    //if we arent already english
                    if(strGetFile != "translate_EN-US.json")
                    {
                        //try to load english
                        strGetFile = "translate_EN-US.json";

                        //do the load / if this doesn't work the
                        //router is most likely really really broken
                        loadTranslation(loadedCB,strGetFile);
                    }
                    else
                    {
                        //if we are here the browser failed to load a translation file
                        console.log("Unable to load translation files. Please check your network connection.");

                        //callback return
                        loadedCB();
                    }
                },

    });

    return bReturn;
}

////////////////////////////////////////////////////////////////////////
/*
    Function: _t

    get the translated version of a string

    Parameters:
      strTranslate - string to translate

    Returns:
      the translated string

    See also:
      nothing.
*/
////////////////////////////////////////////////////////////////////////
function _t(strTranslate)
{
    //set the string given to return by default
    var strReturn =strTranslate;

    if(typeof g_Language!='undefined' && g_Language != null)
    {
        //to get the language string
        var strTemp = g_Language.trans[strTranslate];

        //if we got back a valid string
        if(typeof strTemp != 'undefined' && strTemp != "")
        {
            //set the return
            strReturn = strTemp;
        }
    }

    return strReturn;
};


////////////////////////////////////////////////////////////////////////
/*
    Function: addUpdateLicense

    Add the Streamboost Update License to a div.

    Parameters:
      div - the div to add the eula to.

    Returns:
      nothing

    See also:
      none.
*/
////////////////////////////////////////////////////////////////////////
function addUpdateLicense(div)
{
    //load the translated license
    var strLicense =   _t("Update_License");

    //if we get back a valid string array
    if(strLicense!= undefined && strLicense != null && strLicense.length != undefined && strLicense.length > 0)
    {
        //join the array of the license and put it in the div
        $(div).append(strLicense.join(""));
    }
}


//////////////////////////////////////////////////////////////
/*
    this is the functionality for drawing the node view
*/
//////////////////////////////////////////////////////////////

//darmok node class
function dkNode()
{
    //for scoping reasons
    var self = this;

    //the parent div to draw in
    this.strDiv = "";

    this.upChart = null;
    this.dnChart = null;

    //id's
    this.nItemID = 0;
    this.nSubItemID = 0;
    this.nMeterID = 0;

    //make bw for meter
    //set max to 1 gig
    this.nMaxUp = 1000*1000*1000;
    this.nMaxDn = 1000*1000*1000;

    //default to no children
    this.bChildren = false;

    //create the node view
    this.init = function(   parent,
                            strName,
                            strIcon,
                            bChildren,
                            nMaxUp,
                            nMaxDn)
    {
        self.strDiv = parent;
        self.bChildren = bChildren;

        //store off the max bandwidths
        if(typeof nMaxUp != 'undefined')
        {
            self.nMaxUp = parseInt(nMaxUp);
        }

        if(typeof nMaxDn != 'undefined')
        {
            self.nMaxDn = parseInt(nMaxDn);
        }

        //set up the node
        self.setNode(   strName,
                        strIcon);

        //create flow table
        var strTable = "<table  id='FlowTable' border='0' cellspacing='0' cellpadding='0'></table>";

        //append it
        $("#"+self.strDiv).append(strTable);
    }

    //set the top node
    this.setNode = function(strName,
                            strIcon)
    {
        var strNode = "";

        strNode += "<table id='NodeTable' border'0' cellspacing='0' cellpadding='0'>";
        strNode += "<tr class='networkrow' style='height: 240px;background-color:#8b8f95'>";
        strNode += "<td style='width: 240px; background-color:#81868c;border-radius:10px 0px 0px 10px;border: 0px;'>";
        strNode += "<table id='LabelTable' style='width: 100%; height: 100%; padding: 0; border-collapse: collapse; border-spacing: 0px;min-width: 240px;' border='0' cellspacing='0' cellpadding='0'>";
        strNode += "<tr class='networkrow' style='height: 40px;background-color:#498ba0'>";
        strNode += "<td style='border: 0px;margin: 0px;border-radius: 10px 0px 0px 0px;font-family: \"Segoe UI\", sans-serif; font-weight: bold; color: white;'>";
        strNode += _t(strName);
        strNode += "</td></tr>";
        strNode += "<tr class='networkrow' style='height: 200px;'><td style='height: 200px; border: 0px; margin: 0px;'>";
        strNode += "<img id='deviceicon' src='"+strIcon+"'>";
        strNode += "</td></tr>";
        strNode += "</table>";
        strNode += "</td>";
        strNode += "<td style='background-color:#8b8f95;border-radius:0px 00px 00px 0px;border: 0px;color:white;'>";
        strNode += _t("Upload");
        strNode += "<canvas id='chartupload' style='font-family: \"Segoe UI\", sans-serif; font-size: 12px;fill:white;width: 90%; height: 200px;'' height='200' width='450'></canvas>";
        strNode += "</td>";
        strNode += "<td style='background-color:#8b8f95;border-radius:0px 10px 10px 0px;border: 0px;color:white;'>";
        strNode += _t("Download");
        strNode += "<canvas id='chartdownload' style='font-family: \"Segoe UI\", sans-serif; font-size: 12px;fill:white;width: 90%; height: 200px;'  height='200' width='450'></canvas>";
        strNode += "</td>";
        strNode += "</tr>";
        strNode += "</table>";

        $("#"+self.strDiv).append(strNode);

        self.upChart = self.makeChart(  "chartupload",
                                        "rgba(220,220,220,0.2)",
                                        "rgba(220,220,220,1)",
                                        "rgba(220,220,220,1)",
                                        "#fff",
                                        parseInt(self.nMaxUp/1000/1000));

        self.dnChart = self.makeChart(  "chartdownload",
                                        "rgba(73,139,160,0.2)",
                                        "rgba(73,139,160,1)",
                                        "rgba(73,139,160,1)",
                                        "#fff",
                                        parseInt(self.nMaxDn/1000/1000));
    }

    //set the top node
    this.updateNode = function( nUp,
                                nDn)
    {
        //update the charts with the two new values
        self.updateChart(   self.upChart,
                            nUp);

        self.updateChart(   self.dnChart,
                            nDn);
    }

    this.makeChart = function(  divid,
                                fillcolor,
                                strokecolor,
                                pointcolor,
                                pointstrokecolor,
                                nMyMax)
    {
        var myData = [];
        var myLabels = [];
        for(var n = 60; n>=0;n--)
        {
            var label = "";
            if((parseInt(n/10)*10) == n)
            {
                label = n.toString();
            }

            myLabels.push(label);

            myData.push(0);
        }

        var lineChartData = {
            labels : myLabels,
            datasets : [
                {
                    label: "My First dataset",
                    fillColor : fillcolor,
                    strokeColor : strokecolor,
                    pointColor : pointcolor,
                    pointStrokeColor : pointstrokecolor,
                    pointHighlightFill : "#fff",
                    pointHighlightStroke : "rgba(220,220,220,1)",
                    data : myData,
                    showXLabels: 10
                }
            ]
        };

        var ctx = document.getElementById(divid).getContext("2d");

        var nKeys = 10;

        var myChart = new Chart(ctx).Line(lineChartData,
        {
            responsive: false,
            scaleOverride: true,
            scaleStartValue: 0,
            scaleStepWidth: (nMyMax)/nKeys,
            scaleSteps: nKeys,
            animationSteps: 1,
            scaleFontColor: "#fff",
            pointDot : false,
            animation: true,
            showTooltips: false
        });

        return myChart;
    }

    this.updateChart = function(chart,
                                nAddValue)
    {
        var data = chart.datasets[0].points;
        //remove old data
        for(var n = 0; n<60;n++)
        {
            data[n].value = data[n+1].value;
        }
        //add new value
        data[60].value = nAddValue;
        chart.update();
    }

    //add item
    //return id of item for updates/children
    this.addItem = function(strName,
                            strIcon,
                            bShrinkIcon)
    {
        var strItem = "";
        var nMyID = self.nItemID++;

        var strDisplay = "";

        if(self.bChildren == false)
        {
            strDisplay = "display:none;";
        }

        //create the items html to add
        strItem += "<tr id='item"+nMyID+"' class='flowrow'>";
        strItem += "<td style='padding-bottom: 10px;'>";
        strItem += "<table  id='DeviceTable' border='0' cellspacing='0' cellpadding='0'>";
        strItem += "<tr class='devicerow'>";
        strItem += "<td style='border-radius: 10px; background-color: #81868c;'>";
        strItem += "<table  border='0' cellspacing='0' cellpadding='0' style='width: 100%;'>";
        strItem += "<tr>";
        strItem += "<td style='width: 72px;'>";

        //do we use smaller icons?
        if(typeof bShrinkIcon != 'undefined' && bShrinkIcon == true)
        {
            strItem += "<img id='deviceicon' src='"+strIcon+"' style='height: 60px;position: absolute;margin-top: -28px;margin-left: -15px;z-index: 128;'>";
        }
        else // nope use larger
        {
            strItem += "<img id='deviceicon' src='"+strIcon+"' style='height: 72px;position: absolute;margin-top: -36px;margin-left: -20px;z-index: 128;'>";
        }

        strItem += "</td>";
        strItem += "<td style='width:165px;'>";
        strItem += "<div style='position: relative;width: 165px;display: table-cell;vertical-align: middle;background-color: white;height: 32px;border-radius: 0px 5px 5px 0px;-webkit-box-shadow: 5px 5px 0px 0px #b6b7b7;-moz-box-shadow: 5px 5px 0px 0px #b6b7b7;box-shadow: 5px 5px 0px 0px #b6b7b7;font-family: \"Segoe UI\";font-size: 14px;font-weight: 800;color: #498ba0;'>";
        strItem += _t(strName);
        strItem += "</div>";
        strItem += "</td>";
        strItem += "<td id='itemmeters"+nMyID+"'>";
        strItem += "</td>";
        strItem += "<td id='openclose"+nMyID+"' style='width:48px;cursor: pointer;"+strDisplay+"'>";
        strItem += "<img id='openclose"+nMyID+"' src='/images/down_arrow.png' width='16' style='position: absolute;margin-left: -18px;margin-top: -8px;cursor: pointer; '>";
        strItem += "</td>";
        strItem += "</tr>";
        strItem += "</table>";
        strItem += "</td>";
        strItem += "</tr>";
        strItem += "<tr class='childrow'>";
        strItem += "<td style='height:100%;min-height:64px;'>";
        strItem += "<div style='border-radius: 0px 0px 10px 10px; background-color:#8b8f95;height: 100%;margin-left: 25px;margin-right: 25px;'>";
        strItem += "<table  id='children"+nMyID+"' border='0' cellspacing='0' cellpadding='0' style='width: 97%;'>";
        strItem += "</table>";
        strItem += "</div>";
        strItem += "</td>";
        strItem += "</tr>";
        strItem += "</table>";
        strItem += "</td>";
        strItem += "</tr>";

        $("#FlowTable").append(strItem);

        var meterID = self.meters(  undefined,
                                    0,
                                    0,
                                    self.nMaxUp,
                                    self.nMaxDn,
                                    "itemmeters"+nMyID);

        $("#item"+nMyID).attr("meterID",parseInt(meterID));
        $("#item"+nMyID).attr("childID",0);

        $("#openclose"+nMyID).click(function(event)
        {
            var id = event.target.id;
            var nID =id.replace("openclose","");

            var visible = $("#children"+nID).css("display");

            if(visible == "none")
            {
                $("#children"+nID).css("display","table");
                $("#openclose"+nID).children().attr("src", "/images/down_arrow.png");
            }
            else
            {
                $("#children"+nID).css("display","none");
                $("#openclose"+nID).children().attr("src", "/images/left_arrow.png");
            }
        });

        //return item id
        return nMyID;
    }


    //add item
    this.itemEvents = function(nID)
    {

    }

    //update item
    this.updateItem = function( nID,
                                nUp,
                                nDn)
    {
        var meterID = $("#item"+nID).attr("meterID");

        self.meters(meterID,
                    nUp,
                    nDn,
                    self.nMaxUp,
                    self.nMaxDn);
    }

    //delete item
    this.delItem = function(nID)
    {
        $("#item"+nID).remove();
    }

    //add child itom
    //return id for subsequent updates
    this.addChild = function(   nIDParent,
                                strName,
                                strIcon)
    {
        var strChild = "";

        var childID = $("#item"+nIDParent).attr("childID");
        $("#item"+nIDParent).attr("childID",parseInt(childID)+1);

        strChild += "<tr id='child"+nIDParent+"_"+childID+"' class='flowrow'>";
        strChild += "<td style='width: 72px;'>";
        strChild += "<img id='deviceicon' src='"+strIcon+"' style='height: 60px;position: absolute;margin-top: -28px;margin-left: -15px;z-index: 128;'>";
        strChild += "</td>";
        strChild += "<td style='width:140px;'>";
        strChild += "<div style='position: relative;width: 165px;display: table-cell;vertical-align: middle;background-color: white;height: 32px;border-radius: 0px 5px 5px 0px;-webkit-box-shadow: 5px 5px 0px 0px #b6b7b7;-moz-box-shadow: 5px 5px 0px 0px #b6b7b7;box-shadow: 5px 5px 0px 0px #b6b7b7;font-family: \"Segoe UI\";font-size: 14px;font-weight: 800;color: #498ba0;'>";
        strChild += _t(strName);
        strChild += "</div>";
        strChild += "</td>";
        strChild += "<td id='childmeters"+nIDParent+"_"+childID+"'>";
        strChild += "</td>";
        strChild += "</tr>";
        strChild += "</table>";
        strChild += "</div>";
        strChild += "</div>";
        strChild += "</td>";
        strChild += "</tr>";

        //add it to our parent
        $("#children"+nIDParent).append(strChild);

        var meterID = self.meters(  undefined,
                                    0,
                                    0,
                                    self.nMaxUp,
                                    self.nMaxDn,
                                    "childmeters"+nIDParent+"_"+childID);

       $("#child"+nIDParent+"_"+childID).attr("meterID",meterID);

        return childID;
    }

    //update child item
    this.updateChild = function(nIDParent,
                                nChildID,
                                nUp,
                                nDn)
    {
        var meterID = $("#child"+nIDParent+"_"+nChildID).attr("meterID");

        self.meters(meterID,
                    nUp,
                    nDn,
                    self.nMaxUp,
                    self.nMaxDn);
    }


    //delete child item
    this.delChild = function(   nIDParent,
                                nChildID)
    {
        $("#child"+nIDParent+"_"+nChildID).remove();
    }

    //draw item meters
    //return the ID fo the meter
    //if nID in populated this in an update
    //if nID in undefined this creates the meter
    this.meters = function( nID,
                            nUp,
                            nDn,
                            nMaxUp,
                            nMaxDn,
                            strDiv)
    {
        var nMyID = nID;

        if(typeof nID == 'undefined')
        {
            var strMeters = "";
            nMyID = this.nMeterID++;

            strMeters += "<div id='meters"+nMyID+"'>";
            strMeters += "<div style='position: relative;width: 95%;border-radius: 8px;display: inline-block;padding-left: 16px;text-align: right;font-family: \"Segoe UI\";font-size: 14px;'>";
            strMeters += "<table border='0' cellspacing='0' cellpadding='0' style='width:100%;'>";
            strMeters += "<tr>";
            strMeters += "<td id='labelUp"+nMyID+"' style='width:100px;width: 100px;border-right-color: black;border-right-style: solid;border-right-width: 1px;padding-right: 16px;'>12.33 Mbps</td>";
            strMeters += "<td>";
            strMeters += "<div id='trackUp"+nMyID+"' style='border-radius: 0px 10px 10px 0px; background-color:white;width: 100%;margin-right: 25px;height:16px;'>";
            strMeters += "<div id='meterUp"+nMyID+"'style='border-radius: 0px 10px 10px 0px; background-color:#498ba0;width: 0%;height:16px;'>";
            strMeters += "</div>";
            strMeters += "</div>";
            strMeters += "</td>";
            strMeters += "</tr>";
            strMeters += "<tr>";
            strMeters += "<td id='labelDn"+nMyID+"' style='width:100px;width: 100px;border-right-color: black;border-right-style: solid;border-right-width: 1px;padding-right: 16px;'> 2.19 Mbps</td>";
            strMeters += "<td>";
            strMeters += "<div id='trackDn"+nMyID+"' style='border-radius: 0px 10px 10px 0px; background-color:white;width: 100%;margin-right: 25px;height:16px;'>";
            strMeters += "<div id='meterDn"+nMyID+"'style='border-radius: 0px 10px 10px 0px; background-color:#498ba0;width: 0%;height:16px;'>";
            strMeters += "</div>";
            strMeters += "</div>";
            strMeters += "</td>";
            strMeters += "</tr>";
            strMeters += "</table>";
            strMeters += "</div>";
            strMeters += "</div>";

            $("#"+strDiv).append(strMeters);
        }

        var nUpPercent = ((nUp*8)/nMaxUp)*100;
        var nDnPercent = ((nDn*8)/nMaxDn)*100;
        var strUpLabel = bytesToSize(nUp*8,1);
        var strDnLabel = bytesToSize(nDn*8,1);

        if(nUpPercent<0)
        {
            nUpPercent = 0;
        }

        if(nDnPercent<0)
        {
            nDnPercent = 0;
        }

        if(nUpPercent>100)
        {
            nUpPercent = 100;
        }

        if(nDnPercent>100)
        {
            nDnPercent = 100;
        }

        $("#meterUp"+nMyID).stop().animate({width: nUpPercent+"%" });
        $("#meterDn"+nMyID).stop().animate({width: nDnPercent+"%" });

        $("#labelUp"+nMyID).text(strUpLabel);
        $("#labelDn"+nMyID).text(strDnLabel);

        return nMyID;
    }
};

//////////////////////////////////////////////////////////////
/*
    this is the functionality for drawing the darmok menus
*/
//////////////////////////////////////////////////////////////

//darmok menu class
function dkMenu()
{
    var self=this;

    //selected sub menu item
    this.idSelectSubmenu = null;

    //there are two selected menu icons at the top of the page
    //so that we can fade in the new selection and fade out the ould
    //cache whethe we are we using the 1 or 2 for the next select
    this.nSelect = 1;

    //the current select
    this.strCurrentSelect = "menuStreamBoost";

    this.init = function(parent)
    {
        var bReturn = false;

        if($("#Menu").length == 0)
        {
            var menu = "";

            menu += "<div id='topopen' class='SelectItem' style='display:none;'>";
            menu += "<div class='SelectRow'>";
            menu += "<img id='iconopen' class='SelectIcon' src='/images/selected_streamboost.png'>";
            menu += "<div id='submenuopen' class='SelectSub'>Network</div>";
            menu += "</div>";
            menu += "</div>";
            menu += "<div id='Menu' class='Menu'>";
            menu += "<img id='selectee' class='MenuIcon' style='position: absolute;display:none;z-index:75000;' src='/images/selected_streamboost.png'>";
            menu += "<div id='top1' class='SelectItem'>";
            menu += "<div class='SelectRow'>";
            menu += "<div id='label1' class='SelectLabel'>StreamBoost</div>";
            menu += "<img id='icon1' class='SelectIcon' src='/images/selected_streamboost.png'>";
            menu += "<div id='submenu1' class='SelectSub'>Network</div>";
            menu += "</div>";
            menu += "</div>";
            menu += "<div id='top2' class='SelectItem' style='display:none;'>";
            menu += "<div class='SelectRow'>";
            menu += "<div id='label2' class='SelectLabel'>StreamBoost</div>";
            menu += "<img id='icon2' class='SelectIcon' src='/images/selected_streamboost.png'>";
            menu += "<div id='submenu2' class='SelectSub'>Network</div>";
            menu += "</div>";
            menu += "</div>";
            menu += "<div id='SubMenu' class='SubMenu'>";
            menu += "</div>";
            menu += "<div id='MainMenu' class='MainMenu'>";
            menu += "</div>";
            menu += "</div>";

            $("#"+parent).append(menu);
        }

        bReturn = true;
        return bReturn;
    }


    //get menu id minus element parts
    this.toID = function(inID)
    {
        var id = inID;
        id = id.replace(/label/g, "");
        id = id.replace(/menu/g, "");
        id = id.replace(/icon/g, "");

        return id;
    }

    //set menu hover state
    this.state = function(id, bHover, strIcon)
    {
        //are we hovering?
        if(bHover)
        {
            //set hover style on label and icon
            $("#label"+id).addClass("MenuLabel_hover");
            $("#icon"+id).addClass("MenuIcon_hover");
        }
        else // nope
        {
            //set normal styles
            $("#label"+id).removeClass("MenuLabel_hover");
            $("#icon"+id).removeClass("MenuIcon_hover");
        }

        //set icon to the image in strIcon parameter
        $("#icon"+id).attr("src", "/images/"+strIcon);
    }

    //add a submenu item and return its id
    this.add = function(strLabel,
                        strIcon,
                        strIconHover,
                        clickHandler)
    {
        //make the id by removing spaces and prependinging sub
        var idMenu = strLabel.replace(/ /g, "").replace(/\//g, "");

        //make the html element for the sub item
        var item  = "<div id=\"menu"+idMenu+"\" class=\"MenuRow\">";
            item += "<div id=\"label"+idMenu+"\" class=\"MenuLabel\">"+_t(strLabel)+"</div>";
            item += "<img id=\"icon"+idMenu+"\"class=\"MenuIcon\" src=\"/images/"+strIcon+"\">";
            item += "</div>";

        //add it
        $("#MainMenu").append(item);

        //on label hover enter
        $("#label"+idMenu).hover(function(e)
            {
               self.state(self.toID(e.target.id),true,strIconHover);
            },
            function(e) // on hover leave
            {
               self.state(self.toID(e.target.id),false,strIcon);
            });

        //on menu div hover enter
        $("#menu"+idMenu).hover(function(e)
            {
               self.state(self.toID(e.target.id),true,strIconHover);
            },
            function(e) // on hover leave
            {
               self.state(self.toID(e.target.id),false,strIcon);
            });

        //on icon hover enter
        $("#icon"+idMenu).hover(function(e)
            {
               self.state(self.toID(e.target.id),true,strIconHover);
            },
            function(e) // on hover leave
            {
               self.state(self.toID(e.target.id),false,strIcon);
            });


        //when the label is clicked
        $("#label"+idMenu).click(function(e)
        {
            //do we need to call a handler?
            if(typeof clickHandler != 'undefined')
            {
                clickHandler();
            }

            self.clickItem(  e,
                        strLabel,
                        strIcon,
                        strIconHover);
        });

        //when the icon is clicked
        $("#icon"+idMenu).click(function(e)
        {
            //do we need to call a handler?
            if(typeof clickHandler != 'undefined')
            {
                clickHandler();
            }

            self.clickItem(  e,
                        strLabel,
                        strIcon,
                        strIconHover);
        });

        //hook up the resize function to lines
        $(window).resize(function(){
            self.redraw();
        });

        //return the items id
        return idMenu;
    }

    this.redraw = function()
    {
        //get document height
        var height = $(document).height();

        //set menu height to same as html document
        $("#Menu").css("height",height);
    }

    //function called when menu item is clicked
    //does the selecting and changes current data
    this.clickItem = function(  event,
                                strLabel,
                                strIcon,
                                strIconHover)
    {
        //store the previously selected item
        var prev = self.strCurrentSelect;

        //get the newly selected item
        self.strCurrentSelect = "menu"+self.toID(event.target.id);

        //fade out the new item
        $("#"+self.strCurrentSelect).fadeOut(function()
            {
                //fade in the old item
                $("#"+prev).fadeIn();
            });

        //new position
        var newposition = null;

        //there are two different select elements so we can
        //fade out the old and fade in the new
        //depending on which one is visible we do slightly
        //different elements fades
        if(self.nSelect == 1) // 1 is visible
        {
            $("#icon2").attr("src", "/images/"+strIcon);
            $("#label2").text(strLabel);
            $("#top1").finish().fadeOut(function() {
                $("#top2").finish().fadeIn();
            });
            self.nSelect = 2;
            newposition = $("#icon1").offset();
        }
        else // 2 is visible
        {
            $("#icon1").attr("src", "/images/"+strIcon);
            $("#label1").text(strLabel);
            $("#top2").finish().fadeOut(function() {
                $("#top1").finish().fadeIn();
            });
            self.nSelect = 1;
            newposition = $("#icon2").offset();
        }

        var currentIcon = self.strCurrentSelect.replace(/menu/g, "icon");

        //selectee is the elment used to draw the animation of selection
        //set the source for the select animation element so that it
        //has the icon of the selected item
        $("#selectee").attr("src","/images/"+strIcon);

        //get position of our menu element icon
        var position = $("#"+currentIcon).offset();

        //set the animation selection element to the same positionas our menu icon
        $("#selectee").css(position);

        //set to menu size icon
        $("#selectee").css("width","64px");

        //set selection element to make it visible
        $("#selectee").css("display","block");

        //stop any previous selection animations and start the new one
        //animate from the current icon position to the selection icon at the top
        //animate the size change from the small menu size to the large selection size
        $("#selectee").finish().animate({top:newposition.top,left:newposition.left,width:"128px"});

    }

    //add a submenu item and return its id
    this.addSub = function( idParent,
                            strLabel,
                            clickHandler)
    {
        var idDivSub = "sub"+idParent;
        var divSub = $("#"+idDivSub);

        //if the parent doesn't exist make it.
        if(divSub.length === 0)
        {
            //add it
            $("#SubMenu").append("<div id='"+idDivSub+"'></div>");
        }

        //make the id by removing spaces and prependinging sub
        var idSubMenu = "subitem"+strLabel.replace(/ /g, "").replace(/\//g, "").replace(/:/g, "");

        //make the html element for the sub item
        var item = "<div id=\""+idSubMenu+"\" class=\"SubItem\">"+_t(strLabel)+"</div>";

        //add it
        $("#"+idDivSub).append(item);

        //set the selection handler
        $("#"+idSubMenu).click(function(e)
        {
            //do we need to call a handler?
            if(typeof clickHandler != 'undefined')
            {
                clickHandler();
            }

            //set the selection
            $("#submenu").text($(e.target).text());
            self.selectSub(e.target.id);
        });

        //return the items id
        return idSubMenu;
    }

    //select a submenu item by id
    this.selectSub = function(idSubMenu)
    {
        //change the currently selected item to normal
        $("#"+self.idSelectSubmenu).removeClass("SelectSubMenu");
        $("#"+self.idSelectSubmenu).addClass("SubItem");

        //store our new selections
        self.idSelectSubmenu = idSubMenu;

        var strLabel = $("#"+self.idSelectSubmenu).text();

        //set the new selection to selected styles
        $("#"+idSubMenu).removeClass("SubItem");
        $("#"+idSubMenu).addClass("SelectSubMenu");

        if(self.nSelect == 1) // 1 is visible
        {
            $("#submenu1").text(strLabel);
        }
        else
        {
            $("#submenu2").text(strLabel);
        }

        //set the collapsed label
        $("#submenuopen").text(strLabel);
    }

    //select a submenu item by id
    this.select = function(idMenu)
    {
        var strLabel = $("#label"+idMenu).text();
        var strIcon = $("#icon"+idMenu).attr("src");

        $("#menu"+idMenu).css("display","none");

        $("#label1").text(strLabel);
        $("#label2").text(strLabel);

        $("#icon2").attr("src", strIcon);
        $("#icon1").attr("src", strIcon);
    }

    //local storage get
    this.lsGet = function(item)
    {
        var rt = null;

        rt = localStorage.getItem(item);

        return rt;
    }

    //local storage set
    this.lsSet = function(  item,
                            state)
    {
        localStorage.setItem(item, state);
    }

    //is the menu open?
    this.isOpen = function()
    {
        var bReturn = true;

        var strShow = self.lsGet("menushow");

        //if we don't have the local variable
        if(strShow == null)
        {
            self.lsSet("menushow","true");
        }
        else if(strShow=="false")//if the menu is closed
        {
            bReturn = false;
        }

        return bReturn
    }

    //show hide the menu
    this.open = function(bShow)
    {
        //if the menu is closed
        if(bShow==true)
        {
            $("#Menu").css("left","-0px");
            $("#topopen").css("display","none");
            self.lsSet("menushow","true");
        }
        else
        {
            $("#Menu").css("left","-1000px");
            $("#topopen").css("display","");
            self.lsSet("menushow","false");
        }
    }
}

//////////////////////////////////////////////////////////////
/*
    this is the functionality for drawing the darmok network
    graph page
*/
//////////////////////////////////////////////////////////////

function dkNetwork()
{
    var self = this;

    //the previous node data from /api/nodes
    this.lsPrevDevices = null;

    this.nDeviceID = 0;

    //row of the next device
    this.nDevRow = 0;

    //left or right for next device? left=0, right=1
    this.nDevLR = 0;

    //make default bw for meter
    this.nMaxUp = 125000000;
    this.nMaxDn = 125000000;

    //set the bw number
    sbBandwidth().done(function(nUp,nDown){

        //if new Max up BW
        if(nUp/8 != self.nMaxUp)
        {
            self.nMaxUp = parseInt(nUp/8);
        }

        //if new Max up BW
        if(nDown/8 != self.nMaxDn)
        {
            self.nMaxDn = parseInt(nDown/8);
        }
    });

    this.mapDevID2Row = [];

    this.idLine = 0;

    this.bHasResize = false;

    //this is the div we are drawing in
    this.strDiv = null;

    //this is our node hover popup
    this.nodeinfo = null;

    this.init = function(parent)
    {
        self.strDiv = parent;

        var origDisplay = $("#"+parent).css("display");
        $("#"+parent).css("display","none");

        //create flow table
        var strNetwork = "";

        strNetwork += "<img id='devhover' class='DeviceIcons' style='position: absolute;display:none;z-index:75000;' src='/images/device_desktop.png'>";
        strNetwork += "<table id='NetworkTable' style='width: 100%;height: 100%;text-align: center;padding: 64px;  min-width: 1140px;'>";
        strNetwork += "<tr class='networkrow'>";
        strNetwork += "<td style='width: 20%;'></td>";
        strNetwork += "<td style='width: 20%;'></td>";
        strNetwork += "<td><img id='interneticon' src='/images/device_globe.png' class='DeviceIcons'></td>";
        strNetwork += "<td style='width: 20%;'></td>";
        strNetwork += "<td style='width: 20%;'></td>";
        strNetwork += "</tr>";
        strNetwork += "<tr id='row' ldevid='-1' rdevid='-1' lmeterid='-1' rmeterid='-1' class='networkrow'>";
        strNetwork += "<td id='ldevice'></td>";
        strNetwork += "<td id='ltblmeter'></td>";
        strNetwork += "<td id='mmeter0'></td>";
        strNetwork += "<td id='rtblmeter'></td>";
        strNetwork += "<td id='rdevice'></td>";
        strNetwork += "</tr>";
        strNetwork += "<tr id='row0' ldevid='-1' rdevid='-1' lmeterid='-1' rmeterid='-1' class='networkrow'>";
        strNetwork += "<td id='ldevice0'></td>";
        strNetwork += "<td id='ltblmeter0'></td>";
        strNetwork += "<td><img id='routericon' src='/images/device_router.png' class='DeviceIcons'></td>";
        strNetwork += "<td id='rtblmeter0'></td>";
        strNetwork += "<td id='rdevice0'></td>";
        strNetwork += "</tr>";
        strNetwork += "</table>";

        //append it
        $("#"+self.strDiv).append(strNetwork);

        self.router();
        $("#"+parent).css("display",origDisplay);

        //make oure node info popup class
        self.nodeinfo = new NodeInfo(self.strDiv);
        self.nodeinfo.init();
    }

    //set up the router icon after we are ready
    this.router = function()
    {
        $("#routericon").unbind("click").click( function(e)
        {
            if(typeof callback != 'undefined')
            {
                callback();
            }
        });

        //on menu div hover enter
        $("#routericon").hover(function(e)
        {
            $("#"+self.hoverid).css("visibility","visible");
            $("#devhover").css("width", 96);
            var id = "routericon";
            self.hoverid = id;

            var strIcon = $("#"+id).attr("src");

            strIcon = strIcon.replace(/.png/g, "_hover.png");

            //set icon to the image in strIcon parameter
            //and when loaded show the animation of hover
            $("#devhover").one("load",function()
                {
                    //get position of our menu element icon
                    var position = $("#"+id).offset();
                    var newleft = position.left - 16;
                    var newtop = position.top - 16;
                    //set the animation selection element to the same positionas our menu icon
                    $("#devhover").css(position);
                    $("#"+id).css("visibility","hidden");
                    $("#devhover").css("display","block");
                    $("#devhover").stop().animate({width: 128,left: newleft, top:newtop},{duration:100});

                    /*
                    fixing code block will enable a router popup.
                    currently we don't have enough information to do this

                    //get position of our menu element icon
                    var position = $("#"+id).offset();

                    var x = parseInt(position.left)-($("#nodeinfo-hover").width()/2)+48;
                    var y = parseInt(position.top)-150;

                    var node = self.hoverid2info[id];

                    //show device info
                    self.nodeinfo.show( x,
                                        y,
                                        "Router",
                                        "192.168.1.1",
                                        false,
                                        "00:00:00:00:00",
                                        "Router",
                                        true);
                    */
            }).attr("src",strIcon);

        },
        function(e) // on hover leave
        {
            //nothing to do here please move on
        });
    }

    //call this function to get the latest devices and update the display
    this.updateDevices = function()
    {
        //get the nodes from the router
        $.getJSON("/cgi-bin/lil-ozker/api/nodes",
            function(lsNodes)
            {
                //loop over each node
                for(var node in lsNodes)
                {
                    //add/update the device
                    device( node.mac_addr,
                            node.strType,
                            node.rx_bytes,
                            node.tx_bytes);
                }
            });
    }

    //current device icon being hovered over
    //or clicked on
    this.hoverid="";

    this.updatedevice = function(   nID,
                                    nRxBytes,
                                    nTxBytes)
    {
        self.device( undefined,
                undefined,
                nRxBytes,
                nTxBytes,
                nID);
    }

    /*
        hover id to info

        {
            "Name": name,
            "IP": 0.0.0.0,
            "bStatic": true,
            "strMAC": "",
            "strType": "",
            "bWireless": true
        }
    */
    this.hoverid2info = [];

    // this is the one function to update/add a node to the display
    this.device = function( strMAC,
                            strIcon,
                            nRxBytes,
                            nTxBytes,
                            nID,
                            strName,
                            strTypeLabel,
                            handler,
                            strIP,
                            bStatic,
                            bWireless)
    {
        var nMyID = 0;
        var strMyName = _t("Unknown");
        var strMyType = _t("Unknown");

        //item we are going to add
        var strDevice = "";

        if(typeof strName != 'undefined')
        {
            strMyName = strName;
        }

        if(typeof strTypeLabel != 'undefined')
        {
            strMyType = strTypeLabel;
        }

        //does this item exist?
        if(typeof nID != 'undefined')
        {
            //have a valid id
            nMyID = nID;

            var nMID = self.mapDevID2Row[nMyID].nMeterID;

            //is this the right hand side??
            if(self.mapDevID2Row[nMyID].nDevLR == 1)
            {
                self.meters(null,nRxBytes,nTxBytes,self.nMaxDn,self.nMaxUp,nMID);
            }
            else //this is the left hand side
            {
                self.meters(null,nTxBytes,nRxBytes,self.nMaxUp,self.nMaxDn,nMID);
            }
        }
        else // else create it
        {
            nMyID = self.nDeviceID;

            //if this row exists we don't have to create it
            if($("#row"+self.nDevRow).length === 0)
            {
                var strRow = "";

                strRow += "<tr id='row"+self.nDevRow+"' ldevid='-1' rdevid='-1' lmeterid='-1' rmeterid='-1' class='networkrow'>";
                strRow += "<td id='ldevice"+self.nDevRow+"'>";
                strRow += "</td>";
                strRow += "<td id='ltblmeter"+self.nDevRow+"'>";
                strRow += "</td>";
                strRow += "<td id='middle"+self.nDevRow+"'>";
                strRow += "</td>";
                strRow += "<td id='rtblmeter"+self.nDevRow+"'>";
                strRow += "</td>";
                strRow += "<td id='rdevice"+self.nDevRow+"'>";
                strRow += "</td>";
                strRow += "</tr>";

                $("#NetworkTable").append(strRow);
            }

            //set the dev 2 row hand lookup
            self.mapDevID2Row[self.nDeviceID] =   {
                                            'nRow': self.nDevRow,
                                            'nDevLR': self.nDevLR,
                                            'nMeterID': -1,
                                            'nLine': -1
                                        };

            //get the xy center of the icon
            var ptLeft = null;

            //get the xy center of the arrows
            var ptRight = null;

            var id = "";
            //is this the right side?
            if(self.nDevLR == 1)
            {
                id = "rdevicon"+self.nDevRow;
                strDevice = "<img id='"+id+"' src='/images/"+strIcon+".png' class='DeviceIcons'>";
                strDevice +="<div class='DevLabel'>"+_t(strMyName)+" - "+_t(strMyType)+"</div>";

                $("#rdevice"+self.nDevRow).append(strDevice);

                var nMID = self.meters("#rtblmeter"+self.nDevRow,0,0,self.nMaxUp,self.nMaxDn);

                $("#row"+self.nDevRow).attr("rmeterid",nMID);
                $("#row"+self.nDevRow).attr("rdevid",self.nDeviceID);

                self.mapDevID2Row[self.nDeviceID].nMeterID = nMID;
                self.mapDevID2Row[self.nDeviceID].nDevLR = self.nDevLR;

                //change next to left
                self.nDevLR = 0;

                var nYAdjust = 0;//5;
                var nArrowWidth = 96;

                //get the xy center of the icon
                ptRight = getOffset($("#rdevice"+self.nDevRow));//.offset()
                ptRight.left += $("#rdevice"+self.nDevRow).width()/2;
                ptRight.top += ($("#rdevice"+self.nDevRow).height()/2) - nYAdjust;

                //get the xy center of the arrows
                ptLeft = getOffset($("#rtblmeter"+self.nDevRow));//.offset();
                ptLeft.left += ($("#rtblmeter"+self.nDevRow).width()/2) + nArrowWidth;
                ptLeft.top += ($("#rtblmeter"+self.nDevRow).height()/2) - nYAdjust;

                //increment the row for next add
                self.nDevRow++;

            }
            else // if not we added to the left
            {
                id = "ldevicon"+self.nDevRow;
                strDevice = "<img id='"+id+"'src='/images/"+strIcon+".png' class='DeviceIcons'>";
                strDevice +="<div class='DevLabel'>"+_t(strMyName)+" - "+_t(strMyType)+"</div>";

                $("#ldevice"+self.nDevRow).append(strDevice);

                var nMID = self.meters("#ltblmeter"+self.nDevRow,0,0,self.nMaxUp,self.nMaxDn);

                $("#row"+self.nDevRow).attr("lmeterid",nMID);
                $("#row"+self.nDevRow).attr("ldevid",self.nDeviceID);
                self.mapDevID2Row[self.nDeviceID].nMeterID = nMID;
                self.mapDevID2Row[self.nDeviceID].nDevLR = self.nDevLR;

                //change next to right
                self.nDevLR = 1;

                var nYAdjust = 0;//5;
                var nArrowWidth = 96;

                //get the xy center of the icon
                ptLeft = getOffset($("#ldevice"+self.nDevRow));//.offset();
                ptLeft.left += $("#ldevice"+self.nDevRow).width()/2;
                ptLeft.top += ($("#ldevice"+self.nDevRow).height()/2) - nYAdjust;
                //get the xy center of the arrows
                ptRight = getOffset($("#ltblmeter"+self.nDevRow));//.offset();
                ptRight.left += ($("#ltblmeter"+self.nDevRow).width()/2) - nArrowWidth;
                ptRight.top += ($("#ltblmeter"+self.nDevRow).height()/2) - nYAdjust;

            }

            self.devEvents(id,handler,nMyID);

            //add this guy to our hover info lookup
            self.hoverid2info[id] =
            {
                "name": strName,
                "ip": strIP,
                "static": bStatic,
                "mac": strMAC,
                "type": strTypeLabel,
                "wireless": bWireless
            };

            var strLine = "";

            //cache off the line
            self.mapDevID2Row[self.nDeviceID].nLine = self.idLine;

            //draw the line
            strLine += "<div id='lines"+(self.idLine)+"' class='hline' style='top: "+(ptLeft.top)+"px;left: "+(ptLeft.left)+"px;width:"+(ptRight.left - ptLeft.left)+"px;z-index: 0;'></div>";

            $("#Page").append(strLine);

            //increment the id
            self.nDeviceID++;
            self.idLine++;

            //initial
            self.redrawLines();
        }

        //need to wait for element to update before a redraw
        setTimeout(function()
        {
            //redraw the page
            self.redrawLines();
            //self.redraw();
        },
        50);

        return nMyID;
    }

    this.devEvents = function(id,handler)
    {

        //on menu div hover enter
        $("#"+id).hover(function(e)
        {
            $("#"+self.hoverid).css("visibility","visible");
            $("#devhover").css("width", 96);
            self.hoverid = id;

            var strIcon = $("#"+id).attr("src");

            strIcon = strIcon.replace(/.png/g, "_hover.png");

            //set icon to the image in strIcon parameter
            //and when loaded show the animation of hover
            $("#devhover").one("load",function()
                {
                    //get position of our menu element icon
                    var position = $("#"+id).offset();
                    var newleft = position.left - 16;
                    var newtop = position.top - 16;
                    //set the animation selection element to the same positionas our menu icon
                    $("#devhover").css(position);
                    $("#"+id).css("visibility","hidden");
                    $("#devhover").css("display","block");
                    $("#devhover").stop().animate({width: 128,left: newleft, top:newtop},{duration:100});

                    //get position of our menu element icon
                    var position = $("#"+id).offset();
                    var x = parseInt(position.left);

                    //left or right graph hover?
                    if(x>$("#NetworkTable").width()/2)
                    {
                        x = (x-$("#nodeinfo-hover").width())-50;
                    }
                    else
                    {
                        x+=128+25;
                    }

                    var y = parseInt(position.top);

                    var node = self.hoverid2info[id];

                    //show device info
                    self.nodeinfo.show( x,
                                        y,
                                        node.name,
                                        node.ip,
                                        node.static,
                                        node.mac,
                                        node.type,
                                        node.wireless);
            }).attr("src",strIcon);

        },
        function(e) // on hover leave
        {
            //nothing to do here please move on
        });

        //on menu div hover enter
        $("#devhover").unbind("hover").hover(function(e)
        {
            $("#devhover").css("display","block");
        },
        function(e) // on hover leave
        {
            $("#"+self.hoverid).css("visibility","visible");
            $("#"+id).css("visibility","visible");
            $("#devhover").css("display","none");
            $("#devhover").css("width", 96);
            $("#devhover").stop();

            //hide device info
            self.nodeinfo.hide()

        });

        //on menu div hover enter
        $("#devhover").unbind("click").click(function(e)
        {
            if(typeof handler != 'undefined')
            {
                var id      = "router";
                var nDevLR  = 0;
                var nRow    = self.hoverid.replace("rdevicon","").replace("ldevicon","");

                if(self.hoverid.indexOf("rdevicon")!=-1)
                {
                    nDevLR = 1;
                }

                for(var x in self.mapDevID2Row)
                {
                    if( self.mapDevID2Row[x].nDevLR == nDevLR &&
                        self.mapDevID2Row[x].nRow   == nRow)
                    {
                        id = x;
                        break;
                    }
                }

                handler(id,e);
            }
        });
    }

    //delete the device view for the given ID
    this.delDevice = function(nID)
    {
        //get the device to delete
        var strDevice = "";
        var strMeter = "";
        var strLine = "";

        var nOrignalLR = self.mapDevID2Row[nID].nDevLR;
        if(nOrignalLR == 1)
        {
            strDevice = "rdevice"+self.mapDevID2Row[nID].nRow;
            strMeter = "rtblmeter"+self.mapDevID2Row[nID].nRow;
        }
        else
        {
            strDevice = "ldevice"+self.mapDevID2Row[nID].nRow;
            strMeter = "ltblmeter"+self.mapDevID2Row[nID].nRow;
        }

        strLine = "lines"+self.mapDevID2Row[nID].nLine;

        $("#"+strDevice).empty();
        $("#"+strMeter).empty();
        $("#"+strLine).remove();

        //walk the rows
        for (var nRow = self.mapDevID2Row[nID].nRow+1; nRow <= self.nDevRow; nRow++)
        {
            //default to left
            var LR = "l";

            //if right change sides
            if(nOrignalLR == 1)
            {
                LR = "r";
            }

            //make src/dst l/r variables
            var dstDevice   = LR+"device"+(nRow-1);
            var dstMeter    = LR+"tblmeter"+(nRow-1);
            var srcDevice   = LR+"device"+nRow;
            var srcMeter    = LR+"tblmeter"+nRow;
            var nDstID      = $("#row"+(nRow-1)).attr(LR+"devid");
            var nSrcID      = $("#row"+nRow).attr(LR+"devid");
            var nDstLine      = self.mapDevID2Row[nDstID].nLine;

            if(nSrcID == -1)
            {
                        break;
            }

            var nSrcLine      = self.mapDevID2Row[nSrcID].nLine;

            //copy to dest
            var clnDvc = $("#"+srcDevice).html();
            var clnMtr = $("#"+srcMeter).html();

            $("#"+dstDevice).html(clnDvc);
            $("#"+dstMeter).html(clnMtr);

            //delete source
            $("#"+srcDevice).empty();
            $("#"+srcMeter).empty();

            //set the dev 2 row hand lookup
            self.mapDevID2Row[nDstID].nRow       = (nRow-1);
            self.mapDevID2Row[nDstID].nDevLR     = nOrignalLR;
            self.mapDevID2Row[nDstID].nMeterID   = nSrcID;
            //self.mapDevID2Row[nDstID].nLine      = nSrcLine;

            var devid = $("#row"+nRow).attr(LR+"devid");
            var meterid = $("#row"+nRow).attr(LR+"meterid");
            $("#row"+(nRow-1)).attr(LR+"devid",devid);
            $("#row"+(nRow-1)).attr(LR+"meterid",meterid);


            var oldid = LR+"devicon"+(nRow);
            var newid = LR+"devicon"+(nRow-1);
            $("#"+oldid).attr("id",newid);

            self.devEvents(newid);
        }

        $("#row"+(nRow-1)).attr(LR+"devid","-1");
        $("#row"+(nRow-1)).attr(LR+"meterid","-1");
        $("#row"+nRow).attr(LR+"devid","-1");
        $("#row"+nRow).attr(LR+"meterid","-1");

        //copy last left right?
        if( $("#row"+nRow).attr(LR+"devid") == "-1" &&
            $("#row"+(nRow-1)).attr(LR+"devid") == "-1")
        {
            var newLR = "l";

            if(LR == "l")
            {
                newLR = "r";
            }

            var dstDevice   = LR+"device"+(nRow-1);
            var dstMeter    = LR+"tblmeter"+(nRow-1);
            var srcDevice   = newLR+"device"+nRow;
            var srcMeter    = newLR+"tblmeter"+nRow;

            //copy to dest
            var clnDvc = $("#"+srcDevice).html();
            var clnMtr = $("#"+srcMeter).html();

            $("#"+dstDevice).html(clnDvc);
            $("#"+dstMeter).html(clnMtr);

            $("#"+srcDevice).empty();
            $("#"+srcMeter).empty();

            self.mapDevID2Row[nDstID].nRow       = (nRow-1);
            self.mapDevID2Row[nDstID].nDevLR     = nOrignalLR;

           $("#row"+(nRow-1)).attr(LR+"devid",$("#row"+nRow).attr(newLR+"devid"));
           $("#row"+(nRow-1)).attr(LR+"meterid",$("#row"+nRow).attr(newLR+"meterid"));

           $("#row"+nRow).remove();

            var oldid = newLR+"devicon"+(nRow);
            var newid = LR+"devicon"+(nRow-1);
            $("#"+oldid).attr("id",newid);

            self.devEvents(newid);
        }

        //del last row?
        if($("#row"+(nRow-1)).attr("ldevid") == "-1" && $("#row"+(nRow-1)).attr("rdevid") == "-1")
        {
            $("#row"+(nRow-1)).remove();
        }
        //del last row?
        if($("#row"+nRow).attr("ldevid") == "-1" && $("#row"+nRow).attr("rdevid") == "-1")
        {
            $("#row"+nRow).remove();
        }
    }

    this.redrawLines = function()
    {
        var nLn = 0;
        var nYAdjust = 55;;
        var nArrowWidth = 96;
        for (var nRow = 0; nRow <= self.nDevRow; nRow++)
        {
            //get the id's of the devices and meters from the row attributes added
            //for just such on occasion.
            var ldevid      = parseInt($("#row"+nRow).attr("ldevid"));
            var rdevid      = parseInt($("#row"+nRow).attr("rdevid"));
            var lmeterid    = parseInt($("#row"+nRow).attr("lmeterid"));
            var rmeterid    = parseInt($("#row"+nRow).attr("rmeterid"));

            //do right line
            if( isNaN(ldevid) == false && isNaN(lmeterid) == false &&ldevid !=  -1 && lmeterid != -1)
            {
                //do right line
                //get the xy center of the icon
                ptLeft = $("#ldevicon"+nRow).offset();

                if(typeof ptLeft != 'undefined')
                {
                    ptLeft.left += $("#ldevicon"+nRow).width()/2;
                    ptLeft.top += nYAdjust;

                    //get the xy center of the arrows
                    ptRight = $("#rmeter"+lmeterid).offset();
                    ptRight.left += ($("#rmeter"+lmeterid).width()/2) - nArrowWidth;

                    var line = "#lines"+self.mapDevID2Row[ldevid].nLine;
                    //move the line
                    $("#lines"+self.mapDevID2Row[ldevid].nLine).css("top",ptLeft.top+"px");
                    $("#lines"+self.mapDevID2Row[ldevid].nLine).css("left",ptLeft.left+"px");

                    //resize the line
                    var width = ptRight.left - ptLeft.left;

                    $("#lines"+self.mapDevID2Row[ldevid].nLine).css("width",width+"px");
                }
            }

            nLn++;

            //do left line
            if( isNaN(rdevid) == false && isNaN(rmeterid) == false && rdevid !=  -1 && rmeterid != -1)
            {
                //get the xy center of the icon
                ptRight = $("#rdevicon"+nRow).offset()

                if(typeof ptRight != 'undefined')
                {
                    ptRight.left += $("#rdevicon"+nRow).width()/2;
                    ptRight.top += nYAdjust;

                    //get the xy center of the arrows
                    ptLeft = $("#lmeter"+rmeterid).offset();
                    ptLeft.left += ($("#lmeter"+rmeterid).width()/2) + nArrowWidth;

                    var line = "#lines"+self.mapDevID2Row[rdevid].nLine;

                    //move the line
                    $("#lines"+self.mapDevID2Row[rdevid].nLine).css("top",ptRight.top+"px");
                    $("#lines"+self.mapDevID2Row[rdevid].nLine).css("left",ptLeft.left+"px");

                    //resize the line
                    var width = ptRight.left - ptLeft.left;
                    $("#lines"+self.mapDevID2Row[rdevid].nLine).css("width",width+"px");
                }
            }

            nLn++;
        }

        //does the router line exist??
        if($("#routerline").length == 0)
        {
            //add default rauter line
            var strLine =  "<div id='routerline' class='vline'></div>";

            $("#Page").append(strLine);
        }

        //get the icons for router and internet
        var ptNet       = $("#interneticon").offset();
        var ptRouter    = $("#routericon").offset();

        //calc top of router line to middle of internet icon
        var rlinetop = ptNet.top + nYAdjust;

        //calc line x position to middle of internet icon
        var rlineleft = ptNet.left + nYAdjust-9;

        //length of router line is from mid of interent icon to mid of router icon
        var rlineheight = ptRouter.top - ptNet.top;

        //move/size the router line
        $("#routerline").css("top",rlinetop);
        $("#routerline").css("left",rlineleft);
        $("#routerline").css("height",rlineheight);

        //have we hooked up our line redraw to resizes yet?
        if(self.bHasResize == false)
        {
            self.bHasResize = true;
            //hook up the resize function to lines
            $(window).resize(function(){
                self.redrawLines();
            });
        }
    }

    this.nMeterID = 0;

    //this function draws the bw meter set in bits per second
    //this function will automatically scale to bps,Kbps,Mbps,Gbps
    //as needed to fit the meters xxx.x 3 digit one decimal size
    this.meters = function( strDiv,
                            nLeft,
                            nRight,
                            nMaxLeft,
                            nMaxRight,
                            nID,
                            nMeterWidth)
    {
        //html to defend
        var strMeters = "";

        var nLPercent = nLeft/nMaxLeft;
        var nRPercent = nRight/nMaxRight;

        //default meter width 96 pixels
        var nWidth = 96;
        var nBorderWidth = 0;

        //if this exists use the width
        if(typeof nMeterWidth != 'undefined')
        {
            nWidth = nMeterWidth;
        }

        //calc width of meter fills
        var nLWidth = Math.round(nWidth*nLPercent) + nBorderWidth;
        var nRWidth = Math.round(nWidth*nRPercent) + nBorderWidth;

        //make 3.1 human readable labels
        var strLLabel = bytesToSize(nLeft*8,1);
        var strRLabel = bytesToSize(nRight*8,1);

        var nMyID = 0;

        if(typeof nID != 'undefined')
        {
            nMyID = nID;
        }
        else
        {
            nMyID = ++self.nMeterID;

            //add left meter
            strMeters += "<div style='display:inline;'>";
            strMeters += "<div id='llabel"+nMyID+"' class='MeterText'>"+strLLabel+"</div>";
            strMeters += "<img id='lmeter"+nMyID+"' src='/images/Meter_Left_Full.png' class='bwArrows' style='position: absolute;clip: rect(0px,96px,32px,96px);z-index:19;'>";
            strMeters += "<img src='/images/Meter_Left.png' class='bwArrows'>";
            strMeters += "</div>";

            //add right meter
            strMeters += "<div style='display:inline;'>";
            strMeters += "<div id='rlabel"+nMyID+"' class='MeterText'>"+strRLabel+"</div>";
            strMeters += "<img id='rmeter"+nMyID+"' src='/images/Meter_Right_Full.png' class='bwArrows' style='position: absolute;clip: rect(0px,0px,32px,0px);z-index:19;'>";
            strMeters += "<img src='/images/Meter_Right.png' class='bwArrows'>";
            strMeters += "</div>";

            //add it
            $(strDiv).append(strMeters);

            //init both meters to 0
            $("#lmeter"+nMyID).css('fontSize',96);
            $("#rmeter"+nMyID).css('fontSize',0);
        }


        $("#lmeter"+nMyID).stop().animate({
          fontSize: nWidth-nLWidth //some unimportant CSS to animate so we get some values
        },
        {
            duration: 1000,
            step: function(now, fx) { //now is the animated value from initial css value
                $(this).css('clip','rect(0px,96px,32px,'+now+'px)');
          }
        });

        $("#rmeter"+nMyID).stop().animate({
          fontSize: nRWidth //some unimportant CSS to animate so we get some values
        },
        {
            duration: 1000,
            step: function(now, fx) { //now is the animated value from initial css value
                $(this).css('clip', 'rect(0px,'+now+'px,32px,0px)');
          }
        });

        $("#llabel"+nMyID).text(strLLabel);
        $("#rlabel"+nMyID).text(strRLabel);

        return self.nMeterID;
    }


    //this function draws the vertical bw meter set in bits per second
    //this function will automatically scale to bps,Kbps,Mbps,Gbps
    //as needed to fit the meters xxx.x 3 digit one decimal size
    this.bandwidth = function(  strDiv,
                                nUp,
                                nDn,
                                nMaxUp,
                                nMaxDn,
                                nID,
                                nMeterHeight)
    {
        //html to defend
        var strMeters = "";

        var nUpPercent = nUp/nMaxUp;
        var nDnPercent = nDn/nMaxDn;

        //default meter width 96 pixels
        var nHeight = 96;
        var nBorderHeight = 0;

        //if this exists use the width
        if(typeof nMeterHeight != 'undefined')
        {
            nHeight = nMeterHeight;
        }

        //calc width of meter fills
        var nUpHeight = Math.round(nHeight*nUpPercent) + nBorderHeight;
        var nDnHeight = Math.round(nHeight*nDnPercent) + nBorderHeight;

        //make 3.1 human readable labels
        var strUpLabel = bytesToSize(nUp*8,1);
        var strDnLabel = bytesToSize(nDn*8,1);

        var nMyID = 0;

        if(typeof nID != 'undefined')
        {
            nMyID = nID;
        }
        else
        {
            nMyID = ++self.nMeterID;

            //add left meter
            strMeters += "<div style='display:block;'>";
            strMeters += "<div id='uplabel"+nMyID+"' class='VMeterText'>"+strUpLabel+"</div>";
            strMeters += "<img id='upmeter"+nMyID+"' src='/images/Meter_Up_Full.png' class='bwVArrows' style='position: absolute;clip: rect(0px,0px,32px,0px);z-index:19;'>";
            strMeters += "<img src='/images/Meter_Up.png' class='bwVArrows'>";
            strMeters += "</div>";

            //add right meter
            strMeters += "<div style='display:block;'>";
            strMeters += "<div id='dnlabel"+nMyID+"' class='VMeterText'>"+strDnLabel+"</div>";
            strMeters += "<img id='dnmeter"+nMyID+"' src='/images/Meter_Down_Full.png' class='bwVArrows' style='position: absolute;clip: rect(0px,32px,0px,0px);z-index:19;'>";
            strMeters += "<img src='/images/Meter_Down.png' class='bwVArrows'>";
            strMeters += "</div>";

            //add it
            $(strDiv).append(strMeters);

            //init both meters to 0
            $("#upmeter"+nMyID).css('fontSize',96);
            $("#dnmeter"+nMyID).css('fontSize',0);
        }


        $("#upmeter"+nMyID).stop().animate({
          fontSize: nHeight-nUpHeight //some unimportant CSS to animate so we get some values
        },
        {
            duration: 1000,
            step: function(now, fx) { //now is the animated value from initial css value
                $(this).css('clip','rect('+now+'px,32px,200px,0px)');
          }
        });

        $("#dnmeter"+nMyID).stop().animate({
          fontSize: nDnHeight //some unimportant CSS to animate so we get some values
        },
        {
            duration: 1000,
            step: function(now, fx) { //now is the animated value from initial css value
                $(this).css('clip', 'rect(0px,32px,'+now+'px,0px)');
          }
        });

        $("#uplabel"+nMyID).text(strUpLabel);
        $("#dnlabel"+nMyID).text(strDnLabel);

        return self.nMeterID;
    }
}



//////////////////////////////////////////////////////////////
/*
    this is the functionality for drawing the
    Airtime Fairness Performance view
*/
//////////////////////////////////////////////////////////////


/*
    class: dkATFView

    this draws a table to be used for Airtime Performance
    view and hase all the utilities to update it.
*/
function dkATFView()
{
    /*
        variable: self

        better this for scoping reasons
    */
    var self = this;


    /*
        variable: strDiv
        the parent div to draw in
    */
    this.strDiv = "";

    /*
        function: init

        initialize/create the ATF View Table

        parameters:
        strDiv - parent div to draw in

        returns:
        nothing.
    */
    this.init = function(strDiv,strName)
    {
        self.strDiv = strDiv;

        //if we need to append our empty table
        if($('#ATFView').length == 0)
        {
            var strATF = "";

            //create the top graph with big router icon
            strATF += "<table id='NodeTable' border'0' cellspacing='0' cellpadding='0' style='margin-bottom:25px;'>";
            strATF += "<tr class='networkrow' style='height: 240px;background-color:#8b8f95'>";
            strATF += "<td style='width: 240px; background-color:#81868c;border-radius:10px 0px 0px 10px;border: 0px;padding:0;'>";
            strATF += "<table id='LabelTable' style='width: 100%; height: 100%; padding: 0; border-collapse: collapse; border-spacing: 0px;min-width: 240px;' border='0' cellspacing='0' cellpadding='0'>";
            strATF += "<tr class='networkrow' style='height: 40px;background-color:#498ba0'>";
            strATF += "<td style='border: 0px;margin: 0px;border-radius: 10px 0px 0px 0px;font-family: \"Segoe UI\", sans-serif; font-weight: bold; color: white;'>";
            strATF += strName;
            strATF += "</td></tr>";
            strATF += "<tr class='networkrow' style='height: 200px;'><td style='height: 200px; border: 0px; margin: 0px;'>";
            strATF += "<img id='deviceicon' src='/images/device_router.png'>";
            strATF += "</td></tr>";
            strATF += "</table>";
            strATF += "</td>";
            strATF += "<td style='background-color:#8b8f95;border-radius:0px 10px 10px 0px;border: 0px;color:white;'>";
            strATF += "Percantage of Airtime over the last 60 seconds";
            strATF += "<canvas id='chartpersec' style='font-family: \"Segoe UI\", sans-serif; font-size: 12px;fill:white;width: 90%; height: 200px;'' height='200' width='900'></canvas>";
            strATF += "</td>";
            strATF += "</tr>";
            strATF += "</table>";

            //create our table header and container
            strATF += "<div style='color:white; border-style: solid; border-bottom: solid 1px #ffffff;border-top: none;\
                      border-left: none;border-right: none;min-width: 800px; max-width: 1200px;margin: auto;margin-bottom:25px;'>";
            strATF += "Performance View of SSID Networks and their Devices by MAC";
            strATF += "</div>";

            //this is where the table draws
            strATF += "<table id='ATFView' border='0' cellspacing='0' cellpadding='0' style='width: 100%;min-width: 800px;max-width: 1200px;margin: auto;'>";
            strATF += "</table>";
            strATF += "</div>";

            //add this the the page
            $("#"+self.strDiv).append(strATF);

            //can't draw the graph till everything is fully loaded
            //this waits for all images/html/json requests then draws
            //make sure your page start with window.onload not document.ready
            self.chart = self.makeChart("chartpersec",
                                        "rgba(73,139,160,0.2)",
                                        "rgba(73,139,160,1)",
                                        "rgba(73,139,160,1)",
                                        "#fff",
                                        100);
            //do 1st draw
            self.chart.update();
        }
    }

    /*
        variable: chart

        this is the line graph next to the router on
        the top of the page
    */
    this.chart = undefined;


    /*
        function: makeChart

        make a line graph to update for the last 60 seconds

        parameters:
        divid               - canvas div to draw in
        fillcolor           - fill color of the line
        strokecolor         - stroke on edge of line chart
        pointcolor          - color of points for each sample
        pointstrokecolor    - outline color of point
        nMyMax              - max value in the graph
    */
    this.makeChart = function(  divid,
                                fillcolor,
                                strokecolor,
                                pointcolor,
                                pointstrokecolor,
                                nMyMax)
    {
        //data for the graph
        var myData = [];

        //labels on the bottom of the graph
        var myLabels = [];

        //make the labels for the bottom side of the chart
        for(var n = 60; n>=0;n--)
        {
            var label = "";
            if((parseInt(n/10)*10) == n)
            {
                label = n.toString();
            }

            myLabels.push(label);

            myData.push(0);
        }

        //config for the chart
        var lineChartData = {
            labels : myLabels,
            datasets : [
                {
                    label: "My First dataset",
                    fillColor : fillcolor,
                    strokeColor : strokecolor,
                    pointColor : pointcolor,
                    pointStrokeColor : pointstrokecolor,
                    pointHighlightFill : "#fff",
                    pointHighlightStroke : "rgba(220,220,220,1)",
                    data : myData,
                    showXLabels: 10
                }
            ]
        };

        //get the canvase to draw in
        var ctx = document.getElementById(divid).getContext("2d");

        //number of left hand keys on graph
        var nKeys = 4;

        //make it!
        var myChart = new Chart(ctx).Line(lineChartData,
        {
            responsive: false,
            scaleOverride: true,
            scaleStartValue: 0,
            scaleStepWidth: (nMyMax)/nKeys,
            scaleSteps: nKeys,
            animationSteps: 1,
            scaleFontColor: "#fff",
            pointDot : false,
            animation: true,
            showTooltips: false
        });

        return myChart;
    }

    /*
        function: updateChart

        add a new sample value to the graph

        parameters:
        chart       - the id of the chart to add to (returned by makeChart)
        nAddValue   - float value to add
    */
    this.updateChart = function(nAddValue)
    {
        var data = self.chart.datasets[0].points;
        //remove old data
        for(var n = 0; n<60;n++)
        {
            data[n].value = data[n+1].value;
        }
        //add new value
        data[60].value = nAddValue;
        self.chart.update();
    }

    /*
        variable: nCellWidth

        the width of cells in the below table
    */
    this.nCellWidth = 150;

    /*
        variable: nItemID

        the UUID of each item in the cell
    */
    this.nItemID = 0;
    this.nChildID = 0;

    /*
        function: addSSID

        add an ssid to the table

        parameters:
        strSSID     - the ssid to add
        strChan     - channel BW
        strBitRate  - bits per sec
        nReserve    - % of airtime to reserve
        nCurrent    - % of airtime to used

        returns:
        the ID string of the added element
    */
    this.addSSID = function(strSSID,
                            strChan,
                            strBitRate,
                            nCurrent,
                            nReserve)
    {
        var nMyID = self.nItemID++;
        var strItem = "";

        nCurrent = parseFloat(nCurrent).toFixed(1);
        nReserve = parseFloat(nReserve).toFixed(1);

        //create the HTML for the ssid row
        strItem += "<tr id='item"+nMyID+"' class='devicerow' ssid='"+strSSID+"'>";
        strItem += "<td class='ssid'>";
        strItem += "<table border='0' cellspacing='0' cellpadding='0' style='width: 100%;'>";
        strItem += "<tr>";
        strItem += "<td style='width: 32px;'>";
        strItem += "<img id='deviceicon' src='/images/device_router.png' style='height: 72px;position: absolute;\
                    margin-top: -36px;margin-left: -16px;z-index: 128;padding-left:10px;'>";
        strItem += "</td>";
        var nCell = ((self.nCellWidth*2)-13);
        strItem += "<td id='ssid"+nMyID+"' style='padding-left: 115px;text-align:left;width:"+nCell+"px;min-width:"+nCell+"px;padding-right: 22px;'>";
        strItem += "SSID: "+strSSID;// + " - "+strChan+" / "+strBitRate+"";
        strItem += "</td>";

        var meterID = nMyID;

        //draw the meter
        strItem += "<td id='labelUp"+nMyID+"' style='padding-right:0;'>";

        strItem += "<div id='reserve"+nMyID+"' style='width:100px;text-align:left;cursor: pointer;\
                    height:36px;line-height:36px;padding-left:10px; ;border-right-color: black;border-right-style: solid;border-right-width: 1px;padding-right: 0px;'>";
        strItem += "Reserve: "+nReserve+"%";
        strItem += "</div>";
        strItem += "</td>";
        strItem += "<td style='padding: 0;width:70%;padding-right:32px;'>";
        strItem += "<div id='trackMac"+meterID+"' style='border-radius: 0px 10px 10px 0px; background-color:white;width: 100%;margin-right: 25px;height:20px;'>";
        strItem += "</div>";
        strItem += "</td>";
        //end meter

        strItem += "<td style='min-width:25%;max-width:50%;color:white;'>";
        strItem += " ";
        strItem += "</td>";

        strItem += "<td id='openclose"+nMyID+"' style='width:48px;cursor: pointer;'>";
        strItem += "<img id='openclose"+nMyID+"' src='/images/down_arrow.png' width='16' style='position: absolute;\
                    margin-left: -18px;margin-top: -8px;cursor: pointer;'>";
        strItem += "</td>";
        strItem += "</tr>";
        strItem += "</table>";
        strItem += "</td>";
        strItem += "</tr>";

        //add empty child row where our MAC's go
        strItem += "<tr class='childrow'>";
        strItem += "<td style='height:100%;min-height:64px;padding-top: 0px;'>";
        strItem += "<div style='border-radius: 0px 0px 10px 10px; background-color:#8b8f95;\
                    height: 100%;margin-left: 25px;margin-right: 25px;color:white;'>";
        strItem += "<table id='children"+nMyID+"' border=0' cellspacing='0' cellpadding='0' style='width:100%;'>";
        strItem += "</table>";
        strItem += "</div>";
        strItem += "</td>";
        strItem += "</tr>";

        //add this to the page in the Device table ID
        //created in the init() function
        $("#ATFView").append(strItem);

        var strMeters = "";
        var nWidth = $("#trackMac"+meterID).width();
        var nMyCur = nCurrent;

        //is the current over the reserve limit?
        if(nCurrent > nReserve && nReserve > 0)
        {
            //if so set the current to the reserve
            //so the we can draw the overage
            nMyCur = nReserve;
        }

        //calculate the meter lengths for reserve, current, and overage
        var nResvW = parseFloat(nReserve/100.0)*nWidth;
        var nCurrW = parseFloat(nMyCur/100.0)*nWidth;
        var nOverW = parseFloat(nCurrent/100.0)*nWidth;

        //over label vars
        var nOver = 0.0;
        var strLabel = "";

        //if we have a overage
        if(nReserve > 0 && (nCurrent-nReserve) > 0)
        {
            //make our over label
            nOver = (nCurrent-nReserve);

            strLabel = ""+nCurrent+"% ( "+nOver+"% over reserve )";
        }
        else //if no overage
        {
            //make the normal label
            strLabel = ""+nMyCur+"%";
        }

        //make our meters
        strMeters += "<div id='meterMacResv"+meterID+"' percent="+nReserve+" style='border-radius: 0px 0px 0px 0px; background-color:#aaaaaa;width:"+nResvW+"px;height:20px;position:absolute;'></div>";
        strMeters += "<div id='meterMacOver"+meterID+"'percent="+nCurrent+" style='border-radius: 0px 10px 10px 0px; background-color:#ffb400;width:"+nOverW+"px;height:20px;position:absolute;'></div>";
        strMeters += "<div id='meterMacCrnt"+meterID+"' percent="+nCurrent+" style='border-radius: 0px 10px 10px 0px; background-color:#498ba0;width:"+nCurrW+"px;height:20px;position:absolute;'></div>";
        strMeters += "<div id='meterMacLabel"+meterID+"' style='background-color:transparent;width:"+nWidth+"px;height:20px;position:absolute;color:black;'>"+strLabel+"</div>";

        //add the meters to the page
        $("#trackMac"+meterID).append(strMeters);

        //when the window changes size
        //reddraw the meters to fit their track
        $(window).resize(function()
        {
            var nWidth = $("#trackMac"+meterID).width();
            var nMyCur = nCurrent;

            if(nCurrent > nReserve && nReserve > 0)
            {
                nMyCur = nReserve;
            }

            var nResvW = parseFloat(nReserve/100.0)*nWidth;
            var nCurrW = parseFloat(nMyCur/100.0)*nWidth;
            var nOverW = parseFloat(nCurrent/100.0)*nWidth;

            var nOver = 0.0;
            if(nReserve > 0 && (nCurrent-nReserve) > 0)
            {
                nOver = (nCurrent-nReserve);;
            }
            $("#meterMacResv"+meterID).css("width",nResvW);
            $("#meterMacOver"+meterID).css("width",nOverW);
            $("#meterMacCrnt"+meterID).css("width",nCurrW);
            $("#meterMacLabel"+meterID).css("width",nWidth);
        });

        //set us to a defalt of 0 children
        //is this necessary on this page?
        //probably not.
        $("#item"+nMyID).attr("childID",0);

        //hook up the little open close arrow so that it works
        $("#openclose"+nMyID).click(function(event)
        {
            var id = event.target.id;
            var nID =id.replace("openclose","");

            var visible = $("#children"+nID).css("display");

            if(visible == "none")
            {
                $("#children"+nID).css("display","table");
                $("#openclose"+nID).children().attr("src", "/images/down_arrow.png");
            }
            else
            {
                $("#children"+nID).css("display","none");
                $("#openclose"+nID).children().attr("src", "/images/left_arrow.png");
            }
        });

        //return our id
        return nMyID;
    }

    /*
        function: addMAC

        add a mac to a parrent sssid in the table

        parameters:
        nItemID     - parent id to add this to - returned from addSSID()
        strMAC      - the mac to add
        nCurrent    - current live airtime
        nReserve    - % of airtime to reserve

        returns:
        the ID string of the added element
    */
    this.addMAC = function( nItemID,
                            strMAC,
                            nCurrent,
                            nReserve,
                            strName)
    {
        var strItem = "";

        //get id and increment for next add
        var nMyID       = self.nChildID++;
        var nMyParent   = nItemID;

        nCurrent = parseFloat(nCurrent).toFixed(1);
        nReserve = parseFloat(nReserve).toFixed(1);

        //create the html to append for a mac
        strItem += "<tr id='child"+nMyID+"' class='mac' style='height:72px;' mac='"+strMAC+"'>";
        strItem += "<td style='width: 32px;margin-top: 8px;'>";
        strItem += "<img id='deviceicon' src='/images/flow_default.png' style='height: 64px;position: absolute;\
                    margin-top: -32px;margin-left: -10px;z-index: 128;padding-left:10px;'>";
        strItem += "</td>";
        strItem += "<td style='text-align:left;width:"+self.nCellWidth*2+"px;padding-left:90px;min-width:"+self.nCellWidth*2+"px;'>";

        if(typeof(strName) != 'undefined')
        {
            strItem += "ID: "+strName+" / "+strMAC;
        }
        else
        {
            strItem += "ID: "+strMAC;
        }
        strItem += "</td>";

        var meterID = +nItemID+"-"+nMyID;

        //draw the meter
        strItem += "<td id='labelUp"+nMyID+"' style='padding-right:0;'>";

        strItem += "<div id='childrsv"+nMyID+"' style='width:100px;text-align:left;cursor: pointer;\
                    height:36px;line-height:36px;padding-left:10px; ;border-right-color: black;border-right-style: solid;border-right-width: 1px;padding-right: 0px;'>";
        strItem += "Reserve: "+nReserve+"%";
        strItem += "</div>";
        strItem += "</td>";
        strItem += "<td style='padding: 0;width:70%;padding-right:45px;'>";
        strItem += "<div id='trackMac"+meterID+"' style='border-radius: 0px 10px 10px 0px; background-color:white;width: 100%;margin-right: 25px;height:20px;'>";
        strItem += "</div>";
        strItem += "</td>";

        strItem += "</tr>";

        //add our mac to the parent ssid
        $("#children"+nItemID).append(strItem);

        var strMeters = "";
        var nWidth = $("#trackMac"+meterID).width();
        var nMyCur = nCurrent;

        //do we have an overage?
        if(nCurrent > nReserve && nReserve > 0)
        {
            nMyCur = nReserve;
        }

        //calucate the meter lengeth for reserve, current, and overage
        var nResvW = parseFloat(nReserve/100.0)*nWidth;
        var nCurrW = parseFloat(nMyCur/100.0)*nWidth;
        var nOverW = parseFloat(nCurrent/100.0)*nWidth;

        var nOver = 0.0;
        var strLabel = "";

        //make a label if we have an overage
        if(nReserve > 0 && (nCurrent-nReserve) > 0)
        {
            nOver = (nCurrent-nReserve);

            strLabel = ""+nCurrent+"% ( "+nOver+"% over reserve )";
        }
        else // if we don't make a normal label
        {
            strLabel = ""+nMyCur+"%";
        }

        //make the meter html
        strMeters += "<div id='meterMacResv"+meterID+"' percent="+nReserve+" style='border-radius: 0px 0px 0px 0px; background-color:#aaaaaa;width:"+nResvW+"px;height:20px;position:absolute;'></div>";
        strMeters += "<div id='meterMacOver"+meterID+"'percent="+nCurrent+" style='border-radius: 0px 10px 10px 0px; background-color:#ffb400;width:"+nOverW+"px;height:20px;position:absolute;'></div>";
        strMeters += "<div id='meterMacCrnt"+meterID+"' percent="+nCurrent+" style='border-radius: 0px 10px 10px 0px; background-color:#498ba0;width:"+nCurrW+"px;height:20px;position:absolute;'></div>";
        strMeters += "<div id='meterMacLabel"+meterID+"' style='background-color:transparent;width:"+nWidth+"px;height:20px;position:absolute;color:black;'>"+strLabel+"</div>";

        //append the meters to the page
        $("#trackMac"+meterID).append(strMeters);

        //if the window changes size redraw the meters to fit
        $(window).resize(function()
        {
            var nWidth = $("#trackMac"+meterID).width();
            var nMyCur = nCurrent;

            if(nCurrent > nReserve && nReserve > 0)
            {
                nMyCur = nReserve;
            }

            var nResvW = parseFloat(nReserve/100.0)*nWidth;
            var nCurrW = parseFloat(nMyCur/100.0)*nWidth;
            var nOverW = parseFloat(nCurrent/100.0)*nWidth;

            var nOver = 0.0;
            if(nReserve > 0 && (nCurrent-nReserve) > 0)
            {
                nOver = (nCurrent-nReserve);;
            }
            $("#meterMacResv"+meterID).css("width",nResvW);
            $("#meterMacOver"+meterID).css("width",nOverW);
            $("#meterMacCrnt"+meterID).css("width",nCurrW);
            $("#meterMacLabel"+meterID).css("width",nWidth);
        });

        //hook the defaults() function
        $("#childDefaults"+nMyID).click({nMyID: nMyID},function()
        {

            var divHtml = delabel($("#reserve"+nMyID).html()).replace("%",""); // notice "this" instead of a specific #myDiv

            self.nPrevValue = parseInt(divHtml);

            $("#childrsv"+nMyID).text("Reserve: 0%");

            //if we have a listener call hime and let him know about the table changes
            if(typeof self.pOnChange != 'undefined' && self.pOnChange != null && 0 != self.nPrevValue)
            {
                var id = $("#child"+nMyID).attr("mac");
                self.pOnChange(nItemID,nMyID,0,id);
            }
        });

        return nMyID;
    }

    /*
        function: updateSSID

        update the current and reserve of the given ssid

        parameters:
        nID      - ID returned by addSSID
        nCurrent - current percent of radio time persec used by the ssid
        nReserve - reserved radio percent guarunteed
    */
    this.updateSSID = function( nID,
                                nCurrent,
                                nReserve)
    {
        var meterID = nID;
        var nWidth = $("#trackMac"+meterID).width();
        var nMyCur = nCurrent;

        //get the overage
        if(nCurrent > nReserve && nReserve > 0)
        {
            nMyCur = nReserve;
        }

        //calculate the meter lengths
        var nResvW = parseFloat(nReserve/100.0)*nWidth;
        var nCurrW = parseFloat(nMyCur/100.0)*nWidth;
        var nOverW = parseFloat(nCurrent/100.0)*nWidth;

        var nOver = 0.0;

        //calc the overage
        if(nReserve > 0 && (nCurrent-nReserve) > 0)
        {
            nOver = (nCurrent-nReserve);;
        }

        //update the meters with their new lengths (label resize too)
        $("#meterMacResv"+meterID).stop().animate({width: nResvW});
        $("#meterMacOver"+meterID).stop().animate({width: nOverW});
        $("#meterMacCrnt"+meterID).stop().animate({width: nCurrW});
        $("#meterMacLabel"+meterID).css("width",nWidth);

        var strLabel = "";

        //make an overage label if necessary
        if(nReserve > 0 && (nCurrent-nReserve) > 0)
        {
            nOver = (nCurrent-nReserve);

            strLabel = ""+parseFloat(nCurrent).toFixed(1)+"% ( "+parseFloat(nOver).toFixed(1)+"% over reserve )";
        }
        else //if not make a normal label
        {
            strLabel = ""+parseFloat(nMyCur).toFixed(1)+"%";
        }

        //update the labels as needed
        $("#meterMacLabel"+meterID).text(strLabel);
        $("#reserve"+nID).text("Reserve: "+parseFloat(nReserve).toFixed(1)+"%");

    }

    /*
        function: updateMAC

        update the current and reserve of the given MAC of the parent SSID

        parameters:
        nParentID - id returned by addssid of the parent
        nID       - ID returned by addMAC of the item
        nCurrent  - current percent of radio time persec used by the ssid
        nReserve  - reserved radio percent guarunteed
    */
    this.updateMAC = function(  nParentID,
                                nID,
                                nCurrent,
                                nReserve)
    {
        var meterID = +nParentID+"-"+nID;
        var nWidth = $("#trackMac"+meterID).width();
        var nMyCur = nCurrent;

        //get current minus the overage
        if(nCurrent > nReserve && nReserve > 0)
        {
            nMyCur = nReserve;
        }

        //calc the meter lengths
        var nResvW = parseFloat(nReserve/100.0)*nWidth;
        var nCurrW = parseFloat(nMyCur/100.0)*nWidth;
        var nOverW = parseFloat(nCurrent/100.0)*nWidth;

        var nOver = 0.0;

        //if we have an overage calc it
        if(nReserve > 0 && (nCurrent-nReserve) > 0)
        {
            nOver = (nCurrent-nReserve);;
        }

        //adjust all the meters to the appropriate length
        $("#meterMacResv"+meterID).stop().animate({width: nResvW});
        $("#meterMacOver"+meterID).stop().animate({width: nOverW});
        $("#meterMacCrnt"+meterID).stop().animate({width: nCurrW});
        $("#meterMacLabel"+meterID).css("width",nWidth);

        var strLabel = "";

        //if we have an overage make the label
        if(nReserve > 0 && (nCurrent-nReserve) > 0)
        {
            nOver = (nCurrent-nReserve);

            strLabel = ""+parseFloat(nCurrent).toFixed(1)+"% ( "+parseFloat(nOver).toFixed(1)+"% over reserve )";
        }
        else // if not make a normal label
        {
            strLabel = ""+parseFloat(nMyCur).toFixed(1)+"%";
        }

        //write the labels out
        $("#meterMacLabel"+meterID).text(strLabel);
        $("#childrsv"+nID).text("Reserve: "+parseFloat(nReserve).toFixed(1)+"%");
    }

}//end of dkATFView

//////////////////////////////////////////////////////////////
/*
    this is the functionality for drawing the
    Airtime Fairness configuration view
*/
//////////////////////////////////////////////////////////////

/*
    function: delabel

    parameters:
    strIn - the label value pair to separate

    used in this.ssid() to remove the label from a piece of data
    in the format of "label=value" or "label:value"

    returns:
    the value without the label

*/
function delabel(strIn)
{
    var rt = strIn.split(/:|=/);

    return rt[1];
}


/*
    class: dkATFConfig

    this draws a table to be used for Airtime configuration
    changes and hase all the utilities to manipulate it.
*/
function dkATFConfig()
{
    //for scoping reasons
    var self = this;

    //the parent div to draw in
    this.strDiv = "";

    /*
        variable: nItemID

        parent UUID is just an int that isn't reused
        incremented on every add
    */
    this.nItemID    = 0;

    /*
        variable: nChildID

        child UUID is just an int that isn't reused
        incremented on every add
    */
    this.nChildID   = 0;

    /*
        variable: nCellWidth

        default cell width on the table
    */
    this.nCellWidth = 200;

    /*
        variable: nPrevValue

        the previous reserve value edited
        held for undos and such
    */

    this.nPrevValue = 0;


    /*
        variable: pOnChange

        if defined this is called whenever
        something in the table changes

        set with onChange() function
    */
    this.pOnChange = null;

    /*
        function: init

        initialize/create the ATF Config Table

        parameters:
        strDiv - parent div to draw in

        returns:
        nothing.
    */
    this.init = function(strDiv)
    {
        self.strDiv = strDiv;

        if($('#DeviceTable').length == 0)
        {
            var strATF = "";

            strATF +=   "<table id='DeviceTable' border='0' cellspacing='0' cellpadding='0' style='width: 100%;min-width: 800px;max-width: 1200px;margin: auto;'>";
            strATF +=   "</table>";
            strATF +=   "<div style='color:white; border-style: solid; border-bottom: solid 1px #ffffff;border-top: none;\
                        border-left: none;border-right: none;min-width: 800px; max-width: 1200px;margin: auto;'>";
            strATF +=   "</div>";

            $("#"+self.strDiv).append(strATF);

            strATF =   "<br>";
            strATF +=   "<br>";
            strATF +=   "<div class='cbi-page-actions' style='margin: auto;min-width: 800px;max-width: 1200px;'>";
            strATF +=   "<button id='btApply' class='cbi-button btn.disabled' type='text' value='Apply'>Save & Apply</button>";
            strATF +=   "<button id='btReset' class='cbi-button btn.disabled' type='text' value='Reset'>Reset</button>";
            strATF +=   "<button id='btDefaults' class='cbi-button btn.disabled' type='text' value='Defaults'>Defaults</button>";
            strATF +=   "</div>";

            $(strATF).insertAfter("#"+self.strDiv)
        }
    }

    /*
        function: addSSID

        add an ssid to the table

        parameters:
        strSSID     - the ssid to add
        strChan     - channel BW
        strBitRate  - bits per sec
        nReserve    - % of airtime to reserve

        returns:
        the ID string of the added element
    */
    this.addSSID = function(strSSID,
                            strChan,
                            strBitRate,
                            nCurrent,
                            nReserve)
    {
        var nMyID = self.nItemID++;
        var strItem = "";

        //create the HTML for the ssid row
        strItem += "<tr id='item"+nMyID+"' class='devicerow' ssid='"+strSSID+"'>";
        strItem += "<td class='ssid'>";
        strItem += "<table border='0' cellspacing='0' cellpadding='0' style='width: 100%;'>";
        strItem += "<tr>";
        strItem += "<td style='width: 32px;'>";
        strItem += "<img id='deviceicon' src='/images/device_router.png' style='height: 72px;position: absolute;\
                    margin-top: -36px;margin-left: -32px;z-index: 128;padding-left:10px;'>";
        strItem += "</td>";
        strItem += "<td id='ssid"+nMyID+"' style='padding-left: 89px;text-align:left;width:"+self.nCellWidth*2+"px;'>";
        strItem += "SSID: "+strSSID + " - "+strChan+" / "+strBitRate+"";
        strItem += "</td>";
        strItem += "<td id='current"+nMyID+"' style='text-align:left;width:"+self.nCellWidth+"px;cursor: pointer;'>";
        strItem += "Current: "+nCurrent+"%";
        strItem += "</td>";
        strItem += "<td style='text-align:left;width:"+self.nCellWidth/2+"px;margin-left: 0px;padding-left: 0px;'>";
        strItem += "<div id='reserve"+nMyID+"' style='text-align:left;width:"+self.nCellWidth/2+"px;cursor: pointer;\
                    height:36px;line-height:36px;padding-left:10px;'>";
        strItem += "Reserve: "+nReserve+"%";
        strItem += "</div>";
        strItem += "</td>";
        strItem += "<td style='min-width:25%;max-width:50%;color:white;'>";
        strItem += " ";
        strItem += "</td>";
        strItem += "<td style='margin:0px;padding:0px;'><button id='itemDefaults"+nMyID+"' class='cbi-button btn.disabled' type='text' value='Defaults' style='display:none;'>Defaults</button></td>";


        strItem += "<td id='openclose"+nMyID+"' style='width:48px;cursor: pointer;'>";
        strItem += "<img id='openclose"+nMyID+"' src='/images/down_arrow.png' width='16' style='position: absolute;\
                    margin-left: -18px;margin-top: -8px;cursor: pointer;'>";
        strItem += "</td>";
        strItem += "</tr>";
        strItem += "</table>";
        strItem += "</td>";
        strItem += "</tr>";

        //add empty child row where our MAC's go
        strItem += "<tr class='childrow'>";
        strItem += "<td style='height:100%;min-height:64px;padding-top: 0px;'>";
        strItem += "<div style='border-radius: 0px 0px 10px 10px; background-color:#8b8f95;\
                    height: 100%;margin-left: 25px;margin-right: 25px;color:white;'>";
        strItem += "<table id='children"+nMyID+"' border=0' cellspacing='0' cellpadding='0' style='width:100%;'>";
        strItem += "</table>";
        strItem += "</div>";
        strItem += "</td>";
        strItem += "</tr>";

        //add this to the page in the Device table ID
        //created in the init() function
        $("#DeviceTable").append(strItem);

        //hook the defaults() function
        $("#itemDefaults"+nMyID).click({nMyID: nMyID},function()
        {

            var divHtml = delabel($("#reserve"+nMyID).html()).replace("%",""); // notice "this" instead of a specific #myDiv

            self.nPrevValue = parseInt(divHtml);

            $("#reserve"+nMyID).text("Reserve: 0%");

            //if we have a listener call hime and let him know about the table changes
            if(typeof self.pOnChange != 'undefined' && self.pOnChange != null && 0 != self.nPrevValue)
            {
                var id = $("#item"+nMyID).attr("ssid");
                self.pOnChange(-1,nMyID,0,id);
            }
        });

        //set us to a defalt of 0 children
        //is this necessary on this page?
        //probably not.
        $("#item"+nMyID).attr("childID",0);

        //hook up the little open close arrow so that it works
        $("#openclose"+nMyID).click(function(event)
        {
            var id = event.target.id;
            var nID =id.replace("openclose","");

            var visible = $("#children"+nID).css("display");

            if(visible == "none")
            {
                $("#children"+nID).css("display","table");
                $("#openclose"+nID).children().attr("src", "/images/down_arrow.png");
            }
            else
            {
                $("#children"+nID).css("display","none");
                $("#openclose"+nID).children().attr("src", "/images/left_arrow.png");
            }
        });

        //the handle for editing the reserve on the row
        function editClick(event)
        {
            var divHtml = delabel($(this).html()).replace("%",""); // notice "this" instead of a specific #myDiv

            var strCtrl = "";
            strCtrl += "<td id='killme' style='text-align:left;width:"+self.nCellWidth+"px;'>";
            strCtrl += "<label for='rsrv' style='font-family:\"Segoe UI\";padding-left:0px;'>Reserve:</label>";
            strCtrl += "<input id='rsrv' type='number' min='0'max='100' step='1' style='width:72px;'/></td>";

            var editableText = $(strCtrl);


            self.nPrevValue = parseInt(divHtml);

            $(this).replaceWith(editableText);

            $("#rsrv").val(self.nPrevValue);

            $("#rsrv").focus();
            // setup the blur event for this new textarea
            $("#rsrv").blur({nMyID: event.data.nMyID},editBlur);

            $("#rsrv").keypress(function (e) {
              if (e.which == 13)
              {
                $("#rsrv").blur();
              }
            });
        };

        //the handler for clicking off the reserve on a row
        function editBlur(event)
        {
            var strItem = "";
            var val = parseInt($("#rsrv").val());

            if(val < 0.0)
            {
                val=0.0;
            }

            if(val>100.0)
            {
                val = 100.0;
            }

            if(isNaN(val) == true)
            {
                val = self.nPrevValue;
            }


            strItem += "<td id='reserve"+nMyID+"' style='text-align:left;width:"+self.nCellWidth/2+"px;cursor: pointer;'>";
            strItem += "Reserve: "+val+"%";
            strItem += "</td>";

            var viewableText = $(strItem);
            $("#killme").replaceWith(viewableText);
            // setup the click event for this new div
            $(viewableText).click({nMyID: event.data.nMyID},editClick);

            //if we have a listener call hime and let him know about the table changes
            if(typeof self.pOnChange != 'undefined' && self.pOnChange != null && val != self.nPrevValue)
            {
                var id = $("#item"+nMyID).attr("ssid");
                self.pOnChange(-1,nMyID,val,id);
            }
        };

        //hook the editClick() function to the reserve click
        $("#reserve"+nMyID).click({nMyID: nMyID},editClick);

        //highlight the SSID row on hove
        $("#item"+nMyID).hover(function()
        {
            $("#reserve"+nMyID).css("background-color","white");
            $("#reserve"+nMyID).css("border-radius","10px");
            $("#reserve"+nMyID).css("color","black");
            $("#current"+nMyID).css("color","black");
            $("#itemDefaults"+nMyID).css("display","block");
        },
        function() //unhighlight on leave
        {
            $("#reserve"+nMyID).css("background-color","");
            $("#reserve"+nMyID).css("border-radius","");
            $("#reserve"+nMyID).css("color","");
            $("#current"+nMyID).css("color","");
            $("#itemDefaults"+nMyID).css("display","none");

        });

        //return our id
        return nMyID;
    }

    /*
        function: updateSSID

        this is used to update the current and reserve
        percentages of an existing ssid item

        parameters:
        nItemId  - the id of the item to update returns by addSSID
        nCurrent - current live percentage of airtime
        nReserve - stored reserve of airtime
    */
    this.updateSSID = function( nItemID,
                                nCurrent,
                                nReserve)
    {
        $("#reserve"+nItemID).text("Reserve: "+nReserve+"%");
        $("#current"+nItemID).text("Current: "+nCurrent+"%");
    }

    /*
        function: addMAC

        add a mac to a parrent sssid in the table

        parameters:
        nItemID     - parent id to add this to - returned from addSSID()
        strMAC      - the mac to add
        nCurrent    - current live airtime
        nReserve    - % of airtime to reserve

        returns:
        the ID string of the added element
    */
    this.addMAC = function( nItemID,
                            strMAC,
                            nCurrent,
                            nReserve,
                            strName)
    {
        var strItem = "";

        //get id and increment for next add
        var nMyID       = self.nChildID++;
        var nMyParent   = nItemID;

        //create the html to append for a mac
        strItem += "<tr id='child"+nMyID+"' class='mac' style='height:72px;' mac='"+strMAC+"'>";
        strItem += "<td style='width: 32px;margin-top: 8px;'>";
        strItem += "<img id='deviceicon' src='/images/flow_default.png' style='height: 64px;position: absolute;\
                    margin-top: -32px;margin-left: -20px;z-index: 128;padding-left:10px;'>";
        strItem += "</td>";
        strItem += "<td style='text-align:left;width:"+self.nCellWidth*2+"px;padding-left:64px;'>";

        if(typeof(strName) != 'undefined')
        {
            strItem += "ID: "+strName+" / "+strMAC;
        }
        else
        {
            strItem += "ID: "+strMAC;
        }
        strItem += "</td>";
        strItem += "<td id='childcurrent"+nMyID+"'style='text-align:left;width:"+self.nCellWidth+"px;'>";
        strItem += "Current: "+nCurrent+"%";
        strItem += "</td>";
        strItem += "<td style='text-align:left;width:"+self.nCellWidth/2+"px;margin-left: 0px;padding-left: 0px;'>";
        strItem += "<div id='childrsv"+nMyID+"' style='text-align:left;width:"+self.nCellWidth/2+"px;cursor: pointer;\
                    height:36px;line-height:36px;padding-left:10px;'>";
        strItem += "Reserve: "+nReserve+"%";
        strItem += "</div>";
        strItem += "</td>";

        strItem += "<td style='min-width:25%;max-width:50%;color:white;'>";
        strItem += "<td style='margin:0px;padding:0px;'><button id='childDefaults"+nMyID+"' class='cbi-button btn.disabled' type='text' value='Defaults'  style='display:none;'>Defaults</button></td>";

        strItem += " ";
        strItem += "</td>";
        strItem += "</tr>";


        //add our mac to the parent ssid
        $("#children"+nItemID).append(strItem);

        //hook the defaults() function
        $("#childDefaults"+nMyID).click({nMyID: nMyID},function()
        {

            var divHtml = delabel($("#reserve"+nMyID).html()).replace("%",""); // notice "this" instead of a specific #myDiv

            self.nPrevValue = parseInt(divHtml);

            $("#childrsv"+nMyID).text("Reserve: 0%");

            //if we have a listener call hime and let him know about the table changes
            if(typeof self.pOnChange != 'undefined' && self.pOnChange != null && 0 != self.nPrevValue)
            {
                var id = $("#child"+nMyID).attr("mac");
                self.pOnChange(nItemID,nMyID,0,id);
            }
        });

        //the edit reserve handler
        function editClick(event)
        {
            var divHtml = delabel($(this).html()).replace("%",""); // notice "this" instead of a specific #myDiv

            var strCtrl = "";
            strCtrl += "<td id='killme' style='text-align:left;width:"+self.nCellWidth+"px;'>";
            strCtrl += "<label for='rsrv' style='font-family:\"Segoe UI\";padding-left:0px;'>Reserve:</label>";
            strCtrl += "<input id='rsrv' type='number' min='0'max='100' step='1' style='width:72px;'/></td>";

            var editableText = $(strCtrl);


            self.nPrevValue = parseInt(divHtml);

            $(this).replaceWith(editableText);

            $("#rsrv").val(self.nPrevValue);

            $("#rsrv").focus();

            // setup the blur event for this new textarea
            $("#rsrv").blur({nMyID: event.data.nMyID},editBlur);

            $("#rsrv").keypress(function (e) {
              if (e.which == 13)
              {
                $("#rsrv").blur();
              }
            });
        };

        //done editing reserve handler
        function editBlur(event)
        {
            var strItem = "";
            var val = parseInt($("#rsrv").val());

            if(val < 0.0)
            {
                val=0.0;
            }

            if(val>100.0)
            {
                val = 100.0;
            }

            if(isNaN(val) == true)
            {
                val = self.nPrevValue;
            }


            strItem += "<div id='childrsv"+nMyID+"' style='text-align:left;width:"+self.nCellWidth/2+"px;\
                        cursor: pointer;height:36px;line-height:36px;padding-left:10px;'>";
            strItem += "Reserve: "+val+"%";
            strItem += "</div>";

            var viewableText = $(strItem);
            $("#killme").replaceWith(viewableText);
            // setup the click event for this new div
            $(viewableText).click({nMyID: event.data.nMyID},editClick);

            //if we have a listener call hime and let him know about the table changes
            if(typeof self.pOnChange != 'undefined' && self.pOnChange != null && val != self.nPrevValue)
            {
                var id = $("#child"+nMyID).attr("mac");
                self.pOnChange(nItemID,nMyID,val,id);
            }
        };

        //hook up reserve edit click
        $("#childrsv"+nMyID).click({nMyID: nMyID},editClick);

        //set the row to highlight on a hover
        $("#child"+nMyID).hover(function()
        {
            $("#childrsv"+nMyID).css("background-color","white");
            $("#childrsv"+nMyID).css("border-radius","10px");
            $("#childrsv"+nMyID).css("color","black");
            $("#childcurrent"+nMyID).css("color","black");
            $("#childDefaults"+nMyID).css("display","block");
        },
        function() //unhighlight row on end of hover
        {
            $("#childrsv"+nMyID).css("background-color","");
            $("#childrsv"+nMyID).css("border-radius","");
            $("#childrsv"+nMyID).css("color","");
            $("#childcurrent"+nMyID).css("color","");
            $("#childDefaults"+nMyID).css("display","none");

        });

        return nMyID;
    }


    /*
        function: updateMAC

        this is used to update the current and reserve
        percentages of an existing mac item

        parameters:
        nItemId  - the id of the item to update returns by addMac
        nCurrent - current live percentage of airtime
        nReserve - stored reserve of airtime
    */
    this.updateMAC = function(  nItemID,
                                nCurrent,
                                nReserve)
    {
        $("#childrsv"+nItemID).text("Reserve: "+nReserve+"%");
        $("#childcurrent"+nItemID).text("Current: "+nCurrent+"%");
    }

    /*
        Function: onChange

        when a ssid changes the callback supplide here
        gets the update event

        parameters:
        pFunc - the function to call

        callback parameters:
        nParent  - nParent ID -1 if no parent
        nItem    - my item id
        nReserve - float reserve % of 1000 ms range: 0.0 - 100.0

        returns:
        nothing
    */
    this.onChange = function(pFunc)
    {
        self.pOnChange = pFunc;
    }

    /*
        Function: delSSID

        delete a ssid and its children

        parameters:
        nID - the id of the element to delete

        returns:
        nothing
    */
    this.delSSID = function(nID)
    {
        //not needed yet
    }

    /*
        Function: delMAC

        delete a mac from the parent ssid in the table

        parameters:
        nChildID - the child to delete

        returns:
        nothing
    */
    this.delMAC = function( nChildID)
    {
        //not needed yet
    }

    /*
        Function: changed

        sets internal state to something has changed from the
        initial load state. Enable Apply/Cancel button

        parameters:
        bChanged - true to enable the apply/cancel buttons

        returns:
        nothing.
    */
    this.changed = function(bChanged)
    {
        if(bChanged == true)
        {
            $("#btApply").removeClass("btn.disabled");
            $("#btReset").removeClass("btn.disabled");
            $("#btApply").addClass("cbi-button-apply");
            $("#btReset").addClass("cbi-button-reset");
        }
        else
        {
            $("#btApply").removeClass("cbi-button-apply");
            $("#btReset").removeClass("cbi-button-reset");
            $("#btApply").addClass("btn.disabled");
            $("#btReset").addClass("btn.disabled");
        }
    }
}//end of ATFConfig


//////////////////////////////////////////////////////////////
/*
    Lower level ATF Settings functions and classes
*/
//////////////////////////////////////////////////////////////

/*
    function: removeString

    parameters:
    arr         - the array to remove the values from
    varparams   - a variable list of strings to remove form the array

    remove elements matching a given values
    variabel argument after arr var arr matched for removal
*/
function removeString(arr) {
    var item;
    var args = arguments
    var len = args.length
    var pos;

    while (len > 1 && arr.length)
    {
        item = args[--len];
        while ((pos= arr.indexOf(item)) !== -1)
        {
            arr.splice(pos, 1);
        }
    }
    return arr;
}

/*
    function: removeRange

    parameters:
    arr     - array to work on
    start   - start index
    end     - ending index

    add remove range to array object
    startx to endx, not pos + length like normal function
*/
function removeRange(arr,start, end)
{
  var remains = arr.slice((end || start) + 1 || arr.length);

  arr.length = start < 0 ? arr.length + start : start;

  return arr.push.apply(arr, remains);
};

/*
    class: dkATF

    This is the functionality for get/setting ATF state
    It is used to send and recieve the commands for configuration
    of the ATF and its settings
*/
function dkATF(strMyPage)
{
    //scoperator
    var self = this;

    //the base path to the current pag
    this.strURL = "";

    if(typeof strMyPage == 'undefined')
    {
        //the ending of the url for this page
        this.strMyPage = "atfconfig";
    }
    else
    {
        //the ending of the url for this page
        this.strMyPage = strMyPage;
    }
    //initialize this with your current url
    this.init = function(strURL)
    {
        self.strURL = strURL;
    }


    /*
        function: URL

        this function allows you to call a luci page for data
        by taking the current pages url (with the session cookie)
        and modifying it with the luci page you wish to get
        data from

        parameters:
        command - the command to call

        returns:
        valid url for your data call
    */
    this.URL = function(command)
    {
        //get our current path and remove our page
        var strURL = self.strURL;

        //if this is on the tab page
        if(strURL.endsWith("/atf/"))
        {
            strURL+=command+"/";
        }
        else //if not this has a sub page
        {
            strURL = strURL.replace(self.strMyPage,command);
        }

        return strURL;
    }

    /*
        function: post

        post to a command with the given form data and return
        the result using async $.deferred callback when done

        parameters:
        strURL      - URL of the command in the router
        formdata    -  FormData() object with params for function

        returns:
        $.Deferred

        example:

        var fd = new FormData();
        fd.append( 'value', 1 );

        self.post("http://192.168.1.1/url.htm",fd)
            .done(function(data)
            {
                //process good results
            })
            .fail(function(jqXHR, textStatus, errorThrown)
            {
                //process error
        });
    */
    this.post = function(strURL, formdata)
    {
        var deferred = $.Deferred();

        //send command to get the state
        $.ajax({    processData: false,
                    contentType: false,
                    type: 'POST',
                    url: strURL,
                    dataType: "html",
                    data: formdata
                }).done(function(data, textStatus, jqXHR )
                {
                    //display the streamboost3 status
                    var strBeginToken = "<pre>";
                    var strEndToken = "</pre>";
                    var nStart    = data.indexOf(strBeginToken)+strBeginToken.length;
                    var nEnd      = data.indexOf(strEndToken,nStart);
                    var strData = data.substring(nStart,nEnd);

                    strData = strData.replace(/(\r\n|\n|\r)/gm," ");
                    var arr = strData.split(/ /);

                    removeString(arr,"");

                    deferred.resolve(arr);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    deferred.reject();
                }
        );

        return deferred;
    }

    /*
        function: addSSID

        add a ssid to Air Time Fairness and set a percent of
        airtime value

        parameters:
        wifi    - interface (ath0/etc) to send command to
        ssid    - ssid on the router add
        value   - percentage of airtime to allot for the ssid

        returns:
        nothing
    */
    this.addSSID = function(wifi,ssid,value)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );
        fd.append( 'ssid', ssid );
        fd.append( 'value', value );

        //send command to get the state
        self.post(self.URL("addssid"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: delSSID

        delet a ssid from Air Time Fairness

        parameters:
        wifi    - interface (ath0/etc) to send command to
        ssid    - ssid on the router add

        returns:
        nothing
    */
    this.delSSID = function(wifi,ssid)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );
        fd.append( 'ssid', ssid );

        //send command to get the state
        self.post(self.URL("delssid"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: addSTA

        add a device to Air Time Fairness and set a percent of
        airtime value

        parameters:
        wifi    - interface (ath0/etc) to send command to
        mac     - mac of the device to add
        value   - percentage of airtime to allot for the device

        returns:
        nothing
    */
    this.addSTA = function(wifi,mac,value)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );
        fd.append( 'mac', mac );
        fd.append( 'value', value );

        //send command to get the state
        self.post(self.URL("addsta"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: delSTA

        delet a device from Air Time Fairness
        parameters:
        wifi    - interface (ath0/etc) to send command to
        mac     - mac of the device to add

        returns:
        nothing
    */
    this.delSTA = function(wifi,mac)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );
        fd.append( 'mac', mac );

        //send command to get the state
        self.post(self.URL("delsta"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: liststa

        get the sta list of macs using the openwrt shell command
        'wlanconfig <wifi> showatftable'

        returns:
        array of devices connected to the given wifi interface

        example:

        {
        data: [
            {
                mac: "70:14:a6:39:da:f7",
                aid: "1",
                chan: "11",
                txrate: "0m",
                rxrate: "6m",
                rssi: "21",
                idle: "0",
                txseq: "94",
                rxseq: "1584",
                caps: "ESs",
                acaps: "0",
                erp: "1f",
                state: "",
                maxrate(dot11): "0",
                htcaps: "P",
                assoctime: "00:10:13",
                ies: "WME",
                mode: "IEEE80211_MODE_11NG_HT20",
                psmode: "1",
            },
            {
                mac: "d8:41:33:a7:b1:19",
                aid: "1",
                chan: "11",
                txrate: "0m",
                rxrate: "6m",
                rssi: "21",
                idle: "0",
                txseq: "94",
                rxseq: "1584",
                caps: "ESs",
                acaps: "0",
                erp: "1f",
                state: "",
                maxrate(dot11): "0",
                htcaps: "P",
                assoctime: "01:23:01",
                ies: "WME",
                mode: "IEEE80211_MODE_11NG_HT20",
                psmode: "1",
            }
        ]}
    */
    this.liststa = function(wifi,ssid)
    {

        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );

        //send command to get the state
        self.post(self.URL("liststa"),fd)
                .done(function(arr)
                {
                    //json return
                    var json = [];

                    //the row in the json return
                    var nRow = 0;

                    //remove the 1st 12 elements
                    removeRange(arr,0,20);

                    //set elements
                    for(var n = 0;n<arr.length;n++)
                    {
                        json[nRow] = [];

                        json[nRow]["mac"]               = arr[n++];
                        json[nRow]["aid"]               = arr[n++];
                        json[nRow]["chan"]              = arr[n++];
                        json[nRow]["txrate"]            = arr[n++];
                        json[nRow]["rxrate"]            = arr[n++];
                        json[nRow]["rssi"]              = arr[n++];
                        json[nRow]["minrssi"]           = arr[n++];
                        json[nRow]["maxrssi"]           = arr[n++];
                        json[nRow]["idle"]              = arr[n++];
                        json[nRow]["txseq"]             = arr[n++];
                        json[nRow]["rxseq"]             = arr[n++];
                        json[nRow]["caps"]              = arr[n++];
                        json[nRow]["acaps"]             = arr[n++];
                        json[nRow]["erp"]               = arr[n++];
                        json[nRow]["state"]             = "";
                        json[nRow]["maxrate(dot11)"]    = arr[n++];
                        json[nRow]["htcaps"]            = arr[n++];
                        json[nRow]["assoctime"]         = arr[n++];
                        json[nRow]["ies"]               = arr[n++];
                        json[nRow]["mode"]              = arr[n++];
                        json[nRow]["psmode"]            = arr[n];

                        nRow++;

                    }

                    //return our data
                    deferred.resolve(json,wifi,ssid);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };
    /*
        function: atftable

        get the atf table using the openwrt shell command
        'wlanconfig <wifi> showatftable'

        returns:
        array of ssid's, their devices, and air time %'s

        example:

        {
        data: [
            {
                atf: "50.0"
                configatf: "0.0"
                length: 0
                mac: "d8:50:e6:73:ef:b7"
                peerstatus: "1"
            },
            {
                atf: "50.0"
                configatf: "0.0"
                length: 0
                mac: "70:14:a6:39:da:f7"
                peerstatus: "1"
            }
        ]}
    */
    this.atftable = function(wifi,ssid)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );

        //send command to get the state
        self.post(self.URL("showatftable"),fd)
                .done(function(arr)
                {
                    //remove the 1st elements
                    removeRange(arr,0,11);

                    //find the end to kill
                    var nStart = -1;

                    //if we have ATF error
                    //or the ctl driver response
                    for(var n = 0;n<arr.length;n++)
                    {
                        //found it
                        if(arr[n] == "ATF" || arr[n] =="ctl")
                        {
                            //set strip point
                            nStart = n;
                            break;
                        }
                    }

                    //strip the bad ending elements
                    if(nStart != 0)
                    {
                        removeRange(arr,nStart,arr.length);
                    }

                    //create our return array
                    var json = [];

                    //the row in the json return
                    var nRow = 0;

                    var lastSSID = "";
                    //set each
                    for(var n = 0;n<arr.length;n++)
                    {
                        //is this the end of the list?
                        if(arr[n] == "Others")
                        {
                            n+=2;
                        }

                        json[nRow] = [];

                        //is this a ssid
                        if(arr[n].indexOf(":") == -1)
                        {
                            json[nRow]["interface"]     = wifi;
                            json[nRow]["ssid"]          = arr[n++];
                            json[nRow]["atf"]           = arr[n++];
                            json[nRow]["configatf"]     = parseInt(arr[n]);
                            json[nRow]["peerstatus"]    =  undefined;

                            //cache the ssid for devices
                            lastSSID                    = json[nRow]["ssid"];
                        }
                        else // this is a device
                        {
                            json[nRow]["myssid"]        = lastSSID;
                            json[nRow]["interface"]     = wifi;
                            json[nRow]["mac"]           = arr[n++];
                            json[nRow]["atf"]           = arr[n++];
                            json[nRow]["configatf"]     = parseInt(arr[n++]);
                            json[nRow]["peerstatus"]    =  arr[n++];
                        }

                        nRow++;
                    }

                    //return our data
                    deferred.resolve(json,wifi,ssid);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: airtime

        get the airte using the openwrt shell command
        'wlanconfig <wifi> showairtime'

        returns:
        array of macs and air time %'s

        example:

        {
        data: [
            {
                airtime: "50.0"
                length: 0
                mac: "d8:50:e6:73:ef:b7"
            },
            {
                airtime: "50.0"
                configatf: "0.0"
                length: 0
                mac: "70:14:a6:39:da:f7"
            }
        ]}
    */
    this.airtime = function(wifi)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );

        //send command to get the state
        self.post(self.URL("showairtime"),fd)
                .done(function(arr )
                {
                    //remove the 1st 12 elements
                    removeRange(arr,0,7);

                    //create our return array
                    var json = [];

                    //the row in the json return
                    var nRow = 0;

                    //set each
                    for(var n = 0;n<arr.length;n++)
                    {
                        json[nRow] = [];

                        json[nRow]["mac"]       = arr[n++]
                        json[nRow]["airtime"]   = arr[n++];

                        nRow++;
                    }

                    //return our data
                    deferred.resolve(json);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };


    /*
        function: addGroup

        add an ssid to a group

        parameters:
        wifi    - interface on the router to send command to
        ssid    - wifi ssid to add
        group   - group to add ssid to

        returns:
        nothing
    */
    this.addGroup = function(wifi,ssid,group)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );
        fd.append( 'ssid', ssid );
        fd.append( 'group', group );

        //send command to get the state
        self.post(self.URL("addatfgroup"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: delGroup

        delete a group

        parameters:
        wifi    - interface on the router to send command to
        group   - group to add ssid to

        returns:
        nothing
    */
    this.delGroup = function(wifi,group)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );
        fd.append( 'group', group );

        //send command to get the state
        self.post(self.URL("delatfgroup"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: groups

        get a list of the groups on the router

        parameters:
        wifi    - interface on the router to send command to

        returns:
        list of groups

        example:

        {
        data: [
            {
                airtime: "50.0"
                ssid: "OpenWrt"
                group: "Kids"
            },
            {
                airtime: "50.0"
                ssid: "OpenWrt2"
                group: "Kids"
            },
        ]}
    */
    this.groups = function(wifi)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );

        //send command to get the state
        self.post(self.URL("showatfgroup"),fd)
                .done(function(arr )
                {
                    //remove the 1st 12 elements
                    removeRange(arr,0,6);

                    //create our return array
                    var json = [];

                    //the row in the json return
                    var nRow = 0;

                    //set each
                    for(var n = 0;n<arr.length;n++)
                    {
                        json[nRow] = [];

                        json[nRow]["ssid"]     = arr[n++];
                        json[nRow]["airtime"]   = arr[n++];
                        json[nRow]["group"]      = arr[n++];

                        nRow++;
                    }

                    //return our data
                    deferred.resolve(json);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };


    /*
        function: flushtable

        flush the atf table and clear it

        parameters:
        wifi    - interface on the router to send command to

        returns:
        none.
    */
    this.flushtable = function(wifi)
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', wifi );

        //send command to get the state
        self.post(self.URL("flushatftable"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: ssid

        get the ssids hosted on this router

        parameters:
        none.

        returns:
        array of ssid and their settings

        example:

        {
        data: [{
                access-point: " 02:03:7F:48:82:B3",
                bit-rate: "216.7 Mb/s",
                encryption-key: "off",
                essid: "OpenWrt2",
                fragment-thr: "off",
                frequency: "2.462 GHz",
                interface: "ath01",
                invalid-misc: "0",
                length: 0,
                link-quality: "0/94",
                missed-beacon: "0",
                mode: "Master",
                noise-level: "-95 dBm",
                power-management: "off",
                rts-thr: "off",
                rx-invalid-crypt: "0",
                rx-invalid-frag: "0",
                rx-invalid-nwid: "22594",
                signal-level: "-95 dBm",
                tx-excessive-retries: "0",
                tx-power: "27 dBm",
                wifi-type: "IEEE 802.11ng"
            },
            {
                access-point: " 01:02:4F:57:3b:B4",
                bit-rate: "216.7 Mb/s",
                encryption-key: "off",
                essid: "OpenWrt",
                fragment-thr: "off",
                frequency: "2.462 GHz",
                interface: "ath01",
                invalid-misc: "0",
                length: 0,
                link-quality: "0/94",
                missed-beacon: "0",
                mode: "Master",
                noise-level: "-95 dBm",
                power-management: "off",
                rts-thr: "off",
                rx-invalid-crypt: "0",
                rx-invalid-frag: "0",
                rx-invalid-nwid: "22594",
                signal-level: "-95 dBm",
                tx-excessive-retries: "0",
                tx-power: "27 dBm",
                wifi-type: "IEEE 802.11ng"
            },
        ]}
    */
    this.ssids = function()
    {
        var deferred = $.Deferred();

        var fd = new FormData();

        //send command to get the state
        self.post(self.URL("iwconfig"),fd)
                .done(function(arr )
                {
                    //create our return array
                    var json = [];

                    //the row in the json return
                    var nRow = 0;

                    //walk all the data and add each AP
                    for(var n = 0;n<arr.length;n++)
                    {
                        json[nRow] = [];

                        //the 1st two elements are unamed so we have to do them manually
                        json[nRow]["interface"]         = arr[n++];
                        json[nRow]["wifi-type"]         = arr[n++] + " " + arr[n++];

                        //loop exit var
                        var bLoop = true;

                        //this is used to assemble the labels/names for our each variable
                        var Label = "";

                        //loop until the end of the data
                        while(bLoop == true)
                        {
                            ///if we already have a label
                            if(Label != "")
                            {
                                //append a - to hyphenate the label parts
                                Label += "-";
                            }

                            //add the next lable part in remove any = or : 's
                            Label += arr[n].split(/:|=/)[0];

                            //if we have an = or : then this part has the value
                            if( arr[n].indexOf(":") != -1 ||
                                arr[n].indexOf("=") != -1)
                            {
                                //pull out the label
                                var value = delabel(arr[n]);

                                //check and see if a value type is the
                                //next element and if it is
                                if( arr[n+1]=="dBm" ||
                                    arr[n+1]=="GHz" ||
                                    arr[n+1]=="MHz" ||
                                    arr[n+1]=="b/s" ||
                                    arr[n+1]=="Kb/s" ||
                                    arr[n+1]=="Mb/s" ||
                                    arr[n+1]=="Gb/s" ||
                                    (typeof arr[n+1] == 'string' &&
                                        typeof arr[n+1].split(":") != 'undefined' &&
                                        arr[n+1].split(":").length >= 6))
                                {
                                    //append the value type to our value
                                    value += " "+arr[n+1];
                                    n++;
                                }

                                value = value.replace(/\"/g,"");

                                //add the add the value to our return
                                json[nRow][Label.toLowerCase()] = value;

                                //clear the label for the next variable
                                Label = "";
                            }

                            //is this the last element?
                            if(arr[n].indexOf("beacon") != -1)
                            {
                                //if so exit the loop to start a new AP
                                bLoop = false;
                            }
                            else // if not the last element
                            {
                                //increment our data index
                                n++;
                            }
                        }

                        //we add an AP go to the next one
                        nRow++;
                    }

                    //return our data
                    deferred.resolve(json);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: enabled

        is airtime fairness enabled?

        parameters:
        none

        returns:
        true  = enabled
        false = disabled
    */
    this.enabled = function()
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', "ath0" );

        //send command to get the state
        self.post(self.URL("atfstatus"),fd)
                .done(function(arr )
                {
                    var rt = false;

                    //if we get back nothing we succeded
                    if( typeof arr != 'undefined' &&
                        arr.length != 0 &&
                        arr != "" &&
                        arr.indexOf("get_commitatf:1")!=-1)
                    {
                        rt = true;
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: enable

        set airtime fairness to enabled/disabled

        parameters:
        bEnable - true = enable / false = disable

        returns:
        nothing
    */
    this.enable = function(bEnable)
    {
        var deferred = $.Deferred();
        var fd = new FormData();
        var onoff = 0;

        //set the integer for true/false
        if(bEnable)
        {
            onoff = 1;
        }

        fd.append( 'wifi', "ath0" );
        fd.append( 'onoff', onoff);

        //send command to get the state
        self.post(self.URL("setatf"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };


    /*
        function: enabled

        is airtime strict scheduling enabled?

        parameters:
        none

        returns:
        true  = enabled
        false = disabled
    */
    this.isStrict = function()
    {
        var deferred = $.Deferred();

        var fd = new FormData();
        fd.append( 'wifi', "wifi0" );

        //send command to get the state
        self.post(self.URL("strict"),fd)
                .done(function(arr )
                {
                    var rt = false;

                    //if we get back nothing we succeded
                    if( typeof arr != 'undefined' &&
                        arr.length != 0 &&
                        arr != "" &&
                        arr.indexOf("gatfstrictsched:1")!=-1)
                    {
                        rt = true;
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: enable

        set airtime fairness to enabled/disabled

        parameters:
        bEnable - true = enable / false = disable

        returns:
        nothing
    */
    this.strict = function(bEnable)
    {
        var deferred = $.Deferred();
        var fd = new FormData();
        var onoff = 0;

        //set the integer for true/false
        if(bEnable)
        {
            onoff = 1;
        }

        fd.append( 'wifi', "wifi0" );
        fd.append( 'onoff', onoff);

        //send command to get the state
        self.post(self.URL("setstrict"),fd)
                .done(function(arr )
                {
                    var rt = "Error";

                    //if we get back nothing we succeded
                    if( typeof arr == 'undefined' ||
                        arr.length == 0 ||
                        arr == "")
                    {
                        rt = "Success";
                    }

                    //return our data
                    deferred.resolve(rt);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

    /*
        function: leases

        get an array of the leases for macs/ips and their names

        parameters:
        none.

        returns:
        data: [
        {
            "expire": 0000000000,
            "mac": "00:00:00:00:00:01",
            "ip": "192.168.1.1",
            "name": "myname",
            "client-id": "00:00:00:00:00:01"
        },
        {
            "expire": 0000000000,
            "mac": "00:00:00:00:00:01",
            "ip": "192.168.1.1",
            "name": "anothername",
            "client-id": "00:00:00:00:00:01"
        },
        ]
    */
    this.leases = function()
    {

        var deferred = $.Deferred();

        var fd = new FormData();

        //send command to get the state
        self.post(self.URL("leases"),fd)
                .done(function(arr)
                {
                    //json return
                    var json = [];

                    //the row in the json return
                    var nRow = 0;

                    //set elements
                    for(var n = 0;n<arr.length;n++)
                    {
                        json[nRow] = [];

                        json[nRow]["expire"]              = arr[n++];
                        json[nRow]["mac"]             = arr[n++];
                        json[nRow]["ip"]              = arr[n++];
                        json[nRow]["name"]            = arr[n++];
                        json[nRow]["clientid"]        = arr[n];

                        nRow++;
                    }

                    //return our data
                    deferred.resolve(json);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    //danger will robinson!
                    deferred.reject();
                }
        );

        return deferred.promise();
    };

}

//////////////////////////////////////////////////////////////
/*
    non-specific general purpose utility functions
*/
//////////////////////////////////////////////////////////////


/*
    Function: sbConfig

    get/set the current StreamBoost bandwidth state

    Parameters:
    nUp           - Up banwdith of streamboost
    nDown         - Down bandwidth of streamboost
    bAutoBW       - Auto bandwidth testing true/false
    bOptIn        - Opt In to data collection true/false
    bAutoUpdate   - Auto Update streamboost true/false

    Returns:
    deferred() success/error

    if no params success returns JSON
    {
        up_limit: int,
        down_limit: int,
        auto_bandwidth: "yes" or "no",
        auto_opdate: "yes" or "no",
        opt_in: "yes" or "no",
    }
*/
function sbConfig(nUp,nDown,bAutoBW,bOptIn,bAutoUpdate)
{
    var deferred = $.Deferred();

    //get our current path and remove our page
    var strURL = "/cgi-bin/lil-ozker/api/config"

    //if we are in read mode
    if( typeof nUp          == "undefined" &&
        typeof nDown        == "undefined" &&
        typeof bAutoBW      == "undefined" &&
        typeof bOptIn       == "undefined" &&
        typeof bAutoUpdate  == "undefined")
    {
        //send command to get the state
        $.ajax({type: "GET",
                url: strURL,
                dataType: "json"}).done(function(data, textStatus, jqXHR )
                {
                    //return state in json data
                    deferred.resolve(data);

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    deferred.reject();
                }
        );
    }
    else
    {
        //make our bool go yes/no
        var strAutoBW       = "no";
        var strOptIn        = "no";
        var strAutoUpdate   = "no";

        //if Auto BW testing is yes
        if(bAutoBW == true)
        {
            strAutoBW = "yes";
        }

        //if auto update is yes
        if(bAutoUpdate == true)
        {
            strAutoUpdate = "yes";
        }

        //if opt-in is yes
        if(bOptIn == true)
        {
            strOptIn = "yes";
        }

        //make our json string to send
        var strJSON = "";
        strJSON += '{';
        strJSON += '"up_limit": '+parseInt(nUp)+',';
        strJSON += '"down_limit": '+parseInt(nDown)+',';
        strJSON += '"auto_bandwidth": "'+strAutoBW+'",';
        strJSON += '"opt_in": "'+strOptIn+'",';
        strJSON += '"auto_update": "'+strAutoUpdate+'"';
        strJSON += '}';

        //send command to set the state
        $.ajax({type: "PUT",
                url: strURL,
                dataType: "json",
                data: strJSON}).done(function(data, textStatus, jqXHR )
                {
                    deferred.resolve();

                }).fail(function(jqXHR, textStatus, errorThrown)
                {
                    deferred.reject();
                }
        );
    }

    return deferred.promise();
}

//class to display node information
function NodeInfo(div)
{
    //hand self ref
    var self = this;

    //dive we are appending too
    this.strDiv = div;

    //setup the class
    this.init = function()
    {
        //the popup window html
        var strDLG ="<div id=\"nodeinfo-hover\" title=\"Device Information\" style = \"-webkit-user-select:none;-moz-user-select:none;\
        font-size: 12px;display: none;position: absolute;background-color: white;border-radius: 10px;top: 100px;left: 200px;width: 300px;height: 180px;\
        height: 130px;padding: 10px;z-index:100000;\">\
        <div style=\"display: inline-table;width: 100px;margin-top: 8px;margin-left: 8px;\">"+_t("Name:")+" </div><div style=\"display: inline-table;margin-top: 8px;\" id=\"hover-name\">name</div><br>\
        <div style=\"display: inline-table;width: 100px;margin-left: 8px;\">"+_t("IP:")+" </div><div style=\"display: inline-table;\"id=\"hover-ip\"><!--# echo lan_ipaddr --></div><br>\
        <div id=\"iptype\"><div style=\"display: inline-table;width: 100px;margin-left: 8px;\">"+_t("IP Type:")+" </div><div style=\"display: inline-table;\"id=\"hover-ip-type\">DHCP</div><br></div>\
        <div style=\"display: inline-table;width: 100px;margin-left: 8px;\">"+_t("MAC:")+" </div><div style=\"display: inline-table;\"id=\"hover-mac\"><!--# echo lan_mac --></div><br>\
        <div style=\"display: inline-table;width: 100px;margin-left: 8px;\">"+_t("Type:")+" </div><div style=\"display: inline-table;\"id=\"hover-type\">MS Windows PC</div><br>\
        <!--div style=\"display: inline-table;width: 100px;margin-left: 8px;\">"+_t("Connection:")+" </div><div style=\"display: inline-table;\"id=\"hover-contype\">Wired</div!--><br>\
        </div>";

        //if this hasn't already been appended
        if($("#nodeinfo-hover").length == 0)
        {
            //add the dialog
            $("#"+self.strDiv).append(strDLG);
        }
    }

    //show the node information
    this.show = function(   nX,
                            nY,
                            Name,
                            IP,
                            bStatic,
                            strMAC,
                            strType,
                            bWireless)
    {
        //update name
        $("#hover-name").text(Name);

        //update ip address
        $("#hover-ip").text(IP);

        //default ip type
        var iptype = _t("Static");

        //are we DHCP?
        if(bStatic == false)
        {
            iptype = _t("DHCP");
        }

        //update ip type
        $("#hover-ip-type").text(iptype);

        //set mac address
        $("#hover-mac").text(strMAC);

        //set device type
        $("#hover-type").text(_t(strType));

        //default connection type to wireless
        var strConType = _t("Wireless");

        //are we wired?
        if(bWireless == true)
        {
            strConType = _t("Wired");
        }

        //set connection type
        $("#hover-contype").text(strConType);

        //move the window to new x,y
        $("#nodeinfo-hover").css("top",nY);
        $("#nodeinfo-hover").css("left",nX);

        //show the window
        $("#nodeinfo-hover").css("display","block");
    }

    //hide this node info popup
    this.hide = function()
    {
        $("#nodeinfo-hover").css("display","none");
    }
}

//get a friendly name from a node
function nodeName(nd)
{
    var name = "Unknown";
    var node = {};

    if(typeof nd.Pipeline != 'undefined')
    {
        node = nd.Pipeline;
    }
    else
    {
        node = nd;
    }

    //get the name of the node
    if( typeof node.name != 'undefined' &&
        node.name != "")
    {
        name = node.name;
    }
    else if(typeof node.ipv4 != 'undefined' &&
            node.ipv4 != "")
    {
        name = node.ipv4;
    }
    else if(typeof node.mac_addr != 'undefined' &&
            node.mac_addr != "")
    {
        name = node.mac_addr;
    }

    return name;
}

// a pretty bytes to size with controlable precision for text display
// automaticlly append b, Kb, Mb, Gb, Tb
function bytesToSize(   bytes,
                        precision)
{
    var strValue = "";
    // Ben/Ryan say all numbers in Megabytes (10^6) not Mebibytes (2^20)
    var kilobyte = 1000;
    var megabyte = kilobyte * 1000;
    var gigabyte = megabyte * 1000;
    var terabyte = gigabyte * 1000;

    if ((bytes >= 0) && (bytes < kilobyte))
    {
       strValue = bytes + " "+_t('bps');

    }
    else if ((bytes >= kilobyte) && (bytes < megabyte))
    {
        strValue = (bytes / kilobyte).toFixed(precision) + " "+_t('Kbps');

    }
    else if ((bytes >= megabyte) && (bytes < gigabyte))
    {
        strValue = (bytes / megabyte).toFixed(precision) + " "+_t('Mbps');

    }
    else if ((bytes >= gigabyte) && (bytes < terabyte))
    {
        strValue = (bytes / gigabyte).toFixed(precision) + " "+_t('Gbps');

    }
    else if (bytes >= terabyte)
    {
        strValue = (bytes / terabyte).toFixed(precision) + " "+_t('Tbps');

    }
    else
    {
        strValue = bytes + " "+_t('bps');
    }

    return strValue;
}

//cache an array of images
//appends .png and _hover.png to each image
//to load dev icons
function cacheImages(array)
{
    if (!cacheImages.list)
    {
        cacheImages.list = [];
    }

    var list = cacheImages.list;

    for (var i = 0; i < array.length; i++)
    {
        //regular icon
        var img = new Image();

        //hover icon
        var imgH = new Image();

        //when the reg is loaded
        img.onload = function()
        {
            var index = list.indexOf(this);

            if (index !== -1)
            {
                // remove image from the array once it's loaded
                // for memory consumption reasons
                list.splice(index, 1);
            }
        }

        //when the hover is loaded
        imgH.onload = function()
        {
            var index = list.indexOf(this);

            if (index !== -1)
            {
                // remove image from the array once it's loaded
                // for memory consumption reasons
                list.splice(index, 1);
            }
        }

        //add our icons to our list
        list.push(img);
        list.push(imgH);

        //make our paths
        img.src = "/images/"+array[i]+".png";
        imgH.src = "/images/"+array[i]+"_hover.png";
    }
}

//get the elements offset position
function getOffset( element )
{
    //add in our offsets
    var nX = 0;
    var nY = 0;

    //walk the elements parents and on up
    while( element && !isNaN( element.offsetLeft ) && !isNaN( element.offsetTop ) )
    {
        //add in the offset
        nX += element.offsetLeft - element.scrollLeft;
        nY += element.offsetTop - element.scrollTop;

        //get the next guy
        element = element.offsetParent;
    }

    //return the x/y offset
    return { top: nY, left: nX };
}

//the previous devices from /api/nodes
var lsPrevDevices = [];

//the previous devices from /api/nodes
var lsPrevFlows = [];

//how many samples does getAverage use?
var nSampleSize = 4;

//arrays used to store samples
var lsUp = [];
var lsDn = [];
var lsFlowUp = [];
var lsFlowDn = [];

//add a new sample and get the average of
//them in the given array for the ID
function getAverage(array,
                    id,
                    sample)
{
    if(typeof array[id] == "undefined")
    {
        array[id] = [];

        for(var x = 0;x<nSampleSize;x++)
        {
            array[id].push(0);
        }
    }

    //remove the 1st element
    array[id].shift();

    //apppend or new element
    array[id].push(sample);

    var nReturn = 0;
    for(var x = 0;x<nSampleSize;x++)
    {
        nReturn += array[id][x];
    }

    nReturn = nReturn / nSampleSize;

    return nReturn;
}

//async way to get the api/nodes, parse them, add bps calcs, then hand to callback
function getNodes(callback,failcb)
{
    $.ajax( { url: "/cgi-bin/lil-ozker/api/nodes",
              method: "GET",
              dataType: "json"
    }).done(function(data) {

        //if this is our 1st pass or a new sample
        if(lsPrevDevices.length == 0 || lsPrevDevices.epoch != data.epoch)
        {
            var nMax = 250000;
            var nGlobalUp = 0;
            var nGlobalDn = 0;

            //the delete element list
            var lsDelMe = [];

            //loop over all the nodes in our data
            for(var x in data.nodes)
            {
                var node = data.nodes[x].Pipeline;
                node.upBps = 0;
                node.dnBps = 0;

                var deviceclass = "Unknown";

                //does this have a device class?
                if(typeof lsClasses[node.type] != "undefined")
                {
                    deviceclass = lsClasses[node.type].class;
                }

                node.deviceclass = deviceclass;
                //is this a new ID?
                var prevNode = null;

                //find in previous nodes
                for(var y in lsPrevDevices.nodes)
                {
                    if(lsPrevDevices.nodes[y].Pipeline.mac_addr == node.mac_addr)
                    {
                        prevNode = lsPrevDevices.nodes[y].Pipeline;
                        break;
                    }
                }

                if(prevNode != null)
                {
                    //the delta for the time sliperage
                    var timedelta = node.epoch - prevNode.epoch;

                    var nUpCur  = 0;
                    var nDnCur  = 0;
                    var nUp     = 0;
                    var nDn     = 0;

                    //if we have a delta its new sample
                    if(timedelta >0)
                    {
                        nUpCur = parseInt((node.tx_bytes - prevNode.tx_bytes) / timedelta);
                        nDnCur = parseInt((node.rx_bytes - prevNode.rx_bytes) / timedelta);
                        nUp = getAverage( lsUp, node.mac_addr, nUpCur);
                        nDn = getAverage( lsDn, node.mac_addr, nDnCur);
                    }
                    else //if not show the old bps
                    {
                        nUp  = prevNode.upBps;
                        nDn  = prevNode.dnBps;
                    }

                    //set the bps from the above delta or last sample
                    node.upBps = nUp;
                    node.dnBps = nDn;

                    //add up global bandwidths
                    nGlobalUp += nUp;
                    nGlobalDn += nDn;
                }
            }

            //set global
            data.globalUp = nGlobalUp;
            data.globalDn = nGlobalDn;
        }

        callback(data);

        lsPrevDevices = data;
    }).fail(function(){
        //we had an error
        if(typeof failcb != 'undefined')
        {
            failcb();
        }
    });
};

//async way to get the api/flows, parse them, add bps calcs, then hand to callback
function getFlows(callback,failcb)
{
    $.ajax( { url: "/cgi-bin/lil-ozker/api/flows",
              method: "GET",
              dataType: "json"
    }).done(function(data) {

        //if this is our 1st pass or a new sample
        if(lsPrevFlows.length == 0 || lsPrevFlows.epoch != data.epoch)
        {
            var nMax = 250000;
            var nGlobalUp = 0;
            var nGlobalDn = 0;

            //the delete element list
            var lsDelMe = [];

            //loop over all the nodes in our data
            for(var x in data.flows)
            {
                var flow = data.flows[x];

                flow.upBps = 0;
                flow.dnBps = 0;

                //is this a new ID?
                var prevFlow = null;

                //find in previous nodes
                for(var y in lsPrevFlows.flows)
                {
                    if(lsPrevFlows.flows[y].mac == flow.mac && lsPrevFlows.flows[y].name == flow.name)
                    {
                        prevFlow = lsPrevFlows.flows[y];
                        break;
                    }
                }

                if(prevFlow != null)
                {
                    //the delta for the time sliperage
                    var timedelta = flow.epoch - prevFlow.epoch;

                    var nUpCur  = 0;
                    var nDnCur  = 0;
                    var nUp     = 0;
                    var nDn     = 0;

                    //if we have a delta its a new sample
                    if(timedelta > 0)
                    {
                        nUpCur = parseInt((flow.up_bytes - prevFlow.up_bytes) / timedelta);
                        nDnCur = parseInt((flow.down_bytes - prevFlow.down_bytes) / timedelta);
                        nUp = getAverage( lsFlowUp, flow.mac + flow.name, nUpCur);
                        nDn = getAverage( lsFlowDn, flow.mac + flow.name, nDnCur);
                    }
                    else //if not show the old bps
                    {
                        nUp  = prevFlow.upBps;
                        nDn  = prevFlow.dnBps;
                    }

                    //set the bps from the above delta or last sample
                    flow.upBps = nUp;
                    flow.dnBps = nDn;

                    //add up global bandwidths
                    nGlobalUp += nUp;
                    nGlobalDn += nDn;
                }
            }

            //set global
            data.globalUp = nGlobalUp;
            data.globalDn = nGlobalDn;
        }

        callback(data);

        lsPrevFlows = data;
    }).fail(function()
    {
        //we had an error
        if(typeof failcb != 'undefined')
        {
            failcb();
        }
    });
};

//dumb simple clone of javascript object
function clone(object)
{
    //if not an object!
    if(object == null || typeof object != "object" )
    {
        return object;
    }

    //create new
    var copy = object.constructor();

    //loop over properties
    for (var attr in object)
    {
        //if not existing
        if(object.hasOwnProperty(attr))
        {
            //add it!
            copy[attr] = clone(object[attr]);
        }
    }

    return copy;
}

//extract value of parameter from the given url
 function urlParam(name)
{
    var results = new RegExp('[\\?&]' + name + '=([^&#]*)').exec(window.location.href);

    if(results !=undefined && results.length>=1)
    {
        return results[1];
    }
    else
    {
        return 0;
    }
}

//make a random number in the givenRange
function myRand(min, max)
{
  return Math.random() * (max - min) + min;
}

/*
    Function: sbBandwidth

    get and return the sb bandwidth settings

    Parameters:
    none.

    Returns:
    Defered callback on .done returns:

    integer nUp bps
    integer nDown bps
*/
function sbBandwidth()
{
    var deferred = $.Deferred();

    //get our current path and remove our page
    var pathname = "/cgi-bin/lil-ozker/api/bw_config";//window.location.pathname;
    /*var strFind = "darmok/";
    var nDarmok = pathname.indexOf(strFind)+strFind.length;
    pathname = pathname.substring(0,nDarmok);*/

    //make clean path to bandwidth data
    var strURL = pathname;//+"sbbandwidth";

    //send the command now that we have one
    $.ajax({type: "GET",
            url: strURL,
            dataType: "json",
            timeout: 60000}).done(function(data, textStatus, jqXHR )
            {
                //up down int return values
                var nUp = 0;
                var nDown = 0;

                if(typeof data != 'undefined')
                {

                    if(typeof data.up_limit != 'undefined')
                    {
                        //if we have a valid
                        nUp = parseInt(data.up_limit);
                    }

                    if(typeof data.down_limit != 'undefined')
                    {
                        //if we have a valid down
                        nDown = parseInt(data.down_limit);
                    }
                }

                //return success!
                deferred.resolve(nUp,nDown);

            }).fail(function(jqXHR, textStatus, errorThrown)
            {
                //we failed
                deferred.reject();
            }
    );

    return deferred.promise();
}

//////////////////////////////////////////////////////////////
/*
    This provides a unified wait spinner in a darmok style
*/
//////////////////////////////////////////////////////////////

//the spinner for applying changes and saving
var spinner = null;

// function to call for the new wait spinner
// bShow   - true = show / false = hide
// strText - The message to display while spinning
function wait(bShow, strText, noblock)
{
    //are we going to show??
    if(typeof bShow != 'undefined' && bShow == true)
    {
        //does the spinner object exist?
        if(spinner == null)
        {
            //create the html to append to our page for the spinning
            var html = "";

            //block the whole page?
            if(typeof noblock == "undefined" || (typeof noblock != "undefined" && noblock == true))
            {
                html += "<div id='shadow' style='height: 100%;position: absolute;width: 100%;top: 0;left: 0;background-color: rgba(50,50,50,0.75);z-index: 100000;'>";
            }
            else //don't block the whole page
            {
                html += "<div id='shadow' style='height: 400px;position: absolute;width: 400px;top: 50%;left: 50%;z-index: 100000;display: block;background-color: transparent;margin: -200px 0 0 -200px;'>";
            }
            html += "<div id='spinner' style='display: table;margin: 0 auto;margin-top: 20%;position: relative;font-size: 48px;color: white;width:256px;'>";
            html += "</div>";
            html += "<div  id='spintext' style='display: table;margin: 0 auto;;position: relative;font-size: 32px;color: white;margin-top:72px;'>";
            html += "Applying Changes...";
            html += "</div>";
            html += "</div>";

            //append the spinning fun
            $("body").append(html);

            //option for our spin object
            var options = {
                            lines: 13,
                            length: 28,
                            width: 14,
                            radius: 42,
                            scale: .5,
                            corners: 1,
                            color: '#000',
                            opacity: 0.25,
                            rotate: 0,
                            direction: 1,
                            speed: 1,
                            trail: 60,
                            fps: 20,
                            zIndex: 2e9,
                            className: 'spinner',
                            top: '50%',
                            left: '50%',
                            shadow: false,
                            hwaccel: false,
                            position: 'absolute'
                        };

            //create the Spinner object that makes
            //all the magic work
            spinner = new Spinner(options);
        }

        //do we want to display a message?
        if(typeof strText != undefined)
        {
            //set message to display
            $("#spintext").text(strText);
        }

        //make the spinner visible
        $("#shadow").css("display","block");

        //start the animation running
        spinner.spin($("#spinner")[0]);
    }
    else // are we supposed to hide?
    {
        //de we exist
        if(spinner != null)
        {
            //stop and hide the spinner
            spinner.stop();
            $("#shadow").css("display","none");
        }
    }
}