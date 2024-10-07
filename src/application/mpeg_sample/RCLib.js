"use strict";
import functions_js from "/functions.js";
var RCLib = {};
try
{
    console.log("importing RCLib");
    var functions = null;
    var packet_len = 0;
    var packet_id = null;
    var image_len = 0;
    var image_id = null;
    function functions_ready()
    {
	console.log("functions ready");
	functions.fill_it = functions.cwrap('fillArray', null, ['number', 'number']);
	functions.decode_packet = functions.cwrap('decode_packet', 'number', ['number', 'number']);
	functions.init_heaps =  functions.cwrap('decode_packet', null, ['number', 'number', 'number', 'number']);
	functions.init_heaps(0, 4096);
	packet_len = 4096;
	console.log("functions bound");
	if(RCLib.FunctionsReady)
	{
	    try
	    {
		RCLib.FunctionsReady();

	    }
	    catch(e)
	    {
		console.log("FunctionsReady needs to be a function.");
	    }
	}
    };
    
    console.log("compiling functions.cpp");
    console.log("crossOriginIsolated: "+crossOriginIsolated.toString());
    var t0 = performance.now();
    functions_js().then(result => {
	functions = result;
	var t1 = performance.now();
	console.log("compiled! it took: "+(t1-t0)+"ms");
	functions_ready();
    });

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
	    console.log("WS not open, failed to send: " + message);
	}
    }
    function WSMessage(e)
    {
	var out = null;
	if (WS.binaryType == "blob")
	{
	    // #todo: this doesn't appear to ever work. WS.binarytype is always blob
	    console.log("blob size: "+e.data.size);
	    out = e.data.text();
	}
	else if(WS.binaryType == "arraybuffer")
	{
	    out = new Uint8Array(e.data);
	    // console.log("array size: "+out.length);
	}
	else
	{
	    console.log("text: "+e.data);
	    out = e.data;
	}
	RCLib.ResponseCB(out);
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
	console.log("WSConnect");
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
		WS.binaryType = "arraybuffer";
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
    RCLib.ResponseCB = function(message) { console.log("ws response: "+message); };
    RCLib.ErrorCB = function(message) { console.log("ws error: "+message); };
    RCLib.CloseCB = function(message) { console.log("ws close: "+message); };
    RCLib.OpenCB = function(message) { console.log("ws opened: "+message); };
    RCLib.GetStream = function(canvas_image_len)
    {
	console.log("begin promise");
	// allocating r-8bits g-8bits b-8bits a-8bits image
	return new Promise(resolve =>
	    {
		const in_len = canvas_image_len;
		var GetStreamInternal = function() {
		    if (RCLib.FunctionsReady)
		    {
			console.log("allocating: "+in_len+" bytes");
			image_id = functions._malloc(in_len);
			image_len = in_len;
			functions.fill_it(image_id, in_len);
			console.log("returning image_buffer");
			resolve(image_id);
		    }
		    else
		    {
			console.log("waiting for functions ready");
			setTimeout(GetStreamInternal, 1000);
		    }
		};
		GetStreamInternal();
	    });
    };

    function _arrayToHeap(typedArray){
	var numBytes = typedArray.length * typedArray.BYTES_PER_ELEMENT;
	var ptr = functions._malloc(numBytes);
	var ptr_i32 = ptr >> 2; // divide byte offset by 4 to account for 4-byte int pointer
	var heapBytes = functions.HEAP32.subarray(ptr_i32, ptr_i32 + typedArray.length);
	heapBytes.set(typedArray);
	return heapBytes;
    }

    RCLib.DecodePacket = function(packet, image_buffer) {
	// Allocate some "max packet size" amount of space to copy the packet into. (e.g.)
	if(!functions)
	{
	    console.log("functions not ready");
	    return false;
	}
	if(image_buffer.length > image_len || packet.length > packet_len)
	{
	    if(image_buffer.length > image_len)
	    {
		console.log("allocating "+image_buffer.length+" for image buffer");
		image_len = image_buffer.length;
	
	    }
	    if(packet.length > packet_len)
	    {
		console.log("allocating "+packet.length+" for packet buffer");
		packet_len = packet.length;
	    }
	    functions.init_heaps(image_len, packet_len);
	}
	
	var packet_heap = functions.getPacketBuffer();
	packet_heap.set(packet);
	if(functions.decode_packet(packet.length))
	{
	    console.log("decoded packet! filling "+image_buffer.length);
	    var image_heap = functions.getImageBuffer();
	    for (let i = 0; i < image_buffer.length; i += 4)
	    {
		var value = image_heap[i];
		var val = value >>> 0;
		var r = 0xff000000;
		var g = 0x00ff0000;
		var b = 0x0000ff00;
		var a = 0x000000ff;
		r = (val & r) >> 24; g = (val & g) >> 16; b = (val & b) >> 8; a = (val & a);
		if (r < 0) r = 255;
		// Modify pixel data
		image_buffer[i + 0] = r;  // R value
		image_buffer[i + 1] = g;  // G value
		image_buffer[i + 2] = b;  // B value
		image_buffer[i + 3] = 255;  // A value hacked to be full
	    }
	    return true;
	}
	else
	{
	    console.log("error: packet decode failed!");
	    // var nonzero = 0;
	    // for(let i = 0; i < packet.length; i++)
	    // {
	    // 	if(packet[i])
	    // 	{
	    // 	    nonzero++;
	    // 	}
	    // }
	    // console.log("nonzero count: "+nonzero);
	}
	return false;
    }

    RCLib.FunctionsReady = null;
    RCLib.GetFunctions = function()
    {
	return functions;
    };
    console.log("RCLib Imported");
}
catch(e)
{
    console.log(e.name+": "+e.message);
}
export {RCLib};
