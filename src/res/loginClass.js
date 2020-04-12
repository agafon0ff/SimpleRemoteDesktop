class LoginClass 
{
    constructor()
    {
        this.loginField = null;
        this.dataManager = null;
        this.nonce = null;
    }
    
    createLoginHtml()
    {
        this.loginField = document.createElement('div');
        this.loginField.id = 'loginField';
        this.loginField.style.cssText = 'position:absolute;\
            margin:0; \
            padding:0; \
            left:0; \
            top:0; \
            width:100%; \
            height:100%; \
            z-index:10; \
            background: #111;';
        
        this.titleLabel = document.createElement('div');
        this.titleLabel.id = 'titleLabel';
        this.titleLabel.innerHTML = 'Simple Remote Desktop';
        this.titleLabel.style.cssText = 'position: absolute; \
            background: none; \
            color: white; \
            font: 16px monospace; \
            text-align: center; \
            letter-spacing: 1px; \
            line-height: 30px; \
            border: none; \
            top: calc(50% - 135px); \
            left: calc(50% - 130px); \
            width: 300px; \
            height: 30px;';
        this.loginField.append(this.titleLabel);
        
        this.titleImg = document.createElement("img");
        this.titleImg.id = 'titleImg';
        this.titleImg.src = "favicon.ico";
        this.titleImg.style.cssText = 'position: absolute; \
            border: none; \
            top: calc(50% - 135px); \
            left: calc(50% - 135px); \
            width: 30px; \
            height: 30px;';
        this.loginField.append(this.titleImg);

        this.loginBox = document.createElement('div');
        this.loginBox.id = 'loginBox';
        this.loginBox.style.cssText = 'position: absolute; \
            background: #333; \
            color: white; \
            border: 1px solid #777; \
            border-radius: 5px; \
            top: calc(50% - 100px); \
            left: calc(50% - 150px); \
            width: 300px; \
            height: 200px;';

        var cssLabel = 'position: absolute; \
            font: 16px monospace; \
            color: #ddd; \
            left: 6%; \
            width: 88%; \
            height: 13%; \
            letter-spacing: 1px;';
        
        this.labelLogin = document.createElement('label');
        this.labelLogin.id = 'labelLogin';
        this.labelLogin.innerHTML = 'Login:';
        this.labelLogin.style.cssText = cssLabel + 'top: 9%;';
        this.loginBox.append(this.labelLogin);
        
        this.labelPass = document.createElement('label');
        this.labelPass.id = 'labelPass';
        this.labelPass.innerHTML = 'Password:';
        this.labelPass.style.cssText = cssLabel + 'top: 38%;';
        this.loginBox.append(this.labelPass);
        
        var cssInput = 'position: absolute; \
            font: 16px monospace; \
            background: #556; \
            color: #fff; \
            text-align: center; \
            left: 5%; \
            width: 90%; \
            height: 13%; \
            border: 1px solid #666; \
            border-radius: 5px; \
            letter-spacing: 1px;';
        
        this.inputLogin = document.createElement('input');
        this.inputLogin.id = 'inputLogin';
        this.inputLogin.type = 'text';
        this.inputLogin.attributes.required = "requiredd";
        this.inputLogin.style.cssText = cssInput + 'top: 20%;';
        this.loginBox.append(this.inputLogin);
        
        this.inputPass = document.createElement('input');
        this.inputPass.id = 'inputPass';
        this.inputPass.type = 'password';
        this.inputPass.attributes.required = "requiredd";
        this.inputPass.style.cssText = cssInput + 'top: 50%;';
        this.loginBox.append(this.inputPass);
        
        this.btnSubmit = document.createElement('input');
        this.btnSubmit.id = 'btnSubmit';
        this.btnSubmit.type = 'submit';
        this.btnSubmit.value = 'Submit';
        this.btnSubmit.style.cssText = 'position: absolute; \
            background: #676; \
            font: 16px monospace; \
            color: white; \
            border: 1px solid #777; \
            border-radius: 5px; \
            top: 75%; \
            left: 20%; \
            width: 60%; \
            height: 16%;';
        
        this.btnSubmit.onmouseover = function(){this.style.borderColor = "#aaa";}
        this.btnSubmit.onmouseout = function() {this.style.borderColor = "#777";}
        this.btnSubmit.addEventListener('mouseup', this.btnSubmitClicked.bind(this));
        
        this.loginBox.append(this.btnSubmit);
        
        this.loginField.append(this.loginBox);
        
        this.statusLabel = document.createElement('div');
        this.statusLabel.id = 'titleLabel';
        this.statusLabel.style.cssText = 'position: absolute; \
            background: none; \
            color: #900; \
            text-align: center; \
            font: 12px monospace; \
            letter-spacing: 1px; \
            line-height: 30px; \
            border: none; \
            top: calc(50% + 110px); \
            left: calc(50% - 150px); \
            width: 300px; \
            height: 30px;';
        this.loginField.append(this.statusLabel);
        
        document.body.append(this.loginField);
        window.addEventListener("keyup", this.keyStateChanged.bind(this));
    }

    removeLoginHtml()
    {
        this.loginField.remove();
    }

    showWaitImage()
    {
        if(this.waitLabel || !this.loginBox)
            return;

        this.waitLabel = document.createElement('div');
        this.waitLabel.id = 'waitLabel';
        this.waitLabel.innerHTML = '<svg id="svgProcess" width="38" height="38" viewBox="0 0 38 38" xmlns="http://www.w3.org/2000/svg"> \
            <circle fill="#fff" cx="8" cy="18" r="2"> \
                <animate attributeName="opacity" dur="1s" values="0;1;0" repeatCount="indefinite" begin="0.1"/></circle> \
            <circle fill="#fff" cx="19" cy="18" r="2"> \
                <animate attributeName="opacity" dur="1s" values="0;1;0" repeatCount="indefinite" begin="0.2"/></circle></circle> \
                <circle fill="#fff" cx="30" cy="18" r="2"> \
            <animate attributeName="opacity" dur="1s" values="0;1;0" repeatCount="indefinite" begin="0.3"/></circle></circle> \
            </svg>';
        this.waitLabel.style.cssText = 'position: absolute; \
            background: rgba(4,4,4,0.9); \
            border-radius: 5px; \
            top: 1px; \
            left: 1px; \
            width: calc(100% - 2px); \
            height: calc(100% - 2px); \
            z-index: 10;';
        this.loginBox.append(this.waitLabel);
    }

    removeWaitImage()
    {
        if(this.waitLabel)
            this.waitLabel.remove();
    }
    
    showWrongRequest()
    {
        this.removeWaitImage();
        this.inputLogin.value = '';
        this.inputPass.value = '';
        this.statusLabel.innerHTML = 'ERROR: wrong login or password!';
        console.log("Wrong login pass!");
    }
    
    setNonce(nonce)
    {
        this.nonce = nonce;
    }
    
    addDesktopButton(uuid, name)
    {
        this.removeLoginHtml();

        if(!this.buttonsBox)
        {
            this.buttonsBox = document.createElement('div');
            this.buttonsBox.id = 'buttonsBox';
            this.buttonsBox.style.cssText = 'position: absolute; \
                background: none; \
                color: white; \
                border: none; \
                top: calc(50% - 100px); \
                left: calc(50% - 150px); \
                width: 300px; \
                height: 200px;';
        }

        this.desktopButton = document.createElement('div');
        this.desktopButton.innerHTML = name;
        this.desktopButton.classList.add("btnDesktop");
        this.desktopButton.classList.add("button");
        this.desktopButton.style.cssText = 'position: absolute; \
            background: #444; \
            font: 16px monospace; \
            color: white; \
            border: 1px solid #777; \
            border-radius: 5px; \
            text-align: center; \
            line-height: 34px; \
            margin: 5px; \
            width: 285px; \
            height: 34px;';
        this.desktopButton.onmouseover = function(){this.style.backgroundColor = "#555";}
        this.desktopButton.onmouseout = function() {this.style.backgroundColor = "#444";}
        this.desktopButton.onmousedown = this.buttonDesktopClicked.bind(this, uuid);

        this.buttonsBox.append(this.desktopButton);
        document.body.append(this.buttonsBox);

        var btnHeight = 50;
        var btnsDesktopList = document.getElementsByClassName('btnDesktop');
        var btnsCount = btnsDesktopList.length;
        var boxWidth = btnsCount * btnHeight;
        this.buttonsBox.style.top = 'calc(50% - '+ boxWidth/2 +'px)';
        this.buttonsBox.style.height = boxWidth +'px';

        for(var i=0;i<btnsCount;++i)
            btnsDesktopList[i].style.top = i * btnHeight + 'px';
    }

    setDataManager(dManager)
    {
        if(dManager)
            this.dataManager = dManager;
    }
    
    btnSubmitClicked()
    {
        for(var i=0;i<5;++i)
            this.addDesktopButton("qweqwe"+i, "ASD asd ZXC zxc"+i);

        return;

        this.showWaitImage();

        var concatFirst = btoa(rstr_md5(this.inputLogin.value + this.inputPass.value));
        var concatSecond = btoa(rstr_md5(concatFirst + this.nonce));
        
        var binaryString = atob(concatSecond);
        var requestSize = binaryString.length;
        var key = KEY_SET_AUTH_REQUEST;
        
        var buf = new Uint8Array(requestSize + 6);
        buf[0] = key[0];
        buf[1] = key[1];
        buf[2] = key[2];
        buf[3] = key[3];
        buf[4] = requestSize;
        buf[5] = requestSize >> 8;
        
        for(var i=0;i<requestSize;++i)
            buf[i + 6] = binaryString.charCodeAt(i);
        
        if(this.dataManager)
            this.dataManager.sendToSocket(buf);
    }
    
    keyStateChanged(event)
    {
        if(this.waitLabel)
            return;

        if(event.keyCode == 13)
            this.btnSubmitClicked();
    }

    buttonDesktopClicked(uuid)
    {
        console.log('button uuid:', uuid);
    }
}
