#!/usr/bin/env python
# Module      : ObsAutoRecordWin.py
# Description : Windows System tray icon app that auto records apps/games in OBS.
# Author      : Frederick Meyer
# Version     : 1.3.1
# Date        : 8 April 2018

import os

import pkg_resources
import win32con
from infi.systray import SysTrayIcon

import ObsUtils
import win32api
import win32process
from ObsAutoRecord import ObsAutoRecord

def get_resource_path(relative_path):
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

if __name__ == '__main__':
    obs_auto_record = ObsAutoRecord()
    systray = SysTrayIcon(get_resource_path("record_red.ico"), "OBS Auto Record", ())
    def update_from_state(state):
        title, icon = ObsUtils.get_title_and_icon_from_state(state)
        systray.update(hover_text=title, icon=get_resource_path(icon))
    obs_auto_record.set_on_state_change(update_from_state)
    systray.start()
