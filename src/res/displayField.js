class DisplayField
{
    constructor()
    {
        this.id = '123';
        this.dataManager = null;
        
        this.canvas = document.getElementById('canvas');
        this.cursor = document.getElementById('cursor');
        this.cursorField = document.getElementById('cursorField');
        this.cursorContainer = document.getElementById('cursorContainer');
        
        this.canvasRect = new Rect(0,0,1920,1280);
        this.transformRect = new Rect(0,0,1920,1280);
        
        window.addEventListener('resize', this.updateTransformRect.bind(this));
        
        this.updateTransformRect();
        
        if(isMobilePhone)
            this.initTouchField();
        else this.initMouseField();
    }

    // _______________ Touch  interaction _______________
    initTouchField()
    {
        this.touchX = 0;
        this.touchY = 0;
        this.cursorPosX = 100;
        this.cursorPosY = 100;
        this.realCursorPosX = 100;
        this.realCursorPosY = 100;
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
        if(e.touches.length === 1)
        {
            this.touchX = e.touches[0].pageX;
            this.touchY = e.touches[0].pageY;
            
            this.stepPress += 1;
            this.touchCounter();
        }
    }
    
    touchMove(e)
    {
        if(e.touches.length === 1)
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
            if(this.touchTimer)
                clearTimeout(this.touchTimer);
            
            var x1 = e.touches[0].pageX;
            var y1 = e.touches[0].pageY;

            var x2 = e.touches[1].pageX;
            var y2 = e.touches[1].pageY;

            var distance = Math.sqrt(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));

            this.resizeField(distance);
        }
    }
    
    touchRelease(e)
    {
        this.stepRelease += 1;
        this.touchCounter();
        
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

        if(this.cursorPosX < 0)this.cursorPosX = 0;
        
        else if(this.cursorPosX > this.canvasRect.w)
            this.cursorPosX = this.canvasRect.w;

        if(this.cursorPosY < 0)this.cursorPosY = 0;
        
        else if(this.cursorPosY > this.canvasRect.h)
            this.cursorPosY = this.canvasRect.h;

        this.cursor.style.left = this.cursorPosX + "px";
        this.cursor.style.top = this.cursorPosY + "px";

        this.realCursorPosX = this.canvas.width / this.canvasRect.w * this.cursorPosX;
        this.realCursorPosY = this.canvas.height / this.canvasRect.h * this.cursorPosY;

        this.dataManager.sendParameters(KEY_SET_CURSOR_POS,this.realCursorPosX,this.realCursorPosY);
    }
    
    resizeField(distance)
    {
        var scaleSize = 0;

        if(distance > this.touchDistance)
            scaleSize = 10;
        if(distance < this.touchDistance)
            scaleSize = -10;

        var percentX = 1 / this.canvas.width * this.realCursorPosX;
        var percentY = 1 / this.canvas.height * this.realCursorPosY;

        this.transformRect.x = this.transformRect.x - ((scaleSize) * percentX);
        this.transformRect.y = this.transformRect.y - ((scaleSize) * percentY);

        if(this.transformRect.x > 0)
            this.transformRect.x = 0;

        if(this.transformRect.y > 0)
            this.transformRect.y = 0;
        
        this.transformRect.w = this.transformRect.w + scaleSize;
        this.transformRect.h = this.transformRect.h + scaleSize;

        if(this.transformRect.w > this.canvas.width)
            this.transformRect.w = this.canvas.width;

        if(this.transformRect.h > this.canvas.width)
            this.transformRect.h = this.canvas.height;

        this.touchDistance = distance;

        this.cursorPosX = this.realCursorPosX * this.canvasRect.w / this.canvas.width;
        this.cursorPosY = this.realCursorPosY * this.canvasRect.h / this.canvas.height;
        
        this.cursor.style.left = this.cursorPosX + "px";
        this.cursor.style.top = this.cursorPosY + "px";

        this.updateSizes();
    }
    
    
    // _______________ Desktop interaction _______________
    initMouseField()
    {
        this.cursorContainer.addEventListener('mousedown', this.mouseKeyStateChanged.bind(this));
        this.cursorContainer.addEventListener('mouseup', this.mouseKeyStateChanged.bind(this));
        
        this.cursorContainer.addEventListener('wheel', this.mouseWheelEvent.bind(this));
        this.cursorContainer.addEventListener('mousemove', this.cursorPosChanged.bind(this));
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

        var posX = this.canvas.width / this.canvasRect.w * (x - this.canvasRect.x);
        var posY = this.canvas.height / this.canvasRect.h * (y - this.canvasRect.y);

        this.dataManager.sendParameters(KEY_SET_CURSOR_POS,posX,posY);
    }
    // _____________________________________________________________
    
    getCanvas()
    {
        return this.canvas;
    }
    
    updateTransformRect()
    {
        this.transformRect.x = 0;
        this.transformRect.y = 0;
        this.transformRect.w = window.innerWidth;
        this.transformRect.h = window.innerHeight;
        
        this.updateSizes();
    }
    
    updateSizes()
    {
        var w = window.innerWidth;
        var h = window.innerHeight;
        var rect;

        if(this.transformRect.w < this.canvas.width || 
           this.transformRect.h < this.canvas.height)
        {
            rect = proportionalResizing(this.transformRect.x, this.transformRect.y,
                                        this.transformRect.w, this.transformRect.h,
                                        this.canvas.width, this.canvas.height);
        }
        else
        {
            rect = new Rect(this.transformRect.w/2 - this.canvas.width/2,
                            this.transformRect.h/2 - this.canvas.height/2,
                            this.canvas.width, this.canvas.height);
        }
        
        this.canvasRect = rect;
        var rectList = [this.canvas, this.cursorContainer];
        
        for(var i=0;i<rectList.length;++i)
        {
            rectList[i].style.left = rect.x + 'px';
            rectList[i].style.top = rect.y + 'px';
            rectList[i].style.width = rect.w + 'px';
            rectList[i].style.height = rect.h + 'px';
        }
    }
    
    setDataManager(dManager)
    {
        if(dManager)
            this.dataManager = dManager;
    }
}

