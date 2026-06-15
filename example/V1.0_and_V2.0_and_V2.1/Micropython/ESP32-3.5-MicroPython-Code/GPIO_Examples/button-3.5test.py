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




p16 = Pin(16, Pin.OUT)
p16.value(1)


disp = ili9488(miso=12, mosi=13, clk=14, cs=15, dc=2, rst=-1,backlight=27, backlight_on=1, power_on=1,
               spihost=VSPI_HOST, mhz=20,power=-1,
               factor=16, hybrid=True, width=320, height=480,
               invert=False, double_buffer=True, half_duplex=False)

touch = xpt2046(cs=33, spihost=VSPI_HOST, mosi=-1, miso=-1, clk=-1, cal_y0 = 423, cal_y1=3948)

scr = lv.obj()  # scr====> screen 
scr = lv.scr_act()
scr.clean()


btn = lv.btn(scr)  
btn.set_size(120, 50)  
btn.align(lv.ALIGN.CENTER,0,0)  
cnt = 0
def btn_event_cb(evt):
    code = evt.get_code()
    global cnt
    if code == lv.EVENT.CLICKED:
        #label = lv.label(btn)
        cnt += 1
        label.set_text("Button: " + str(cnt))   

btn.add_event(btn_event_cb, lv.EVENT.CLICKED, None)  

label = lv.label(btn)  
label.set_text("Button")  
label.center()  


lv.scr_load(scr)



