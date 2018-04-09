import configparser
import platform

from ObsAutoRecordState import ObsAutoRecordState

def get_apps_to_record():
    config = configparser.ConfigParser()
    config.read('settings.ini')
    default_apps = ''
    if platform.system() == 'Windows':
        default_apps = ', '.join(['destiny2.exe', 'HeroesOfTheStorm*.exe', 'Wow*.exe'])
    elif platform.system() == 'Darwin':
        default_apps = ', '.join(['Heroes of the Storm', 'Hearthstone'])
    if 'DEFAULT' not in config:
        config['DEFAULT'] = {'apps_to_record': default_apps}
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
    if 'apps_to_record' not in config['DEFAULT']:
        config['DEFAULT']['apps_to_record'] = default_apps
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
    apps = config['DEFAULT']['apps_to_record'].split(',')
    return [app.strip() for app in apps]

def get_address():
    config = configparser.ConfigParser()
    config.read('settings.ini')
    default_address = 'localhost:4444'
    if 'DEFAULT' not in config:
        config['DEFAULT'] = {'address': default_address}
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
    if not ("address" in config['DEFAULT']):
        config['DEFAULT']['address'] = default_address
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
    return config['DEFAULT']['address']

def get_folder():
    config = configparser.ConfigParser()
    config.read('settings.ini')
    if 'DEFAULT' in config and 'folder' in config['DEFAULT']:
        return config['DEFAULT']['folder'].replace('\\', '/')
    else:
        return None

def get_app_name_win(exe_path, fallback):
    blacklisted_file_descriptions = {"TslGame"}
    try:
        import win32api
        language, codepage = win32api.GetFileVersionInfo(exe_path, '\\VarFileInfo\\Translation')[0]
        string_file_info_desc = u'\\StringFileInfo\\%04X%04X\\%s' % (language, codepage, "FileDescription")
        file_description = win32api.GetFileVersionInfo(exe_path, string_file_info_desc)
        string_file_info_name = u'\\StringFileInfo\\%04X%04X\\%s' % (language, codepage, "ProductName")
        product_name = win32api.GetFileVersionInfo(exe_path, string_file_info_name)
        if isNotEmpty(file_description) and file_description not in blacklisted_file_descriptions:
            app_name = file_description
        elif isNotEmpty(product_name):
            app_name = product_name
        else:
            app_name = fallback
    except:
        app_name = fallback
    return app_name

def get_app_name_from_process(proc):
    if platform.system() == 'Windows':
        import win32api, win32con, win32process
        win32proc = win32api.OpenProcess(win32con.PROCESS_ALL_ACCESS, False, proc.pid)
        procpath = win32process.GetModuleFileNameEx(win32proc, 0)
        name = get_app_name_win(procpath, proc.name())
        if name.endswith('.exe'):
            return name[:-4]
        else:
            return name
    else:
        return proc.name()

def get_title_and_icon_from_state(state):
    title = 'OBS Auto Record ('
    if state is ObsAutoRecordState.PAUSED:
        icon = 'pause.ico'
        title += 'paused'
    elif state is ObsAutoRecordState.CONNECTED:
        icon = 'record_green.ico'
        title += 'connected'
    elif state is ObsAutoRecordState.DISCONNECTED:
        icon = 'record_red.ico'
        title += 'disconnected'
    else:
        icon = 'warning.ico'
        title += 'unknown state'
    title += ')'
    return title, icon

def isNotEmpty(s):
    return bool(s and s.strip())
