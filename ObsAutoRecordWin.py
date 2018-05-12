#!/usr/bin/env python
# Module      : ObsAutoRecordWin.py
# Description : Windows System tray icon app that auto records apps/games in OBS.
# Author      : Frederick Meyer
# Version     : 1.3.1
# Date        : 8 April 2018

import os
import sys
import time

import win32con
from infi.systray import SysTrayIcon

import ObsUtils
import win32api
import win32process
from ObsAutoRecord import ObsAutoRecord
from ObsAutoRecordState import ObsAutoRecordState

def get_resource_path(relative_path):
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

if __name__ == '__main__':
    obs_auto_record = ObsAutoRecord()
    def pause_menu_item_click(sysTrayIcon):
        obs_auto_record.on_pause_key()
    def pause_5_min(sysTrayIcon):
        obs_auto_record.on_pause_key(time.time() + 5 * 60)
    def pause_15_min(sysTrayIcon):
        obs_auto_record.on_pause_key(time.time() + 15 * 60)
    def do_nothing(sysTrayIcon):
        pass
    pause_5_min_option = ("Pause for 5 min", None, pause_5_min)
    pause_15_min_option = ("Pause for 15 min", None, pause_15_min)
    systray = SysTrayIcon(get_resource_path("record_red.ico"), "OBS Auto Record", (("Pause", None, pause_menu_item_click), pause_5_min_option, pause_15_min_option))
    def update_from_state(state):
        title, icon = ObsUtils.get_title_and_icon_from_state(state)
        menu_options = (("Resume" if state is ObsAutoRecordState.PAUSED else "Pause", None, pause_menu_item_click),)
        if state is ObsAutoRecordState.PAUSED:
            if obs_auto_record.pause_end_time > 0:
                seconds_left = int(obs_auto_record.pause_end_time - time.time())
                formatted_time = str(int(seconds_left / 60)).zfill(2) + ":" + str(seconds_left % 60).zfill(2)
                title = title + " " + formatted_time
                menu_options = menu_options + (("Paused time remaining: " + formatted_time, None, do_nothing),)
        else:
            menu_options = menu_options + (pause_5_min_option, pause_15_min_option)
        systray.update(hover_text=title, icon=get_resource_path(icon), menu_options=menu_options)
    obs_auto_record.set_on_state_change(update_from_state)
    systray.start()
