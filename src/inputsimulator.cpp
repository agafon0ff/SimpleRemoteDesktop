#include "inputsimulator.h"
#include <QDebug>
#include <QCursor>

#ifdef Q_OS_WIN
#include "windows.h"
#endif

#ifdef Q_OS_UNIX
//sudo apt install libxtst-dev
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

InputSimulator::InputSimulator(QObject *parent) : QObject(parent),
    m_screenPosition(QPoint(0,0))
{
#ifdef Q_OS_UNIX
    createKeysMap();
#endif
}

void InputSimulator::simulateKeyboard(quint16 keyCode, bool state)
{
#ifdef Q_OS_UNIX
    if(!m_keysMap.contains(keyCode))
    {
        qDebug()<<"InputSimulator::simulateKeyboard"<<keyCode<<state;
        return;
    }

    Display *display;
    display = XOpenDisplay(Q_NULLPTR);

    unsigned int keycode = XKeysymToKeycode(display, m_keysMap.value(keyCode));

    if(state)
        XTestFakeKeyEvent(display, keycode, True, 0);
    else XTestFakeKeyEvent(display, keycode, False, 0);

    XFlush(display);
    XCloseDisplay(display);
#endif


#ifdef Q_OS_WIN
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    ip.ki.wVk = static_cast<unsigned short>(keyCode);

    if(state)
        ip.ki.dwFlags = 0;
    else ip.ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &ip, sizeof(INPUT));
#endif
}

void InputSimulator::simulateMouseKeys(quint16 keyCode, bool state)
{
#ifdef Q_OS_UNIX
    Display *display;
    display = XOpenDisplay(Q_NULLPTR);

    if(keyCode == 0)//left
        XTestFakeButtonEvent(display,Button1,state,0);
    else if(keyCode == 1)//middle
        XTestFakeButtonEvent(display,Button2,state,0);
    else if(keyCode == 2)//right
        XTestFakeButtonEvent(display,Button3,state,0);

    XFlush(display);
    XCloseDisplay(display);
#endif

#ifdef Q_OS_WIN
    INPUT ip;

    ZeroMemory(&ip,sizeof(ip));

    ip.type = INPUT_MOUSE;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    if(keyCode == 0)//left
    {
        if(state)ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        else ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    }
    else if(keyCode == 1)//middle
    {
        if(state)ip.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
        else ip.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
    }
    else if(keyCode == 2)//right
    {
        if(state)ip.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        else ip.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    }

    SendInput(1, &ip, sizeof(INPUT));
#endif
}

void InputSimulator::simulateMouseMove(quint16 posX, quint16 posY)
{
    QCursor::setPos(m_screenPosition.x()+posX,m_screenPosition.y()+posY);
}

void InputSimulator::simulateWheelEvent(bool deltaPos)
{
#ifdef Q_OS_UNIX
    Display *display;
    display = XOpenDisplay(Q_NULLPTR);

    quint32 btnNum = Button4;
    if(deltaPos)btnNum = Button5;

    XTestFakeButtonEvent(display,btnNum,true,0);
    XTestFakeButtonEvent(display,btnNum,false,0);

    XFlush(display);
    XCloseDisplay(display);
#endif

#ifdef Q_OS_WIN
    INPUT ip;

    ZeroMemory(&ip,sizeof(ip));

    ip.type = INPUT_MOUSE;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    ip.mi.dwFlags = MOUSEEVENTF_WHEEL;

    if(deltaPos)ip.mi.mouseData = static_cast<DWORD>(-120);
    else ip.mi.mouseData = 120;

    SendInput(1, &ip, sizeof(INPUT));
#endif
}

void InputSimulator::setMouseDelta(qint16 deltaX, qint16 deltaY)
{
    QPoint cursorPos = QCursor::pos();
    quint16 posX = static_cast<quint16>(cursorPos.x() - deltaX);
    quint16 posY = static_cast<quint16>(cursorPos.y() - deltaY);
    QCursor::setPos(posX,posY);
}

void InputSimulator::createKeysMap()
{
#ifdef Q_OS_UNIX
    m_keysMap.insert(8,XK_BackSpace);
    m_keysMap.insert(9,XK_Tab);
    m_keysMap.insert(13,XK_Return);
    m_keysMap.insert(16,XK_Shift_L);
    m_keysMap.insert(17,XK_Control_L);
    m_keysMap.insert(18,XK_Alt_L);
    m_keysMap.insert(44,XK_Pause);
    m_keysMap.insert(20,XK_Caps_Lock);
    m_keysMap.insert(27,XK_Escape);

    m_keysMap.insert(32,XK_space);
    m_keysMap.insert(35,XK_End);
    m_keysMap.insert(36,XK_Home);
    m_keysMap.insert(37,XK_Left);
    m_keysMap.insert(38,XK_Up);
    m_keysMap.insert(39,XK_Right);
    m_keysMap.insert(40,XK_Down);

    m_keysMap.insert(44,XK_Print);
    m_keysMap.insert(45,XK_Insert);
    m_keysMap.insert(46,XK_Delete);

    m_keysMap.insert(48,XK_0);
    m_keysMap.insert(49,XK_1);
    m_keysMap.insert(50,XK_2);
    m_keysMap.insert(51,XK_3);
    m_keysMap.insert(52,XK_4);
    m_keysMap.insert(53,XK_5);
    m_keysMap.insert(54,XK_6);
    m_keysMap.insert(55,XK_7);
    m_keysMap.insert(56,XK_8);
    m_keysMap.insert(57,XK_9);

    m_keysMap.insert(65,XK_A);
    m_keysMap.insert(66,XK_B);
    m_keysMap.insert(67,XK_C);
    m_keysMap.insert(68,XK_D);
    m_keysMap.insert(69,XK_E);
    m_keysMap.insert(70,XK_F);
    m_keysMap.insert(71,XK_G);
    m_keysMap.insert(72,XK_H);
    m_keysMap.insert(73,XK_I);
    m_keysMap.insert(74,XK_J);
    m_keysMap.insert(75,XK_K);
    m_keysMap.insert(76,XK_L);
    m_keysMap.insert(77,XK_M);
    m_keysMap.insert(78,XK_N);
    m_keysMap.insert(79,XK_O);
    m_keysMap.insert(80,XK_P);
    m_keysMap.insert(81,XK_Q);
    m_keysMap.insert(82,XK_R);
    m_keysMap.insert(83,XK_S);
    m_keysMap.insert(84,XK_T);
    m_keysMap.insert(85,XK_U);
    m_keysMap.insert(86,XK_V);
    m_keysMap.insert(87,XK_W);
    m_keysMap.insert(88,XK_X);
    m_keysMap.insert(89,XK_Y);
    m_keysMap.insert(90,XK_Z);

    m_keysMap.insert(91,XK_Super_L);
    m_keysMap.insert(93,XK_Menu);

    m_keysMap.insert(96,XK_KP_0);
    m_keysMap.insert(97,XK_KP_1);
    m_keysMap.insert(98,XK_KP_2);
    m_keysMap.insert(99,XK_KP_3);
    m_keysMap.insert(100,XK_KP_4);
    m_keysMap.insert(101,XK_KP_5);
    m_keysMap.insert(102,XK_KP_6);
    m_keysMap.insert(103,XK_KP_7);
    m_keysMap.insert(104,XK_KP_8);
    m_keysMap.insert(105,XK_KP_9);

    m_keysMap.insert(112,XK_F1);
    m_keysMap.insert(113,XK_F2);
    m_keysMap.insert(114,XK_F3);
    m_keysMap.insert(115,XK_F4);
    m_keysMap.insert(116,XK_F5);
    m_keysMap.insert(117,XK_F6);
    m_keysMap.insert(118,XK_F7);
    m_keysMap.insert(119,XK_F8);
    m_keysMap.insert(120,XK_F9);
    m_keysMap.insert(121,XK_F10);
    m_keysMap.insert(122,XK_F11);
    m_keysMap.insert(123,XK_F12);

    m_keysMap.insert(144,XK_Num_Lock);
    m_keysMap.insert(145,XK_Scroll_Lock);

    m_keysMap.insert(179,179);//Play/pause
    m_keysMap.insert(173,173);//Mute
    m_keysMap.insert(174,174);//Volume-
    m_keysMap.insert(175,175);//Volume+

    m_keysMap.insert(186,XK_semicolon);
    m_keysMap.insert(187,XK_equal);
    m_keysMap.insert(188,XK_comma);
    m_keysMap.insert(189,XK_minus);

    m_keysMap.insert(190,XK_greater);
    m_keysMap.insert(191,XK_question);
    m_keysMap.insert(192,XK_asciitilde);

    m_keysMap.insert(219,XK_bracketleft);
    m_keysMap.insert(220,XK_backslash);
    m_keysMap.insert(221,XK_bracketright);
    m_keysMap.insert(222,XK_apostrophe);
    m_keysMap.insert(226,XK_bar);
#endif
}
