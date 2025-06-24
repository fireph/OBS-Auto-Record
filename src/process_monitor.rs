use sysinfo::{System, ProcessesToUpdate};
use std::collections::HashMap;

pub struct ProcessMonitor;

impl ProcessMonitor {
    /// Get a HashMap of all currently running process names
    pub fn get_running_processes() -> HashMap<String, bool> {
        let mut system = System::new();
        system.refresh_processes(ProcessesToUpdate::All, false);
        
        let mut running_processes = HashMap::new();
        for process in system.processes().values() {
            let name = process.name().to_string_lossy().to_string();
            running_processes.insert(name, true);
        }
        
        running_processes
    }
}