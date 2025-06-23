use sysinfo::{System, ProcessesToUpdate};
use std::collections::HashMap;

#[derive(Debug)]
pub struct ProcessMonitor {
    system: System,
}

impl ProcessMonitor {
    pub fn new() -> Self {
        Self {
            system: System::new_all(),
        }
    }

    pub fn get_running_processes(&mut self) -> HashMap<String, bool> {
        self.system.refresh_processes(ProcessesToUpdate::All, true);
        
        let mut running_processes = HashMap::new();
        
        for process in self.system.processes().values() {
            let name = process.name().to_string_lossy().to_string();
            running_processes.insert(name, true);
        }
        
        running_processes
    }

    pub fn is_process_running(&mut self, exe_name: &str) -> bool {
        self.system.refresh_processes(ProcessesToUpdate::All, true);
        
        self.system
            .processes()
            .values()
            .any(|process| process.name().to_string_lossy().eq_ignore_ascii_case(exe_name))
    }
}