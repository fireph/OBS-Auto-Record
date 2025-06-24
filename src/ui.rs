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
    let header = header_section(app);
    let obs_section = obs_section(app);
    let games_section = games_section(app);

    container(
        column![
            header,
            horizontal_rule(2),
            Space::with_height(10),
            obs_section,
            Space::with_height(20),
            horizontal_rule(1),
            Space::with_height(10),
            games_section,
        ]
        .spacing(10)
        .max_width(900)
    )
    .padding(20)
    .center_x(Length::Fill)
    .into()
}

fn header_section(app: &App) -> Element<Message> {
    row![
        text("OBS Auto Record & Stream")
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