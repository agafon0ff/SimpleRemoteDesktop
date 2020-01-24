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