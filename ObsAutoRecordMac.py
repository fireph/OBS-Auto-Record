#!/usr/bin/env python
# Module      : ObsAutoRecordMac.py
# Description : Mac menu bar icon app that auto records apps/games in OBS.
# Author      : Frederick Meyer
# Version     : 1.2.0
# Date        : 11 February 2018

from ObsAutoRecord import ObsAutoRecord
import rumps

if __name__ == "__main__":
    obs_auto_record = ObsAutoRecord()
    app = rumps.App('OBS Auto Record', icon='record_red.ico')
    name_menu_item = rumps.MenuItem('OBS Auto Record')
    app.menu = [name_menu_item, None]
    def update_from_state(state):
        app.icon = 'record_green.ico' if state else 'record_red.ico'
        name_menu_item.title = 'OBS Auto Record ' + ('(connected)' if state else '(disconnected)')
    obs_auto_record.set_on_state_change(update_from_state)
    app.run()
