
#include "pch.h"
#include "MainPage.xaml.h"

#include "..\NethackNative.h"

using namespace NethackUWP;

using namespace Microsoft::Graphics::Canvas::UI::Xaml;
using namespace Microsoft::Graphics::Canvas::Text;

using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;

MainPage::MainPage()
{
	InitializeComponent();

    NethackNative::start_nethack();

    window = CoreWindow::GetForCurrentThread();

    window->KeyDown += 
        ref new Windows::Foundation::TypedEventHandler<
            Windows::UI::Core::CoreWindow ^,
            Windows::UI::Core::KeyEventArgs ^>(this, &NethackUWP::MainPage::OnKeyDown);

    NethackInterop::RedrawMap += 
        ref new NethackUWP::RedrawMapEventHandler(this, &MainPage::OnRedrawMap);

    NethackInterop::InputRequest += 
        ref new NethackUWP::RequestForInputEventHandler(this, &MainPage::OnInputRequest);

    MapCanvas->PointerPressed += 
        ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &NethackUWP::MainPage::OnPointerPressed);

    MapCanvas->PointerMoved += 
        ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &NethackUWP::MainPage::OnPointerMoved);

    MapCanvas->PointerReleased += 
        ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &NethackUWP::MainPage::OnPointerReleased);

    gestureRecognizer = ref new GestureRecognizer();

    gestureRecognizer->GestureSettings = GestureSettings::Tap;

    gestureRecognizer->Tapped += ref new Windows::Foundation::TypedEventHandler<
        Windows::UI::Input::GestureRecognizer ^,
        Windows::UI::Input::TappedEventArgs ^>(this, &NethackUWP::MainPage::OnTapped);
}

void MainPage::Redraw(
    CanvasControl^ sender,
    CanvasDrawEventArgs^ args)
{
    auto tile_size = 24.0f;
    
    auto player_position = NethackNative::get_player_position();

    int start_x = player_position.x - (sender->ActualWidth / tile_size) / 2;
    int start_y = player_position.y - (sender->ActualHeight / tile_size) / 2;

    float screen_x_start = (sender->ActualWidth / 2.0f) - (player_position.x - start_x) * tile_size;
    float screen_y_start = (sender->ActualHeight / 2.0f) - (player_position.y - start_y) * tile_size;

    auto text_format = ref new CanvasTextFormat();
    text_format->FontFamily = "Consolas";
    text_format->FontSize = 18.0f;
    text_format->HorizontalAlignment = CanvasHorizontalAlignment::Center;
    text_format->VerticalAlignment = CanvasVerticalAlignment::Center;

    int y = start_y;
    for (float screenY = screen_y_start; screenY < sender->ActualHeight; screenY += tile_size)
    {
        int x = start_x;
        for (float screenX = screen_x_start; screenX < sender->ActualWidth; screenX += tile_size)
        {
            tile_t tile = NethackNative::get_tile(x, y);

            if (tile.ch != '\0' && tile.ch != ' ')
            {
                auto NetHackColorToColor = [](char color) {
                    switch (color)
                    {
                    case 1: // CLR_RED
                        return Colors::Red;
                    case 2: // CLR_GREEN
                        return Colors::Green;
                    case 3: // CLR_BROWN
                        return Colors::Brown;
                    case 4: // CLR_BLUE
                        return Colors::Blue;
                    case 5: // CLR_MAGENTA
                        return Colors::Magenta;
                    case 6: // CLR_CYAN
                        return Colors::Cyan;
                    case 7: // CLR_GRAY
                        return Colors::Gray;
                    case 9: // CLR_ORANGE
                        return Colors::Orange;
                    case 10: // CLR_BRIGHT_GREEN
                        return Colors::LightGreen;
                    case 11: // CLR_YELLOW
                        return Colors::Yellow;
                    case 12: // CLR_BRIGHT_BLUE
                        return Colors::LightBlue;
                    case 13: // CLR_BRIGHT_MAGENTA
                        return Colors::LightPink;
                    case 14: // CLR_BRIGHT_CYAN
                        return Colors::LightCyan;
                    case 15: // CLR_WHITE
                        return Colors::White;
                    }
                    return Colors::DarkGray;
                };

                std::string tile_string_ascii(1, tile.ch);
                std::wstring tile_string_wide(tile_string_ascii.begin(), tile_string_ascii.end());

                args->DrawingSession->DrawText(
                    ref new Platform::String(tile_string_wide.c_str()),
                    { screenX, screenY, tile_size, tile_size },
                    NetHackColorToColor(tile.color),
                    text_format);
            }
            x++;
        }
        y++;
    }
}

void MainPage::OnRedrawMap()
{
    window->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]()
    {
        this->MapCanvas->Invalidate();
    }));
}


void MainPage::OnInputRequest(NethackInputRequest ^inputRequest)
{
    std::unique_lock<std::mutex> lock(inputQueueLock);

    while (inputQueue.empty())
    {
        inputQueueSignal.wait(lock);
    }

    auto entry = inputQueue.front();

    inputRequest->Key = entry.Key;
    inputRequest->Modifiers = entry.Modifiers;
    inputRequest->X = entry.x;
    inputRequest->Y = entry.y;
    inputRequest->Handled = true;

    inputQueue.pop();
}

void MainPage::OnKeyDown(
    Windows::UI::Core::CoreWindow ^sender,
    Windows::UI::Core::KeyEventArgs ^args)
{
    std::unique_lock<std::mutex> lock(inputQueueLock);

    InputEntry entry;
    entry.Key = args->VirtualKey;
    inputQueue.push(entry);

    lock.unlock();
    inputQueueSignal.notify_all();
}

void NethackUWP::MainPage::OnPointerPressed(
    Platform::Object ^sender,
    Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
    gestureRecognizer->ProcessDownEvent(e->GetCurrentPoint(MapCanvas));
}

void NethackUWP::MainPage::OnPointerMoved(
    Platform::Object ^sender,
    Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
    gestureRecognizer->ProcessMoveEvents(e->GetIntermediatePoints(MapCanvas));
}

void NethackUWP::MainPage::OnPointerReleased(
    Platform::Object ^sender,
    Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e)
{
    gestureRecognizer->ProcessUpEvent(e->GetCurrentPoint(MapCanvas));
}

void NethackUWP::MainPage::OnTapped(
    Windows::UI::Input::GestureRecognizer ^sender,
    Windows::UI::Input::TappedEventArgs ^args)
{
    std::unique_lock<std::mutex> lock(inputQueueLock);

    if (args->Position.X > (MapCanvas->ActualWidth * 2.0 / 3.0))
    {
        InputEntry entry;
        entry.Key = VirtualKey::Right;
        inputQueue.push(entry);
    }
    if (args->Position.X < (MapCanvas->ActualWidth / 3.0))
    {
        InputEntry entry;
        entry.Key = VirtualKey::Left;
        inputQueue.push(entry);
    }
    if (args->Position.Y > (MapCanvas->ActualHeight * 2.0 / 3.0))
    {
        InputEntry entry;
        entry.Key = VirtualKey::Down;
        inputQueue.push(entry);
    }
    if (args->Position.Y < (MapCanvas->ActualHeight / 3.0))
    {
        InputEntry entry;
        entry.Key = VirtualKey::Up;
        inputQueue.push(entry);
    }

    lock.unlock();
    inputQueueSignal.notify_all();
}
