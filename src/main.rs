#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use iced;
use app::App;

mod app;
mod config;
mod obs;
mod process_monitor;
mod ui;
mod messages;
mod game;
mod youtube;

fn main() -> iced::Result {
    // Only initialize logging in debug mode
    #[cfg(debug_assertions)]
    tracing_subscriber::fmt()
        .with_max_level(tracing::Level::DEBUG)
        .init();

    // In release mode, use minimal error-only logging
    #[cfg(not(debug_assertions))]
    tracing_subscriber::fmt()
        .with_max_level(tracing::Level::ERROR)
        .with_writer(std::io::sink) // Discard all output in release
        .init();

    iced::application("OBS Auto Record", App::update, App::view)
        .subscription(App::subscription)
        .theme(App::theme)
        .window_size(iced::Size::new(950.0, 800.0)) // Slightly wider for YouTube section
        .run_with(App::new)
}