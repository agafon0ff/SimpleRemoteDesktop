var webSocket;
var isConnected = false;
var isMobilePhone = false

var KEY_GET_IMAGE = new Uint8Array([71,73,77,71]); //ascii: "GIMG"
var KEY_GET_NEXT_TILE = new Uint8Array([71,78,88,84]); //ascii: "GNXT"
var KEY_SET_CURSOR_POS = new Uint8Array([83,67,85,80]); //SCUP
var KEY_SET_CURSOR_DELTA = new Uint8Array([83,67,85,68]); //SCUD
var KEY_SET_MOUSE_KEY = new Uint8Array([83,77,75,83]); //SMKS
var KEY_SET_MOUSE_WHEEL = new Uint8Array([83,77,87,72]); //SMWH
var KEY_SET_KEY_STATE = new Uint8Array([83,75,83,84]); //"SKST";
var KEY_CHANGE_DISPLAY = new Uint8Array([67,72,68,80]); //"CHDP";

var KEY_IMAGE_PARAM = "73,77,71,80";//new Uint8Array([73,77,71,80]); //ascii: "IMGP";
var KEY_IMAGE_TILE = "73,77,71,84";//IMGT
var KEY_SET_LAST_TILE = "83,76,83,84";//SLST

var COMMAD_SIZE = 4;
var REQUEST_MIN_SIZE = 6;

var dataTmp = new Uint8Array;

var imageWidth = 1920;
var imageHeight = 1280;

var canvasRect = new Rect(0,0,1920,1280);

var rectWidth = 100;
var lastTileReceived = false;
var container;
var canvas;
var ctx;
var isFullScreen = false;
var keyPressedList = [];

var touchX = 0;
var touchY = 0;
var cursorPosX = 100;
var cursorPosY = 100;
var isTouchMoved = false;
var timeoutTouchCounter;
var touchStepPress = 0;
var touchStepRelease = 0;

document.addEventListener("DOMContentLoaded", documentIsLoaded);

function documentIsLoaded()
{
    checkDeviceType();

    container = document.getElementById('container');
    canvas = document.getElementById('canvas');
    ctx = canvas.getContext('2d');

    window.addEventListener("contextmenu", function(event){event.preventDefault();});
    window.addEventListener("resize", updateSizes);
    window.addEventListener("blur", leavePageEvent);

    window.addEventListener("keydown", function(event){keyStateChanged(event,true);});
    window.addEventListener("keyup", function(event){keyStateChanged(event,false);});

    console.log("isMobilePhone",isMobilePhone);

    var keys = document.getElementsByClassName('key');

    if(isMobilePhone)
    {
        container.addEventListener("touchstart", touchPress, false);
        container.addEventListener("touchmove", touchMove, false);
        container.addEventListener("touchend", touchRelease, false);

        for(var j=0;j<keys.length;++j) {
            keys[j].addEventListener('touchstart', function(event){extraKeyStateChanged(this,true);});
            keys[j].addEventListener('touchend', function(event){extraKeyStateChanged(this,false);});
        }
    }
    else
    {
        canvas.addEventListener("mousedown", function(event){mouseKeyStateChanged(event,true);});
        canvas.addEventListener("mouseup", function(event){mouseKeyStateChanged(event,false);});

        canvas.addEventListener('wheel', mouseWheelEvent);
        canvas.addEventListener('mousemove', cursorPosChanged);

        for(var i=0;i<keys.length;++i) {
            keys[i].addEventListener('mousedown', function(event){extraKeyStateChanged(this,true);});
            keys[i].addEventListener('mouseup', function(event){extraKeyStateChanged(this,false);});
        }
    }

    startSocket();
    updateSizes();
}

function touchPress(e)
{
    if(e.target.classList.contains("key"))
        return;

    e.preventDefault();

    touchX = e.touches[0].pageX;
    touchY = e.touches[0].pageY;

    ++touchStepPress;

    clearTimeout(timeoutTouchCounter);
    timeoutTouchCounter = setTimeout(touchCounter,300);
}

function touchMove(e)
{
    if(e.target.classList.contains("key"))
        return;

    e.preventDefault();

    var x = e.touches[0].pageX;
    var y = e.touches[0].pageY;

    var deltaX = touchX - x;
    var deltaY = touchY - y;

    if(!isTouchMoved &&
       (Math.abs(deltaX) < 3 ||
        Math.abs(deltaY) < 3))
        return;

    isTouchMoved = true;

    touchX = x;
    touchY = y;

    sendCursorChanged(KEY_SET_CURSOR_DELTA,deltaX,deltaY);
}

function touchRelease(e)
{
    if(e.target.classList.contains("key"))
        return;

    e.preventDefault();

    isTouchMoved = false;
    ++touchStepRelease;

    clearTimeout(timeoutTouchCounter);
    timeoutTouchCounter = setTimeout(touchCounter,200);
}

function touchCounter()
{
    clearTimeout(timeoutTouchCounter);

    if(!isTouchMoved)
    {
        if(touchStepPress == 1 && touchStepRelease == 1)//Left click
        {
            sendKeyState(KEY_SET_MOUSE_KEY,0,true);
            sendKeyState(KEY_SET_MOUSE_KEY,0,false);
        }
        else if(touchStepPress == 2 && touchStepRelease == 2)//Double click
        {
            sendKeyState(KEY_SET_MOUSE_KEY,0,true);
            sendKeyState(KEY_SET_MOUSE_KEY,0,false);
            sendKeyState(KEY_SET_MOUSE_KEY,0,true);
            sendKeyState(KEY_SET_MOUSE_KEY,0,false);
        }
        else if(touchStepPress == 1 && touchStepRelease == 0)//Right click
        {
            sendKeyState(KEY_SET_MOUSE_KEY,2,true);
            sendKeyState(KEY_SET_MOUSE_KEY,2,false);
        }
        else if(touchStepPress == 2 && touchStepRelease == 1)//Left press
        {
            sendKeyState(KEY_SET_MOUSE_KEY,0,true);
        }
        else if(touchStepPress == 0 && touchStepRelease == 1)//Left press
        {
            sendKeyState(KEY_SET_MOUSE_KEY,0,false);
        }
    }

    touchStepPress = 0;
    touchStepRelease = 0;
}

function leavePageEvent()
{
    var len = keyPressedList.length;

    for(var i=0;i<len;++i)
        sendKeyState(KEY_SET_KEY_STATE,keyPressedList[i],false);
}

function startSocket()
{
    webSocket = new WebSocket('ws://' + window.location.hostname + ':8081/');

    if(!webSocket)
        return;

    webSocket.binaryType = 'arraybuffer';

    webSocket.onopen = function()
    {
        isConnected = true;
        sendToSocket(KEY_GET_IMAGE);
    };

    webSocket.onmessage = function(event){setData(event.data);};
    webSocket.onclose = function(){isConnected = false;};
}

function setData(data)
{
    var dataArray = new Uint8Array(data);
    var activeBuf = new Uint8Array(dataArray.length + dataTmp.length);
    activeBuf.set(dataTmp, 0);
    activeBuf.set(dataArray, dataTmp.length);

    var size = activeBuf.length;

    if(size < REQUEST_MIN_SIZE)
        return;

    var dataStep = 0;

    for(var i=0;i<size;++i)
    {
        var command = activeBuf.subarray(dataStep, dataStep+COMMAD_SIZE);
        var dataSize = uint16FromArray(activeBuf.subarray(dataStep + COMMAD_SIZE, dataStep + COMMAD_SIZE + 2));

        if(size >= (dataStep + COMMAD_SIZE + 2 + dataSize))
        {
            var payload = activeBuf.subarray(dataStep + COMMAD_SIZE + 2, dataStep + COMMAD_SIZE + 2 + dataSize);
            dataStep += COMMAD_SIZE + 2 + dataSize;

            newData(command,payload);

            i = dataStep;
        }
        else
        {
            dataTmp = activeBuf.subarray(dataStep, dataStep + (size - dataStep));
            break;
        }
    }
}

function newData(cmd, data)
{
    if(cmd.length !== 4)
        return;

    var command = cmd.toString();

    if(command === KEY_IMAGE_PARAM)
    {
        imageWidth = uint16FromArray(data.subarray(0,2));
        imageHeight = uint16FromArray(data.subarray(2,4));
        rectWidth = uint16FromArray(data.subarray(4,6));

        canvas.width = imageWidth;
        canvas.height = imageHeight;

        updateSizes();

        console.log("KEY_IMAGE_PARAM:",imageWidth,imageHeight);
    }
    else if(command === KEY_IMAGE_TILE)
    {
        var posX = uint16FromArray(data.subarray(0,2));
        var posY = uint16FromArray(data.subarray(2,4));

        var rawData = data.subarray(4,data.length);
        var b64encoded = btoa(String.fromCharCode.apply(null, rawData));

        var image = new Image();
        image.posX = posX*rectWidth;
        image.posY = posY*rectWidth;

        image.onload = loadImageFinished;

        var base64Png = 'data:image/png;base64,';
        base64Png += b64encoded;
        image.src = base64Png;
    }
    else console.log("newData:",command.toString(),command,data);
}

function loadImageFinished()
{
    ctx.drawImage(this, this.posX, this.posY, rectWidth, rectWidth);
}

function uint16FromArray(buf)
{
    if(buf.length === 2)
    {
        var number;
        number = buf[0] | buf[1] << 8;
        return number;
    }
    else return 0x0000;
}

function arrayFromUint16(num)
{
    var buf = new Uint8Array(2);
    buf[0] = num;
    buf[1] = num >> 8;
    return buf;
}

function sendToSocket(data)
{
    if(isConnected)
        webSocket.send(data);
}

function updateSizes()
{
    var w = window.innerWidth;
    var h = window.innerHeight;
    var rect;

    if(w < imageWidth || h < imageHeight)
        rect = proportionalResizing(0,0,w,h,imageWidth,imageHeight);
    else rect = new Rect(w/2 - imageWidth/2, h/2 - imageHeight/2, imageWidth, imageHeight);

    canvas.style.left = rect.x + "px";
    canvas.style.top = rect.y + "px";
    canvas.style.width = rect.w + "px";
    canvas.style.height = rect.h + "px";

    canvasRect = rect;
}

function proportionalResizing(rectX, rectY, rectW, rectH, ratioW, ratioH)
{
    var sSolution = ((rectW * ratioH) / rectH);

    var sRect = new Rect(rectX,rectY,rectW,rectH);

    if(sSolution > ratioW) {
        sRect.w = (ratioW * rectH) / ratioH;
        sRect.x = rectX + (rectW / 2) - (sRect.w / 2);
    } else {
        sRect.h = (ratioH * rectW) / ratioW;
        sRect.y = rectY + (rectH / 2) - (sRect.h / 2);
    }

    return sRect;
}

function openInNewWindow()
{
    var params = 'status=no,location=no,toolbar=no,menubar=no,width=500,height=400,left=100,top=100';
    window.open('/','Simple Remote Desktop',params);
}

function showFulScreen()
{
    if(!isFullScreen)
        openFullscreen();
    else closeFullscreen();

    setTimeout(updateSizes,100);
}

function openFullscreen()
{
    isFullScreen = true;

    if (container.requestFullscreen) {
        container.requestFullscreen();
    } else if (container.mozRequestFullScreen) { /* Firefox */
        container.mozRequestFullScreen();
    } else if (container.webkitRequestFullscreen) { /* Chrome, Safari and Opera */
        container.webkitRequestFullscreen();
    } else if (container.msRequestFullscreen) { /* IE/Edge */
        container.msRequestFullscreen();
    } else if (container.webkitEnterFullScreen) {
        container.webkitEnterFullScreen();
    }
}

function closeFullscreen()
{
    isFullScreen = false;

    if (document.exitFullscreen) {
        document.exitFullscreen();
    } else if (document.mozCancelFullScreen) { /* Firefox */
        document.mozCancelFullScreen();
    } else if (document.webkitExitFullscreen) { /* Chrome, Safari and Opera */
        document.webkitExitFullscreen();
    } else if (document.msExitFullscreen) { /* IE/Edge */
        document.msExitFullscreen();
    }
}

function changeDisplayNumber()
{
    sendToSocket(KEY_CHANGE_DISPLAY);
}

function mouseWheelEvent(event)
{
    event.preventDefault();

    if(event.deltaY > 0)
        sendKeyState(KEY_SET_MOUSE_WHEEL,0,true);
    else sendKeyState(KEY_SET_MOUSE_WHEEL,0,false);
}

function mouseKeyStateChanged(event, state)
{
    event.preventDefault();
    sendKeyState(KEY_SET_MOUSE_KEY,event.button,state);
}

function extraKeyStateChanged(key,state)
{
    var num = key.getAttribute("num");

    if(num === "1111")
    {if(state)changeDisplayNumber();}
    else if(num === "1112")
    {if(state)showFulScreen();}
    else {sendKeyState(KEY_SET_KEY_STATE,num,state);}
}

function keyStateChanged(event, state)
{
    event.preventDefault();
    sendKeyState(KEY_SET_KEY_STATE,event.keyCode,state);

    if(state)
        keyPressedList.push(event.keyCode);
    else
    {
        if(keyPressedList.includes(event.keyCode))
            keyPressedList.splice(keyPressedList.indexOf(event.keyCode,1));
    }
}

function checkDeviceType()
{
    if(/Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent))
    {
        isMobilePhone = true;
    }
}

function sendKeyState(command, keyNum, state)
{
    var paramSize = arrayFromUint16(3);
    var keyCode = arrayFromUint16(keyNum);

    var buf = new Uint8Array(10);
    buf[0] = command[0];
    buf[1] = command[1];
    buf[2] = command[2];
    buf[3] = command[3];
    buf[4] = paramSize[0];
    buf[5] = paramSize[1];
    buf[6] = keyCode[0];
    buf[7] = keyCode[1];
    buf[8] = 0;

    if(state)
        buf[8] = 1;

    sendToSocket(buf);
}

function cursorPosChanged(event)
{
    var x = event.clientX;
    var y = event.clientY;

    var posX = imageWidth / canvasRect.w * (x - canvasRect.x);
    var posY = imageHeight / canvasRect.h * (y - canvasRect.y);

    sendCursorChanged(KEY_SET_CURSOR_POS,posX,posY);
}

function sendCursorChanged(key, x, y)
{
    var posSize = arrayFromUint16(4);
    var posXBuf = arrayFromUint16(x);
    var posYBuf = arrayFromUint16(y);

    var buf = new Uint8Array(10);
    buf[0] = key[0];
    buf[1] = key[1];
    buf[2] = key[2];
    buf[3] = key[3];
    buf[4] = posSize[0];
    buf[5] = posSize[1];
    buf[6] = posXBuf[0];
    buf[7] = posXBuf[1];
    buf[8] = posYBuf[0];
    buf[9] = posYBuf[1];

    sendToSocket(buf);
}

function Rect(x, y, w, h)
{
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
}
