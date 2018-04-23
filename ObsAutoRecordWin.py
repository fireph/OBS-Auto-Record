#!/usr/bin/env python
# Module      : ObsAutoRecordWin.py
# Description : Windows System tray icon app that auto records apps/games in OBS.
# Author      : Frederick Meyer
# Version     : 1.3.1
# Date        : 8 April 2018

import os
import sys

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
    systray = SysTrayIcon(get_resource_path("record_red.ico"), "OBS Auto Record", (("Pause", None, pause_menu_item_click),))
    def update_from_state(state):
        title, icon = ObsUtils.get_title_and_icon_from_state(state)
        systray.update(
            hover_text=title,
            icon=get_resource_path(icon),
            menu_options=(("Resume" if state is ObsAutoRecordState.PAUSED else "Pause", None, pause_menu_item_click),))
    obs_auto_record.set_on_state_change(update_from_state)
    systray.start()
