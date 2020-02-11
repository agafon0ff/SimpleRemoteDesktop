class DisplayField
{
    constructor()
    {
        this.id = '123';
        this.dataManager = null;
        this.keyPressedList = [];

        this.canvas = document.getElementById('canvas');
        this.cursor = document.getElementById('cursor');
        this.cursorField = document.getElementById('cursorField');
        this.cursorContainer = document.getElementById('cursorContainer');
        this.cursorPosX = 100;
        this.cursorPosY = 100;
        this.width = 1;
        this.height = 1;

        this.isScaling = false;
        this.scaleSize = 0;
        this.canvasRect = new Rect(0,0,1920,1280);
        this.deltaRect = new Rect(0,0,0,0);

        window.addEventListener("contextmenu", function(event){event.preventDefault();});
        window.addEventListener('resize', this.updateGeometry.bind(this));
        window.addEventListener("blur", this.leavePageEvent.bind(this));

        this.updateGeometry();

        if(isMobilePhone)
            this.initTouchField();
        else this.initMouseField();
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
                this.scaleSize += 1;
            if(distance < this.touchDistance)
                this.scaleSize -= 1;
            if(this.scaleSize < 0)
                this.scaleSize = 0;
            
            this.touchDistance = distance;
            
            this.updatePositions();
        }
    }

    touchRelease(e)
    {
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
        else if(this.stepPress === 2 && this.stepMove === 0 && this.stepRelease === 1)
        {
            this.touchTimer = setTimeout(this.touchWaiter.bind(this),200);
            return;
        }
        else if(this.stepPress === 2 && this.stepMove === 0 && this.stepRelease === 2)
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

    resizeField(distance)
    {
        if(distance > this.touchDistance)
            scaleSize = 1;
        if(distance < this.touchDistance)
            scaleSize = -1;

        var percentX = 1.0 / this.canvas.width * this.cursorPosX;
        var percentY = 1.0 / this.canvas.height * this.cursorPosY;

        var deltaW = (this.canvasRect.w / 50) * scaleSize;
        var deltaH = (this.canvasRect.h / 50) * scaleSize;

        if(this.canvasRect.w > this.width || this.canvasRect.h > this.height)
        {
            this.deltaRect.x = this.deltaRect.x - deltaW * percentX;
            this.deltaRect.y = this.deltaRect.y - deltaH * percentY;
            this.deltaRect.w = this.deltaRect.w + deltaW;
            this.deltaRect.h = this.deltaRect.h + deltaH;
        }
        else
        {
            this.deltaRect.x = 0;
            this.deltaRect.y = 0;
            this.deltaRect.w = 0;
            this.deltaRect.h = 0;
        }

        this.touchDistance = distance;
        this.updateGeometry();
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

        if(event.type == 'mouseup')
            this.dataManager.sendParameters(KEY_SET_MOUSE_KEY,event.button,0);

        else if(event.type == 'mousedown')
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
        
        if(event.type == 'keydown')
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

    getCanvas()
    {
        return this.canvas;
    }

    updateGeometry()
    {
        this.width = window.innerWidth;
        this.height = window.innerHeight;

        var rect;

        if(this.width < this.canvas.width || this.height < this.canvas.height)
        {
            rect = proportionalResizing(0, 0, this.width, this.height, this.canvas.width, this.canvas.height);
        }
        else
        {
            rect = new Rect(this.width/2 - this.canvas.width/2,
                            this.height/2 - this.canvas.height/2,
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
}

