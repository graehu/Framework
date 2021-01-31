"use strict";

var WS = null;
var WSConnected = 0;
var WSOpenTime;
var WSSeconds = 0;
var WSFail = 0;
var WSIsOpen = 0;
var WSSend = 0;
var WSReceive = 0;
var WSSendBytes = 0;
var WSReceiveBytes = 0;
var WSHost = location.hostname ? location.hostname : "localhost";
var WSPort = location.port ? location.port : 8000;
//final export object. 
var RCLib = {};

function WSOpen(e)
{
    WSSend = 0;
    WSReceive = 0;
    WSSendBytes = 0;
    WSReceiveBytes = 0;
    WSIsOpen = 1;
    RCLib.OpenCB(e);
}

function WSSendMessage(message)
{
    if(WSIsOpen)
    {
	var str = '' + message;
	WSSend++;
	WSSendBytes += str.length;
	WS.send(str);
    }
    else
    {
	console.log("WS not open, failed to send: " + message)
    }
}

function WSMessage(e)
{
    if (WS.binaryType == "blob")
    {
	console.log("blob");
	console.log(e.data);
    }
    else
    {
	console.log("text");
	console.log(e.data);
    }
    RCLib.ResponseCB(e.data);
}

function WSError(e)
{
    console.log("WSError: " + e);
    RCLib.ErrorCB(e);
}

function WSClose(e)
{
    console.log("WSClosed code: " + e.code); // + " "+ e.reason);
    console.log("WSClosed reason: " + e.reason);
    WSIsOpen = 0;
    RCLib.CloseCB(e);
}
function WSConnect()
{
    if(WS && (WS.readyState == 1 || WS.readyState == 0))
    {
	WSConnected = WS.readyState == 1;
	WSFail = 0;
	WSSeconds = 0;
    }
    else
    {
	WSConnected = 0;
	WSSeconds = (new Date() - WSOpenTime);

	if(!WS || WSSeconds > 2000)
	{
	    if(WS)
	    {
		WS.close();
		WS = null;
	    }
	    WSOpenTime = new Date();
	    var WSPath = "ws://" + WSHost + ":" + WSPort + "/ws";
	    WS = new WebSocket(WSPath);
	    WS.onopen = WSOpen;
	    WS.onmessage = WSMessage;
	    WS.onerror = WSError;
	    WS.onclose = WSClose;
	    WSFail = 0;
	}
	else
	{
	    WSFail++;
	}
    }
}


//find a way to make these readonly
RCLib.Connect = function() { WSConnect(); };
RCLib.Send = function(message) { WSSendMessage(message); };
//find a way to make this writable.
RCLib.ResponseCB = function(message) { console.loog("ws response: "+message); };
RCLib.ErrorCB = function(message) { console.log("ws error: "+message); };
RCLib.CloseCB = function(message) { console.log("ws close: "+message); };
RCLib.OpenCB = function(message) { console.log("ws opened: "+message); };

export {RCLib};
 
