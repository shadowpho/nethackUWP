
#include "pch.h"
#include "MainPage.xaml.h"

#include "..\NethackNative.h"

using namespace NethackUWP;

using namespace Microsoft::Graphics::Canvas::UI::Xaml;
using namespace Microsoft::Graphics::Canvas::Text;

using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::System;

MainPage::MainPage()
{
	InitializeComponent();

    NethackNative::start_nethack();
}

void NethackUWP::MainPage::CanvasControl_Draw(
    CanvasControl^ sender,
    CanvasDrawEventArgs^ args)
{
    auto tile_size = 20.0f * (args->DrawingSession->Dpi / 96.0f);

    auto text_format = ref new CanvasTextFormat();
    text_format->FontFamily = "Consolas";
    text_format->FontSize = 14.0f;
    text_format->HorizontalAlignment = CanvasHorizontalAlignment::Center;
    text_format->VerticalAlignment = CanvasVerticalAlignment::Center;
    
    auto player_position = NethackNative::get_player_position();

    int width_in_tiles = sender->ActualWidth / tile_size;
    int height_in_tiles = sender->ActualHeight / tile_size;

    int start_x = player_position.x - width_in_tiles / 2;
    int start_y = player_position.y - height_in_tiles / 2;

    int y = start_y;
    for (float screenY = 0.0f; screenY < sender->ActualHeight; screenY += tile_size)
    {
        int x = start_x;
        for (float screenX = 0.0f; screenX < sender->ActualWidth; screenX += tile_size)
        {
            tile_t tile = NethackNative::get_tile(x, y);

            if (tile.ch != '\0' && tile.ch != ' ')
            {
                std::string tile_string_ascii(1, tile.ch);
                std::wstring tile_string_wide(tile_string_ascii.begin(), tile_string_ascii.end());

                auto color = Colors::DarkGray;

                switch (tile.color)
                {
                    case 1: // CLR_RED
                        color = Colors::Red;
                        break;
                    case 2: // CLR_GREEN
                        color = Colors::Green;
                        break;
                    case 3: // CLR_BROWN
                        color = Colors::Brown;
                        break;
                    case 4: // CLR_BLUE
                        color = Colors::Blue;
                        break;
                    case 5: // CLR_MAGENTA
                        color = Colors::Magenta;
                        break;
                    case 6: // CLR_CYAN
                        color = Colors::Cyan;
                        break;
                    case 7: // CLR_GRAY
                        color = Colors::Gray;
                        break;
                    case 9: // CLR_ORANGE
                        color = Colors::Orange;
                        break;
                    case 10: // CLR_BRIGHT_GREEN
                        color = Colors::LightGreen;
                        break;
                    case 11: // CLR_YELLOW
                        color = Colors::Yellow;
                        break;
                    case 12: // CLR_BRIGHT_BLUE
                        color = Colors::LightBlue;
                        break;
                    case 13: // CLR_BRIGHT_MAGENTA
                        color = Colors::LightPink;
                        break;
                    case 14: // CLR_BRIGHT_CYAN
                        color = Colors::LightCyan;
                        break;
                    case 15: // CLR_WHITE
                        color = Colors::White;
                        break;
                }

                args->DrawingSession->DrawText(
                    ref new Platform::String(tile_string_wide.c_str()),
                    { screenX, screenY, tile_size, tile_size },
                    color,
                    text_format);
            }

            x++;
        }
        y++;
    }
}
