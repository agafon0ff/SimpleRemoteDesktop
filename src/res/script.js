document.addEventListener("DOMContentLoaded", documentIsLoaded);

var dManager = null;
var displayField = null;

function documentIsLoaded()
{
    checkDeviceType();
    initExtraKeys();
    
    displayField = new DisplayField();
    
    dManager = new DataManager();
    dManager.setDisplayField(displayField);
    dManager.updateSizes = displayField.updateSizes();
}

function initExtraKeys()
{
    var extraKeys = document.getElementsByClassName('extraKey');

    if(isMobilePhone)
    {
        for(var j=0;j<extraKeys.length;++j) {
            extraKeys[j].addEventListener('touchstart', function(event){extraKeyStateChanged(this,true);});
            extraKeys[j].addEventListener('touchend', function(event){extraKeyStateChanged(this,false);});
        }
    }
    else
    {
        for(var i=0;i<extraKeys.length;++i) {
            extraKeys[i].addEventListener('mousedown', function(event){extraKeyStateChanged(this,true);});
            extraKeys[i].addEventListener('mouseup', function(event){extraKeyStateChanged(this,false);});
        }
    }
}

function extraKeyStateChanged(key,state)
{
    var num = key.getAttribute("num");

    if(num === "1111")
    {if(state)dManager.sendToSocket(KEY_CHANGE_DISPLAY);}
    else if(num === "1112")
    {if(state)showFulScreen();}
    else if(num === "1113")
    {if(state)showKeyboard();}
    else {dManager.sendParameters(KEY_SET_KEY_STATE,num,state);}
}

function showKeyboard()
{
    
}