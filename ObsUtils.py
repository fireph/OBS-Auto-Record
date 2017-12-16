import configparser
import os
import platform

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
    try:
        import win32api
        language, codepage = win32api.GetFileVersionInfo(exe_path, '\\VarFileInfo\\Translation')[0]
        string_file_info1 = u'\\StringFileInfo\\%04X%04X\\%s' % (language, codepage, "ProductName")
        product_name = win32api.GetFileVersionInfo(exe_path, string_file_info1)
        string_file_info2 = u'\\StringFileInfo\\%04X%04X\\%s' % (language, codepage, "FileDescription")
        file_description = win32api.GetFileVersionInfo(exe_path, string_file_info2)
        if product_name is not None and file_description is not None:
            if file_description in product_name:
                app_name = file_description
            else:
                app_name = product_name
        elif product_name is not None:
            app_name = product_name
        elif file_description is not None:
            app_name = file_description
        else:
            app_name = fallback
    except:
        app_name = fallback
    return app_name

def get_app_name_from_process(proc):
    if platform.system() == 'Windows':
        import pythoncom
        import wmi
        pythoncom.CoInitialize()
        for wmi_process in wmi.WMI().Win32_Process([], ProcessId=proc.pid):
            name = get_app_name_win(wmi_process.ExecutablePath, proc.name())
            if name.endswith('.exe'):
                return name[:-4]
            else:
                return name
    else:
        return proc.name()

def assure_path_exists(path):
    if not os.path.exists(path):
        os.makedirs(path)
