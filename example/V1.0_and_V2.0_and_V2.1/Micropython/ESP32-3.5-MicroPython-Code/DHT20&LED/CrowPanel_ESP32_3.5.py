"""
ELECROW
www.elecrow.com

"""

import lvgl as lv
import time
from espidf import VSPI_HOST
from ili9XXX import ili9488
from xpt2046 import xpt2046 
from machine import Pin, I2C
#import ui_images
import DHT20

i2c = I2C(scl=Pin(21), sda=Pin(22))
sensor = DHT20.DHT20(i2c)
p25 = Pin(25, Pin.OUT)
p25.value(1)


disp = ili9488(miso=12, mosi=13, clk=14, cs=15, dc=2, rst=-1,backlight=27, backlight_on=1, power_on=1,rot=0xa0|0x40,
               spihost=VSPI_HOST, mhz=20,power=-1,
               factor=16, hybrid=True, width=480, height=320,
               invert=False, double_buffer=True, half_duplex=False)

touch = xpt2046(cs=33, spihost=VSPI_HOST, mosi=-1, miso=-1, clk=-1, cal_x0 = 353, cal_x1=3568,cal_y0 = 269, cal_y1=3491,transpose = 0)


dispp = lv.disp_get_default()
theme = lv.theme_default_init(dispp, lv.palette_main(lv.PALETTE.BLUE), lv.palette_main(lv.PALETTE.RED), False, lv.font_default())
dispp.set_theme(theme)

def SetFlag( obj, flag, value):
    if (value):
        obj.add_flag(flag)
    else:
        obj.clear_flag(flag)
    return

_ui_comp_table = {}
_ui_comp_prev = None
_ui_name_prev = None
_ui_child_prev = None
_ui_comp_table.clear()

def _ui_comp_del_event(e):
    target = e.get_target()
    _ui_comp_table[id(target)].remove()

def ui_comp_get_child(comp, child_name):
    return _ui_comp_table[id(comp)][child_name]

def ui_comp_get_root_from_child(child, compname):
    for component in _ui_comp_table:
        if _ui_comp_table[component]["_CompName"]==compname:
            for part in _ui_comp_table[component]:
                if id(_ui_comp_table[component][part]) == id(child):
                    return _ui_comp_table[component]
    return None
def SetBarProperty(target, id, val):
   if id == 'Value_with_anim': target.set_value(val, lv.ANIM.ON)
   if id == 'Value': target.set_value(val, lv.ANIM.OFF)
   return

def SetPanelProperty(target, id, val):
   if id == 'Position_X': target.set_x(val)
   if id == 'Position_Y': target.set_y(val)
   if id == 'Width': target.set_width(val)
   if id == 'Height': target.set_height(val)
   return

def SetDropdownProperty(target, id, val):
   if id == 'Selected':
      target.set_selected(val)
   return

def SetImageProperty(target, id, val, val2):
   if id == 'Image': target.set_src(val)
   if id == 'Angle': target.set_angle(val2)
   if id == 'Zoom': target.set_zoom(val2)
   return

def SetLabelProperty(target, id, val):
   if id == 'Text': target.set_text(val)
   return

def SetRollerProperty(target, id, val):
   if id == 'Selected':
      target.set_selected(val, lv.ANIM.OFF)
   if id == 'Selected_with_anim':
      target.set_selected(val, lv.ANIM.ON)
   return

def SetSliderProperty(target, id, val):
   if id == 'Value_with_anim': target.set_value(val, lv.ANIM.ON)
   if id == 'Value': target.set_value(val, lv.ANIM.OFF)
   return

def ChangeScreen( src, fademode, speed, delay):
    lv.scr_load_anim(src, fademode, speed, delay, False)
    return

def DeleteScreen(src):
    return

def IncrementArc( trg, val):
    trg.set_value(trg.get_value()+val)
    lv.event_send(trg,lv.EVENT.VALUE_CHANGED, None)
    return

def IncrementBar( trg, val, anim):
    trg.set_value(trg.get_value()+val,anim)
    return

def IncrementSlider( trg, val, anim):
    trg.set_value(trg.get_value()+val,anim)
    lv.event_send(trg,lv.EVENT.VALUE_CHANGED, None)
    return

def KeyboardSetTarget( keyboard, textarea):
    keyboard.set_textarea(textarea)
    return

def ModifyFlag( obj, flag, value):
    if (value=="TOGGLE"):
        if ( obj.has_flag(flag) ):
            obj.clear_flag(flag)
        else:
            obj.add_flag(flag)
        return

    if (value=="ADD"):
        obj.add_flag(flag)
    else:
        obj.clear_flag(flag)
    return

def ModifyState( obj, state, value):
    if (value=="TOGGLE"):
        if ( obj.has_state(state) ):
            obj.clear_state(state)
        else:
            obj.add_state(state)
        return

    if (value=="ADD"):
        obj.add_state(state)
    else:
        obj.clear_state(state)
    return

def set_opacity(obj, v):
    obj.set_style_opa(v, lv.STATE.DEFAULT|lv.PART.MAIN)
    return

def SetTextValueArc( trg, src, prefix, postfix):
    trg.set_text(prefix+str(src.get_value())+postfix)
    return

def SetTextValueSlider( trg, src, prefix, postfix):
    trg.set_text(prefix+str(src.get_value())+postfix)
    return

def SetTextValueChecked( trg, src, txton, txtoff):
    if src.has_state(lv.STATE.CHECKED):
        trg.set_text(txton)
    else:
        trg.set_text(txtoff)
    return

def StepSpinbox( trg, val):
    if val==1 : trg.increment()
    if val==-1 : trg.decrement()
    lv.event_send(trg,lv.EVENT.VALUE_CHANGED, None)
    return

# COMPONENTS

 # COMPONENT Button2
def ui_Button2_create(comp_parent):
    cui_Button2 = lv.btn(comp_parent)
    cui_Button2.set_width(100)
    cui_Button2.set_height(50)
    cui_Button2.set_x(4)
    cui_Button2.set_y(32)
    cui_Button2.set_align( lv.ALIGN.CENTER)
    SetFlag(cui_Button2, lv.obj.FLAG.SCROLLABLE, False)
    SetFlag(cui_Button2, lv.obj.FLAG.SCROLL_ON_FOCUS, True)
    _ui_comp_table[id(cui_Button2)]= {"Button2" : cui_Button2, "_CompName" : "Button2"}
    return cui_Button2

ui____initial_actions0 = lv.obj()

def Panel1_eventhandler(event_struct):
   event = event_struct.code
   if event == lv.EVENT.CLICKED and True:
      p25.value(1)
   return

def Panel2_eventhandler(event_struct):
   event = event_struct.code
   if event == lv.EVENT.CLICKED and True:
      p25.value(0)
   return

ui_Screen1 = lv.obj()
SetFlag(ui_Screen1, lv.obj.FLAG.SCROLLABLE, False)
ui_Screen1.set_style_bg_color(lv.color_hex(0x91E9F4), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Screen1.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )

#ui_Screen1.set_style_bg_img_src( ui_images.TemporaryImage, lv.PART.SCROLLBAR | lv.STATE.DEFAULT )

ui_Label2 = lv.label(ui_Screen1)
ui_Label2.set_text("TEMP:                       °C")
ui_Label2.set_width(172)
ui_Label2.set_height(35)
ui_Label2.set_x(-103)
ui_Label2.set_y(-13)
ui_Label2.set_align( lv.ALIGN.CENTER)
ui_Label2.set_style_text_color(lv.color_hex(0xFDFDFE), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label2.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label2.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label2.set_style_radius( 5, lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label2.set_style_bg_color(lv.color_hex(0x1ED45C), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label2.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )

ui_Label3 = lv.label(ui_Screen1)
ui_Label3.set_text("HUM:                          %")
ui_Label3.set_width(172)
ui_Label3.set_height(35)
ui_Label3.set_x(-103)
ui_Label3.set_y(66)
ui_Label3.set_align( lv.ALIGN.CENTER)
ui_Label3.set_style_text_color(lv.color_hex(0xF7FE13), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label3.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label3.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label3.set_style_radius( 5, lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label3.set_style_bg_color(lv.color_hex(0x5390CC), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label3.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )

ui_Label4 = lv.label(ui_Screen1)
ui_Label4.set_text("23")
ui_Label4.set_width(33)
ui_Label4.set_height(23)
ui_Label4.set_x(-74)
ui_Label4.set_y(-16)
ui_Label4.set_align( lv.ALIGN.CENTER)
ui_Label4.set_style_text_color(lv.color_hex(0xE1F665), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label4.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label4.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )

ui_Label5 = lv.label(ui_Screen1)
ui_Label5.set_text("50")
ui_Label5.set_width(33)
ui_Label5.set_height(23)
ui_Label5.set_x(-74)
ui_Label5.set_y(65)
ui_Label5.set_align( lv.ALIGN.CENTER)
ui_Label5.set_style_text_color(lv.color_hex(0xE1F492), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label5.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label5.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )

ui_Panel1 = lv.obj(ui_Screen1)
ui_Panel1.set_width(50)
ui_Panel1.set_height(50)
ui_Panel1.set_x(102)
ui_Panel1.set_y(-9)
ui_Panel1.set_align( lv.ALIGN.CENTER)
SetFlag(ui_Panel1, lv.obj.FLAG.SCROLLABLE, False)
ui_Panel1.set_style_bg_color(lv.color_hex(0xEA1E1E), lv.PART.MAIN | lv.STATE.PRESSED )
ui_Panel1.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.PRESSED )

ui_Label6 = lv.label(ui_Panel1)
ui_Label6.set_text("ON")
ui_Label6.set_width(25)
ui_Label6.set_height(20)
ui_Label6.set_align( lv.ALIGN.CENTER)
ui_Label6.set_style_text_font( lv.font_montserrat_14, lv.PART.MAIN | lv.STATE.DEFAULT )

ui_Panel1.add_event(Panel1_eventhandler, lv.EVENT.ALL, None)
ui_Panel2 = lv.obj(ui_Screen1)
ui_Panel2.set_width(50)
ui_Panel2.set_height(50)
ui_Panel2.set_x(102)
ui_Panel2.set_y(72)
ui_Panel2.set_align( lv.ALIGN.CENTER)
SetFlag(ui_Panel2, lv.obj.FLAG.SCROLLABLE, False)
ui_Panel2.set_style_bg_color(lv.color_hex(0xB0A8A8), lv.PART.MAIN | lv.STATE.PRESSED )
ui_Panel2.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.PRESSED )

ui_Label7 = lv.label(ui_Panel2)
ui_Label7.set_text("OFF")
ui_Label7.set_width(30)
ui_Label7.set_height(20)
ui_Label7.set_align( lv.ALIGN.CENTER)

ui_Panel2.add_event(Panel2_eventhandler, lv.EVENT.ALL, None)
ui_Panel3 = lv.obj(ui_Screen1)
ui_Panel3.set_width(300)
ui_Panel3.set_height(40)
ui_Panel3.set_x(3)
ui_Panel3.set_y(-90)
ui_Panel3.set_align( lv.ALIGN.CENTER)
SetFlag(ui_Panel3, lv.obj.FLAG.SCROLLABLE, False)
ui_Panel3.set_style_bg_color(lv.color_hex(0x4161F6), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Panel3.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )

ui_Label1 = lv.label(ui_Panel3)
ui_Label1.set_text("Weather and Lamp")
ui_Label1.set_width(170)
ui_Label1.set_height(25)
ui_Label1.set_align( lv.ALIGN.CENTER)
ui_Label1.set_style_text_color(lv.color_hex(0xFCFBFB), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label1.set_style_text_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )
ui_Label1.set_style_text_font( lv.font_montserrat_16, lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label1.set_style_radius( 5, lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label1.set_style_bg_color(lv.color_hex(0x4260F1), lv.PART.MAIN | lv.STATE.DEFAULT )
ui_Label1.set_style_bg_opa(255, lv.PART.MAIN| lv.STATE.DEFAULT )



sensor.read_dht20()
class TEM_HUM():
    def __init__(self, ui_Screen1):
        # 读取DHT20传感器的温湿度值
        global Tem, Hum
        Tem = sensor.dht20_temperature()
        Hum = sensor.dht20_humidity()
        
        # 更新UI界面上的温湿度显示
        ui_Label4.set_text(f"{round(Tem)} °C")  # 更新温度显示
        ui_Label5.set_text(f"{round(Hum)} %")       # 更新湿度显示

# 确保TEM_HUM类在程序中被实例化，并且传入了ui_Screen1作为参数
def update_temperature_humidity():
    global Tem, Hum
    # 读取DHT20传感器的温湿度值
    sensor.read_dht20()  # 确保调用了read_dht20()来更新传感器数据
    Tem = sensor.dht20_temperature()
    Hum = sensor.dht20_humidity()
    
    # 更新UI界面上的温湿度显示
    ui_Label4.set_text(f"{round(Tem)}")
    ui_Label5.set_text(f"{round(Hum)}")
    
def main_loop():
    while True:
        update_temperature_humidity()
        time.sleep(1)  # 暂停10秒钟，然后再次更新

TEM_HUM(ui_Screen1)

# 其余代码保持不变


lv.scr_load(ui_Screen1)
main_loop()
