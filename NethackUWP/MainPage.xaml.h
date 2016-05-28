#pragma once

#include "MainPage.g.h"

namespace NethackUWP
{
	public ref class MainPage sealed
	{

	public:
		MainPage();

    private:
        void CanvasControl_Draw(
            Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl^ sender,
            Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs^ args);

        void OnAcceleratorKeyActivated(
            Windows::UI::Core::CoreDispatcher ^sender,
            Windows::UI::Core::AcceleratorKeyEventArgs ^args);
    };
}
