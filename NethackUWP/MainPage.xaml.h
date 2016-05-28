#pragma once

#include "MainPage.g.h"
#include "NethackInterop.h"

#include <queue>

namespace NethackUWP
{
	public ref class MainPage sealed
	{

	public:

		MainPage();

    private:

        void OnKeyDown(
            Windows::UI::Core::CoreWindow ^sender,
            Windows::UI::Core::KeyEventArgs ^args);

        void Redraw(
            Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl^ sender,
            Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs^ args);

        void OnRedrawMap();
        void OnInputRequest(NethackInputRequest ^inputRequest);

        Windows::UI::Core::CoreWindow^ window;

        struct InputEntry {
            int x, y;
            Windows::System::VirtualKey Key;
            Windows::System::VirtualKeyModifiers Modifiers;
        };

        std::mutex inputQueueLock;
        std::condition_variable inputQueueSignal;

        std::queue<InputEntry> inputQueue;
    };
}
