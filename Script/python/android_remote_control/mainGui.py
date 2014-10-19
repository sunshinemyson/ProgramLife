import wx
import time
import subprocess
import os

app = wx.App(False)
#frame = wx.Frame(None, wx.ID_ANY, "RemoteController")

btnGapV = 5
btnGapH = 5

#TODO
class AdbConnection:
    def __init__(self):
        print "hello"

class RemoteDevices:
    AndroidKeyDict={'Home' : 3,\
                    'Back' : 4,\
                    'Power': 26,\
                        }
    AdbCmdInputKeyEvent="adb shell input keyevent "
    AdbCmdInputTouchScreenSwipe="adb shell input touchscreen swipe "
    AdbCmdInputTouchScreenTap="adb shell input touchscreen tap "
    def __init__(self):
        self.shellFile = "./catch_screen.sh"
    '''
    def __init__(self,dName, aCon):
        self.deviceName = dName
        self.adbConnection = aCon
    '''
    def UpdateScreen(self):
        s = time.clock()
        subprocess.call([self.shellFile], shell=True)
        print("UpdateScreen-- %.6f"%(time.clock()-s))

    def Click(self, point):
        command="%s %d %d"%(self.AdbCmdInputTouchScreenTap,point.x,point.y)
        subprocess.call([command], shell=True)

    def ClickHome(self):
        subprocess.call(["%s %d"%(self.AdbCmdInputKeyEvent,self.AndroidKeyDict['Home'])],shell=True)
        print("Click Home ---")

    def ClickBack(self):
        subprocess.call(["%s %d"%(self.AdbCmdInputKeyEvent,self.AndroidKeyDict['Back'])],shell=True)
        print("Click Back ---")

    def ClickPower(self):
        subprocess.call(["%s %d"%(self.AdbCmdInputKeyEvent,self.AndroidKeyDict['Power'])],shell=True)
        print("Click Power---")

    def DragMouse(self, start, end):
        subprocess.call(["%s %d"%(self.AdbCmdInputTouchScreenSwipe,start.x,start.y, end.x, end.y)],shell=True)

class RemoteView(wx.Window):
    def __init__(self, parent, id, pos, rDevice):
        wx.Window.__init__(self,parent,id,pos,size=(1024,600))
        #self.SetBackgroundColour()
        self.PngFile = "screenshot.png"
        self.remoteDevice=rDevice

        #Update Timer
        self.timerUpdate = wx.Timer(self)
        self.timerUpdate.Start(5000, False)

        #install event handler
        wx.EVT_PAINT(self, self.OnPaint)
        wx.EVT_LEFT_DOWN(self, self.OnMouse)
        wx.EVT_LEFT_UP(self, self.OnMouse)
        #wx.EVT_MOTION(self, self.OnMouse)
        self.Bind(wx.EVT_TIMER, self.OnTimeOut, self.timerUpdate)

    def OnPaint(self, event):
        self.pngimg = wx.Image( self.PngFile, wx.BITMAP_TYPE_PNG )
        self.bmp = wx.BitmapFromImage(self.pngimg)
        dc = wx.PaintDC(self)
        dc.DrawBitmap(self.bmp, 0, 0)

    def OnMouse(self, event):
        '''
        if( event.LeftDown() ):
            print "Left Mouse Down at (%d,%d)"%(event.GetX(),event.GetY())
        '''
        if( event.LeftUp() ):
            #print "Left Mouse Up at (%d,%d)"%(event.GetX(),event.GetY())
            self.remoteDevice.Click(wx.Point(event.GetX(),event.GetY()))
        #if( event.Moving() ):
            #print "Mouse moving"
            #do-nothing

    def OnTimeOut(self,event):
        print "onTimeOut..."
        print time.ctime()
        #1. Get Screen Short
        self.remoteDevice.UpdateScreen()
        self.Refresh()
    '''
    TODO: Auto Resize to screen size
    def GetAutoSize(self):
        print "Get Current screen size"
        self.screenSize = self.remoteDevice.GetScreenSize()
    '''

class MainWindow(wx.Frame):
    def __init__(self, parent, title):
        wx.Frame.__init__(self, parent,title=title, size=(300+1024,700) )
        #1. Layout control
        preCtrlHeight = 0
        #1.1 First Column
        self.deviceList=[]

                # Init devices list
        devFound = False
        while( devFound != True ):
            devProc=subprocess.Popen('adb devices', shell=True, stdout=subprocess.PIPE)

            devList=devProc.stdout.read()
            devListArray=devList.splitlines()
            for items in devListArray:
                for item in items.split():
                    if( len(item.strip()) == 16 ):
                        print("found device %s"%item)
                        self.deviceList.append(item.strip())
                        devFound = True
            if( devFound == False ):
                print("Please Make sure you have device connected!")
                dialog = wx.MessageDialog(self,message="Check Your Device Connection",caption="Device Not\
                                 Found")
                dialog.ShowModal()

        self.deviceRadioList = wx.RadioBox(\
                                     self,\
                                     id=-1,\
                                     pos=(btnGapH,btnGapH+preCtrlHeight),\
                                     choices=self.deviceList,
                                     majorDimension=1,\
                                     #size=(300,300),\
                                     #style =wx.RadioBox.RA_VERTICAL,\
                                    )
        #self.deviceRadioList.SetSelection( 0 )
        preCtrlHeight += btnGapV + self.deviceRadioList.GetVirtualSize().GetHeight()

        self.srchDeviceBtn = wx.Button(self, label="Search Devices", pos=(btnGapH,btnGapV+preCtrlHeight) )
        preCtrlHeight += btnGapV + self.srchDeviceBtn.GetVirtualSize().GetHeight()

        self.resetBtn = wx.Button(self, label="Reset", pos=(btnGapH,preCtrlHeight + btnGapV ))
        preCtrlHeight += btnGapV + self.resetBtn.GetVirtualSize().GetHeight()

        self.startRemoteContrl = wx.Button(self, id=100, label="Start Remote Contrllor", pos=(btnGapH, preCtrlHeight + btnGapV) )
        preCtrlHeight += btnGapV + self.resetBtn.GetVirtualSize().GetHeight()

        self.HomeButton = wx.Button(self, label="Home", pos = (btnGapH,preCtrlHeight+btnGapV))
        preCtrlHeight += btnGapV + self.HomeButton.GetVirtualSize().GetHeight()

        self.BackButton = wx.Button(self, label="Back", pos =(btnGapH,preCtrlHeight+btnGapV))
        preCtrlHeight += btnGapV + self.BackButton.GetVirtualSize().GetHeight()

        self.PowerButton= wx.Button(self, label="Power", pos=(btnGapH, preCtrlHeight + btnGapV ))

        # Install Event handler
        self.resetBtn.Bind(wx.EVT_BUTTON, self.onSearchDevices, self.resetBtn)
        self.srchDeviceBtn.Bind(wx.EVT_BUTTON, self.onSearchDevices, self.srchDeviceBtn)
        self.HomeButton.Bind(wx.EVT_BUTTON, self.onHomeButtonClick, self.HomeButton)
        self.BackButton.Bind(wx.EVT_BUTTON, self.onBackButtonClick, self.BackButton)
        self.PowerButton.Bind(wx.EVT_BUTTON, self.onPowerButtonClick, self.PowerButton)

        #2.2 Graphic View for remote picture
        #screenShortSize=wx.Size(1024,600)
        rDevice = RemoteDevices()
        screenShortPos = wx.Point(200,0)
        self.remoteView = RemoteView(self,101,pos=screenShortPos,rDevice = rDevice)
        self.remoteDevice = rDevice


    def onSearchDevices(self, event):
        if(self.resetBtn == event):
            print("OnResetButton")
        elif(self.srchDeviceBtn == event):
            print("onSearchDevices")
        else:
            print("Nothing")
    def onHomeButtonClick(self, event):
        self.remoteDevice.ClickHome()
    def onBackButtonClick(self, event):
        self.remoteDevice.ClickBack()
        def onPowerButtonClick(self, event):
            self.remoteDevice.ClickPower()


frame = MainWindow(None,"AndroidRemoteController")

frame.Show(True)

app.MainLoop()
