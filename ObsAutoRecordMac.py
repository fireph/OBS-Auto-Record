#!/usr/bin/env python
# Module      : ObsAutoRecordMac.py
# Description : Mac menu bar icon app that auto records apps/games in OBS.
# Author      : Frederick Meyer
# Version     : 1.2.0
# Date        : 11 February 2018

from ObsAutoRecord import ObsAutoRecord
import ObsUtils
import rumps

if __name__ == "__main__":
    obs_auto_record = ObsAutoRecord()
    app = rumps.App('OBS Auto Record', icon='record_red.ico')
    name_menu_item = rumps.MenuItem('OBS Auto Record')
    app.menu = [name_menu_item, None]
    def update_from_state(state):
        title, icon = ObsUtils.get_title_and_icon_from_state(state)
        name_menu_item.title = title
        app.icon = icon
    obs_auto_record.set_on_state_change(update_from_state)
    app.run()
