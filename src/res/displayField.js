class DisplayField
{
    constructor()
    {
        this.dataManager = null;
        this.keyPressedList = [];
        this.rectWidth = 100;
        
        this.canvas = null;
        this.ctx = null;
        this.cursor = null;
        this.cursorField = null;
        this.cursorContainer = null;

        this.cursorPosX = 100;
        this.cursorPosY = 100;
        this.width = 1;
        this.height = 1;

        this.isScaling = false;
        this.scaleSize = 0;
        this.canvasRect = new Rect(0,0,1920,1280);
        this.deltaRect = new Rect(0,0,0,0);
        this.isMobilePhone  = /Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent);
    }
    
    initDisplayField()
    {
        this.createCanvas();
        this.crateCursorField();
        
        window.addEventListener("contextmenu", function(event){event.preventDefault();});
        window.addEventListener('resize', this.updateGeometry.bind(this));
        window.addEventListener("blur", this.leavePageEvent.bind(this));
        
        if(this.isMobilePhone)
            this.initTouchField();
        else this.initMouseField();
        
        this.updateGeometry();
    }

    // _______________ Touch  interaction _______________
    initTouchField()
    {
        this.touchX = 0;
        this.touchY = 0;
        this.touchDistance = 0;

        this.stepPress = 0;
        this.stepRelease = 0;
        this.stepMove = 0;
        this.isPressed = false;

        this.touchTimer = null;

        this.cursorField.addEventListener("touchstart", this.touchPress.bind(this));
        this.cursorField.addEventListener("touchmove", this.touchMove.bind(this));
        this.cursorField.addEventListener("touchend", this.touchRelease.bind(this));

        this.cursor.style.visibility = "visible";
    }

    touchPress(e)
    {
        e.preventDefault();

        if(e.touches.length === 1 && !this.isScaling)
        {
            this.touchX = e.touches[0].pageX;
            this.touchY = e.touches[0].pageY;

            this.stepPress += 1;
            this.touchCounter();
        }
    }

    touchMove(e)
    {
        e.preventDefault();

        if(e.touches.length === 1 && !this.isScaling)
        {
            this.stepMove += 1;
            this.touchCounter();

            if(this.stepPress === 1 && this.stepMove === 1 && this.stepRelease === 0)
            {
                this.isPressed = false;
            }
            else if(this.stepPress === 2 && this.stepMove === 1 && this.stepRelease === 1)
            {
                this.isPressed = true;
                this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,1);
            }

            var x = e.touches[0].pageX;
            var y = e.touches[0].pageY;

            this.calcCursorPos(x, y);
        }
        else if(e.touches.length === 2)
        {
            this.isScaling = true;
            
            if(this.touchTimer)
                clearTimeout(this.touchTimer);

            var x1 = e.touches[0].pageX;
            var y1 = e.touches[0].pageY;

            var x2 = e.touches[1].pageX;
            var y2 = e.touches[1].pageY;

            var distance = Math.sqrt(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));

            if(distance > this.touchDistance)
                this.scaleSize += 0.5;
            if(distance < this.touchDistance)
                this.scaleSize -= 0.5;
            if(this.scaleSize < 0)
                this.scaleSize = 0;
            
            this.touchDistance = distance;
            
            this.updatePositions();
        }
    }

    touchRelease(e)
    {
        e.preventDefault();

        if(!this.isScaling)
        {
            this.stepRelease += 1;
            this.touchCounter();
        }
        else
        {
            this.touchDistance = 0;
            this.isScaling = false;
        }

        if(this.isPressed)
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,0);
    }

    touchCounter()
    {
        if(this.touchTimer)
            clearTimeout(this.touchTimer);

        this.touchTimer = setTimeout(this.touchWaiter.bind(this),200);
    }

    touchWaiter()
    {
        if(this.touchTimer)
        {
            clearTimeout(this.touchTimer);
            this.touchTimer = null;
        }

        if(this.stepPress === 1 && this.stepMove === 0 && this.stepRelease === 0)
        {
            this.touchTimer = setTimeout(this.touchWaiter.bind(this),400);
            this.stepPress = 0;
            return;
        }
        else if(this.stepPress === 0 && this.stepMove === 0 && this.stepRelease === 0)//right click
        {
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,2,1);
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,2,0);
        }
        else if(this.stepPress === 1 && this.stepMove === 0 && this.stepRelease === 1)//left click
        {
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,1);
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,0);
        }
        else if(this.stepPress === 2 && this.stepMove > 3 && this.stepRelease === 2)//left click
        {
            if(this.stepMove < 40)
            {
                this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,1);
                this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,0);
            }
        }
        else if(this.stepPress === 2 && this.stepMove === 0 && this.stepRelease === 1)//left press
        {
            this.touchTimer = setTimeout(this.touchWaiter.bind(this),200);
            return;
        }
        else if(this.stepPress === 2 && this.stepMove === 0 && this.stepRelease === 2)//double left click
        {
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,1);
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,0);
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,1);
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,0);
        }
        else if(this.stepPress === 3 && this.stepMove > 3 && this.stepRelease === 3)//double left click
        {
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,1);
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,0);
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,1);
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,0);
        }

        if(this.isPressed)
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,0,0);

        this.stepPress = 0;
        this.stepRelease = 0;
        this.stepMove = 0;
        this.isPressed = false;
    }

    calcCursorPos(x, y)
    {
        var deltaX = this.touchX - x;
        var deltaY = this.touchY - y;

        this.touchX = x;
        this.touchY = y;
        
        this.cursorPosX = this.cursorPosX - deltaX;
        this.cursorPosY = this.cursorPosY - deltaY;
        
        if(this.cursorPosX < 0) this.cursorPosX = 0;
        if(this.cursorPosY < 0) this.cursorPosY = 0;
        if(this.cursorPosX > this.canvas.width) this.cursorPosX = this.canvas.width;
        if(this.cursorPosY > this.canvas.height) this.cursorPosY = this.canvas.height;

        this.dataManager.sendParameters(KEY_SET_CURSOR_POS,this.cursorPosX,this.cursorPosY);
        this.updatePositions();
    }

    // _______________ Desktop interaction _______________
    initMouseField()
    {
        this.cursorContainer.addEventListener('mousedown', this.mouseKeyStateChanged.bind(this));
        this.cursorContainer.addEventListener('mouseup', this.mouseKeyStateChanged.bind(this));

        this.cursorContainer.addEventListener('wheel', this.mouseWheelEvent.bind(this));
        this.cursorContainer.addEventListener('mousemove', this.cursorPosChanged.bind(this));
        
        window.addEventListener("keydown", this.keyStateChanged.bind(this));
        window.addEventListener("keyup", this.keyStateChanged.bind(this));
    }

    mouseKeyStateChanged(event)
    {
        event.preventDefault();

        if(event.type === 'mouseup')
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,event.button,0);

        else if(event.type === 'mousedown')
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,event.button,1);
    }

    mouseWheelEvent(event)
    {
        event.preventDefault();

        if(event.deltaY > 0)
            this.dataManager.sendParameters(KEY_SET_MOUSE_WHEEL,0,1);
        else this.dataManager.sendParameters(KEY_SET_MOUSE_WHEEL,0,0);
    }

    cursorPosChanged(event)
    {
        var x = event.clientX;
        var y = event.clientY;

        this.cursorPosX = this.canvas.width / this.canvasRect.w * (x - this.canvasRect.x);
        this.cursorPosY = this.canvas.height / this.canvasRect.h * (y - this.canvasRect.y);

        this.dataManager.sendParameters(KEY_SET_CURSOR_POS,this.cursorPosX,this.cursorPosY);
    }
    
    keyStateChanged(event)
    {
        event.preventDefault();
        var state = 0;
        
        if(event.type === 'keydown')
            state = 1;
        
        this.dataManager.sendParameters(KEY_SET_KEY_STATE,event.keyCode,state);
        
        if(state === 1)
            this.keyPressedList.push(event.keyCode);
        else
        {
            if(this.keyPressedList.includes(event.keyCode))
                this.keyPressedList.splice(this.keyPressedList.indexOf(event.keyCode,1));
        }
    }
    // _____________________________________________________________

    updateGeometry()
    {
        this.width = window.innerWidth;
        this.height = window.innerHeight;
        var fieldHeight = this.height;
        var rect;

        if(this.keyboard.style.visibility === "visible")
        {
            if(this.width < this.height)
                rect = new Rect(0,this.height/3*2,this.width,this.height/3);
            else
            {
                rect = this.proportionalResizing(0, this.height/2, this.width, this.height/2, 100, 30);
                rect.y = this.height - rect.h;
            }
            
            keyboard.style.left = rect.x + "px";
            keyboard.style.top = rect.y + "px";
            keyboard.style.width = rect.w + "px";
            keyboard.style.height = rect.h + "px";
            fieldHeight -= rect.h;
        }

        if(this.width < this.canvas.width || this.height < this.canvas.height)
        {
            rect = this.proportionalResizing(0, 0, this.width, fieldHeight, this.canvas.width, this.canvas.height);
        }
        else
        {
            rect = new Rect(this.width/2 - this.canvas.width/2,
                            fieldHeight/2 - this.canvas.height/2,
                            this.canvas.width,
                            this.canvas.height);
        }

        this.deltaRect.w = (this.canvasRect.w) * (this.scaleSize / (this.canvasRect.w/100));
        this.deltaRect.h = (this.canvasRect.h) * (this.scaleSize / (this.canvasRect.w/100));
        
        this.canvasRect.x = rect.x - this.deltaRect.x;
        this.canvasRect.y = rect.y - this.deltaRect.y;
        this.canvasRect.w = rect.w + this.deltaRect.w;
        this.canvasRect.h = rect.h + this.deltaRect.h;

        var rectList = [this.canvas, this.cursorContainer];

        for(var i=0;i<rectList.length;++i)
        {
            rectList[i].style.left = this.canvasRect.x + 'px';
            rectList[i].style.top = this.canvasRect.y + 'px';
            rectList[i].style.width = this.canvasRect.w + 'px';
            rectList[i].style.height = this.canvasRect.h + 'px';
        }
    }
    
    updatePositions()
    {
        var posX = this.cursorPosX * this.canvasRect.w / this.canvas.width;
        var posY = this.cursorPosY * this.canvasRect.h / this.canvas.height;

        this.cursor.style.left = posX + "px";
        this.cursor.style.top = posY + "px";

        if(this.scaleSize > 0)
        {
            var percentX = 1.0 / this.canvas.width * this.cursorPosX;
            var percentY = 1.0 / this.canvas.height * this.cursorPosY;

            var delta = this.scaleSize * 10;
            var deltaX = (delta * percentX) - (delta / 2);
            var deltaY = (delta * percentY) - (delta / 2);

            this.deltaRect.x = (this.deltaRect.w * percentX) + deltaX;
            this.deltaRect.y = (this.deltaRect.h * percentY) + deltaY;
        }
        else
        {
            this.deltaRect.x = 0;
            this.deltaRect.y = 0;
        }
        
        this.updateGeometry();
    }
    
    leavePageEvent()
    {
        var len = this.keyPressedList.length;

        for(var i=0;i<len;++i)
            this.dataManager.sendParameters(KEY_SET_KEY_STATE,this.keyPressedList[i],false);
    }

    setDataManager(dManager)
    {
        if(dManager)
            this.dataManager = dManager;
    }
    
    setImageParameters(w, h, r)
    {
        this.rectWidth = r;
        
        if(this.canvas)
        {
            this.canvas.width = w;
            this.canvas.height = h;
            this.updateGeometry();
        }
    }
    
    setImageData(posX, posY, data, tileNum)
    {
        if(!this.ctx)
            return;

        var image = new Image();
        image.posX = posX * this.rectWidth;
        image.posY = posY * this.rectWidth;
        image.ctx = this.ctx;
        image.dataManager = this.dataManager;
        image.width = this.rectWidth;
        image.height = this.rectWidth;
        image.tileNum = tileNum;

        let b64encoded = 'data:image/png;base64,' + btoa(String.fromCharCode.apply(null, data));

        image.onload = function()
        {
            this.ctx.drawImage(this, this.posX, this.posY, this.width, this.height);
        }

        image.src = b64encoded;
        this.dataManager.sendParameters(KEY_TILE_RECEIVED, tileNum, 0);
    }
    
    createCanvas()
    {
        this.canvas = document.createElement('canvas');
        this.canvas.id = 'canvas';
        this.canvas.width = 1920;
        this.canvas.height = 1280;
        this.canvas.style.cssText = 
            'position: absolute; \
            width: 100%; \
            height: 100%; \
            z-index: 0;'; 
        
        this.ctx = this.canvas.getContext('2d');
        
        document.body.append(this.canvas);
    }
    
    crateCursorField()
    {
        this.cursorField = document.createElement('div');
        this.cursorField.id = 'cursorField';
        this.cursorField.style.cssText = 
            'position:absolute; margin:0; \
            padding:0; \
            left:0; \
            top:0; \
            width:100%; \
            height:100%; \
            z-index:1;';
        
        this.cursorContainer = document.createElement('div');
        this.cursorContainer.id = 'cursorContainer';
        this.cursorContainer.style.cssText = "position:absolute; z-index:1;";
        this.cursorField.append(this.cursorContainer);
        
        this.cursor = document.createElement('div');
        this.cursor.id = 'cursor';
        this.cursor.style.cssText = 
            'position:absolute; \
            margin:0; \
            padding:0; \
            width:10px; \
            height:10px; \
            z-index:2; \
            visibility:hidden;';
        
        this.cursorContainer.append(this.cursor);
        
        var svgCursor = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        svgCursor.id = 'svgCursor';
        svgCursor.setAttribute("width", "24");
        svgCursor.setAttribute("height", "24");
        svgCursor.setAttribute ("viewBox", "0 0 24 24");
        svgCursor.innerHTML = '<path d="M0,0 L24,9 L13,13 L9,24 L0,0" stroke="black" stroke-width="1" fill="white"/>';
        svgCursor.style.cssText = 
            'display: block; \
            position: absolute; \
            margin: 0; \
            padding: 0; \
            left: 0; \
            top: 0; \
            width: 100%; \
            height: 100%;';
        
        this.cursor.append(svgCursor);
        
        document.body.append(this.cursorField);
        
        this.keyboard = document.createElement('div');
        this.keyboard.id = 'keyboard';
        this.keyboard.isLoaded = false;
        this.keyboard.style.cssText = 
            'position: absolute;\
            background: none;\
            z-index: 3;\
            visibility: hidden;';
        
        document.body.append(this.keyboard);
    }
    
    showKeyboard()
    {
        if(!this.keyboard.isLoaded)
            this.dataManager.sendToXmlHttpRequest('GET', 'keyboard.html', '');
        
        if(this.keyboard.style.visibility === "visible")
            this.keyboard.style.visibility = "hidden";
        else this.keyboard.style.visibility = "visible";
        
        this.updateGeometry();
    }
    
    setKeyboardHtml(text)
    {
        this.keyboard.isLoaded = true;
        this.keyboard.innerHTML = text;
        
        var keys = this.keyboard.getElementsByClassName('key');

        if(this.isMobilePhone)
        {
            for(var j=0;j<keys.length;++j) {
                keys[j].addEventListener('touchstart', this.keyboardKeyStateChanged.bind(this,keys[j],1));
                keys[j].addEventListener('touchend', this.keyboardKeyStateChanged.bind(this,keys[j],0));
            }
        }
        else
        {
            for(var i=0;i<keys.length;++i) {
                keys[i].addEventListener('mousedown', this.keyboardKeyStateChanged.bind(this,keys[i],1));
                keys[i].addEventListener('mouseup', this.keyboardKeyStateChanged.bind(this,keys[i],0));
            }
        }
    }
    
    keyboardKeyStateChanged(key,state,event)
    {
        var num = key.getAttribute("num");
        this.dataManager.sendParameters(KEY_SET_KEY_STATE,num,state);
    }
    
    proportionalResizing(rectX, rectY, rectW, rectH, ratioW, ratioH)
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
}

function Rect(x, y, w, h)
{
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
}
