var isMobilePhone = false;
var isFullScreen = false;

function checkDeviceType()
{
    if(/Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent))
    {
        isMobilePhone = true;
    }
}

function openInNewWindow()
{
    var params = 'status=no,location=no,toolbar=no,menubar=no,width=500,height=400,left=100,top=100';
    window.open('/','Simple Remote Desktop',params);
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

function Rect(x, y, w, h)
{
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
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

    if (document.body.requestFullscreen) {
        document.body.requestFullscreen();
    } else if (document.body.mozRequestFullScreen) { /* Firefox */
        document.body.mozRequestFullScreen();
    } else if (document.body.webkitRequestFullscreen) { /* Chrome, Safari and Opera */
        document.body.webkitRequestFullscreen();
    } else if (document.body.msRequestFullscreen) { /* IE/Edge */
        document.body.msRequestFullscreen();
    } else if (document.body.webkitEnterFullScreen) {
        document.body.webkitEnterFullScreen();
    }
}

function closeFullscreen()
{
    isFullScreen = false;

    if (document.body.exitFullscreen) {
        document.body.exitFullscreen();
    } else if (document.body.mozCancelFullScreen) { /* Firefox */
        document.body.mozCancelFullScreen();
    } else if (document.body.webkitExitFullscreen) { /* Chrome, Safari and Opera */
        document.body.webkitExitFullscreen();
    } else if (document.body.msExitFullscreen) { /* IE/Edge */
        document.body.msExitFullscreen();
    }
}