use iced::{
    widget::{
        button, column, container, row, scrollable, text, text_input, Space,
        horizontal_rule
    },
    Element, Length,
};

use crate::app::App;
use crate::messages::Message;
use crate::game::GameMode;

pub fn view(app: &App) -> Element<Message> {
    let content = column![
        header_section(app),
        horizontal_rule(2),
        Space::with_height(10),
        obs_section(app),
        Space::with_height(10),
        youtube_section(app),
        Space::with_height(20),
        horizontal_rule(1),
        Space::with_height(10),
        games_section(app),
    ]
    .spacing(10)
    .max_width(900);

    let main_content = container(content)
        .padding(20)
        .center_x(Length::Fill);

    // Show client secret dialog if needed
    if app.show_client_secret_dialog() {
        column![
            client_secret_dialog(app),
            Space::with_height(20),
            main_content,
        ].into()
    } else {
        main_content.into()
    }
}

fn header_section(app: &App) -> Element<Message> {
    row![
        text("OBS Auto Record")
            .size(24)
            .width(Length::Fill),
        button(if app.dark_mode() { "Light" } else { "Dark" })
            .on_press(Message::ToggleDarkMode)
            .padding(8),
    ]
    .align_y(iced::Alignment::Center)
    .spacing(10)
    .into()
}

fn obs_section(app: &App) -> Element<Message> {
    column![
        text("OBS WebSocket Settings").size(18),
        row![
            text_input("ws://localhost:4455", app.obs_url())
                .on_input(Message::UpdateObsUrl)
                .width(Length::Fill),
            button("Test")
                .on_press(Message::TestObsConnection)
                .padding([5, 10]),
        ]
        .spacing(10),
        text(app.obs_connection_status())
            .color(if app.obs_connection_status().starts_with("Connected") {
                iced::Color::from_rgb(0.0, 0.8, 0.0)
            } else if app.obs_connection_status().starts_with("Error") {
                iced::Color::from_rgb(0.8, 0.0, 0.0)
            } else {
                iced::Color::BLACK
            }),
    ]
    .spacing(10)
    .into()
}

fn youtube_section(app: &App) -> Element<Message> {
    let status_text = if app.youtube_auth().authenticated {
        if let Some(channel_name) = &app.youtube_auth().channel_name {
            format!("✅ Connected as: {}", channel_name)
        } else {
            "✅ Connected to YouTube".to_string()
        }
    } else {
        "Not connected to YouTube".to_string()
    };

    let status_color = if app.youtube_auth().authenticated {
        iced::Color::from_rgb(0.0, 0.8, 0.0)
    } else {
        iced::Color::from_rgb(0.6, 0.6, 0.6)
    };

    let action_button = if app.youtube_auth().authenticated {
        button("Disconnect YouTube")
            .on_press(Message::DisconnectYouTube)
            .padding([5, 10])
    } else {
        button("Connect YouTube")
            .on_press(Message::AuthenticateYouTube)
            .padding([5, 10])
    };

    column![
        row![
            text("YouTube Integration").size(18),
            Space::with_width(Length::Fill),
            if !app.youtube_auth().authenticated {
                button("Setup")
                    .on_press(Message::ShowClientSecretDialog)
                    .padding([3, 8])
            } else {
                // Empty button when authenticated (invisible)
                button("")
                    .padding([3, 8])
                    .style(|_, _| button::Style {
                        background: None,
                        text_color: iced::Color::TRANSPARENT,
                        ..Default::default()
                    })
            }
        ]
        .align_y(iced::Alignment::Center),
        
        row![
            text(status_text)
                .color(status_color)
                .width(Length::Fill),
            action_button,
        ]
        .spacing(10)
        .align_y(iced::Alignment::Center),
        
        if app.youtube_integration_enabled() {
            text("Auto-create YouTube live streams when games start streaming")
                .size(12)
                .color(iced::Color::from_rgb(0.5, 0.5, 0.5))
        } else {
            text("Connect to automatically create YouTube live streams")
                .size(12)
                .color(iced::Color::from_rgb(0.5, 0.5, 0.5))
        }
    ]
    .spacing(10)
    .into()
}

fn client_secret_dialog(app: &App) -> Element<Message> {
    let instructions = crate::youtube::get_client_secret_setup_instructions();
    
    container(
        column![
            text("Setup YouTube Integration").size(20),
            
            Space::with_height(10),
            
            scrollable(
                text(instructions)
                    .size(12)
            )
            .height(150),
            
            Space::with_height(10),
            
            text("Paste your client secret JSON here:")
                .size(14),
            
            text_input(
                r#"{"installed":{"client_id":"...","client_secret":"..."}}"#,
                app.client_secret_input()
            )
            .on_input(Message::ClientSecretInput)
            .width(Length::Fill),
            
            Space::with_height(15),
            
            row![
                button("Cancel")
                    .on_press(Message::HideClientSecretDialog)
                    .padding([8, 16]),
                Space::with_width(Length::Fill),
                button("Save & Authenticate")
                    .on_press(Message::SetClientSecret)
                    .padding([8, 16]),
            ]
            .spacing(10)
            .align_y(iced::Alignment::Center),
        ]
        .spacing(10)
        .width(Length::Fill)
        .max_width(700)
    )
    .padding(20)
    .style(|theme: &iced::Theme| {
        container::Style {
            background: Some(iced::Background::Color(theme.palette().background)),
            border: iced::Border {
                color: theme.palette().primary,
                width: 2.0,
                radius: 8.0.into(),
            },
            ..Default::default()
        }
    })
    .into()
}

fn games_section(app: &App) -> Element<Message> {
    let games_header = row![
        text("Games to Monitor").size(18),
        Space::with_width(Length::Fill),
        button("Add Game")
            .on_press(Message::AddGame)
            .padding([5, 10]),
    ]
    .align_y(iced::Alignment::Center);

    let games_list: Element<Message> = if app.games().is_empty() {
        container(
            text("No games added yet. Click 'Add Game' to get started!")
                .color(iced::Color::from_rgb(0.5, 0.5, 0.5))
        )
        .padding(20)
        .center_x(Length::Fill)
        .into()
    } else {
        scrollable(
            column(
                app.games()
                    .iter()
                    .enumerate()
                    .map(|(i, game)| game_item(i, game))
                    .collect::<Vec<_>>()
            )
            .spacing(10)
        )
        .height(Length::Fill)
        .into()
    };

    column![games_header, Space::with_height(10), games_list]
        .spacing(10)
        .into()
}

fn game_item(index: usize, game: &crate::game::GameEntry) -> Element<Message> {
    let mode_text = match game.mode {
        GameMode::Recording => "Recording",
        GameMode::Streaming => "Streaming",
    };

    container(
        row![
            column![
                text_input("Game name", &game.name)
                    .on_input(move |name| Message::EditGameName(index, name))
                    .width(Length::Fill),
                text(format!("Path: {}", game.path.display()))
                    .size(12)
                    .color(iced::Color::from_rgb(0.6, 0.6, 0.6)),
            ]
            .width(Length::Fill)
            .spacing(5),
            column![
                row![
                    text("Mode:")
                        .size(12)
                        .color(iced::Color::from_rgb(0.6, 0.6, 0.6)),
                    button(mode_text)
                        .on_press(Message::ToggleGameMode(index))
                        .padding([2, 6]),
                ]
                .spacing(5)
                .align_y(iced::Alignment::Center),
                text(game.status_text())
                    .size(14)
                    .color(game.status_color()),
                button("Remove")
                    .on_press(Message::RemoveGame(index))
                    .padding([3, 8]),
            ]
            .align_x(iced::Alignment::End)
            .spacing(5),
        ]
        .align_y(iced::Alignment::Center)
        .spacing(15)
    )
    .padding(15)
    .into()
}