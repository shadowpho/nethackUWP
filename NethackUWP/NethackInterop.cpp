
#include "pch.h"

#include "NethackInterop.h"

using namespace NethackUWP;

using namespace Windows::System;

void NethackInterop::OnRedrawMap()
{
    RedrawMap();
}

char NethackInterop::OnInputRequest(int &x, int &y)
{
    char key_value = 0;
    
    while (key_value == 0)
    {
        NethackInputRequest^ request = ref new NethackInputRequest();

        InputRequest(request);

        if (request->Handled)
        {
            x = request->X->Value;
            y = request->Y->Value;

            if ((request->Key == VirtualKey::Left) ||
                (request->Key == VirtualKey::GamepadLeftThumbstickLeft) ||
                (request->Key == VirtualKey::GamepadDPadLeft))
            {
                key_value = 'h';
            }
            else if ((request->Key == VirtualKey::Up) ||
                     (request->Key == VirtualKey::GamepadLeftThumbstickUp) ||
                     (request->Key == VirtualKey::GamepadDPadUp))
            {
                key_value = 'k';
            }
            else if ((request->Key == VirtualKey::Right) ||
                     (request->Key == VirtualKey::GamepadLeftThumbstickRight) ||
                     (request->Key == VirtualKey::GamepadDPadRight))
            {
                key_value = 'l';
            }
            else if ((request->Key == VirtualKey::Down) ||
                     (request->Key == VirtualKey::GamepadLeftThumbstickDown) ||
                     (request->Key == VirtualKey::GamepadDPadDown))
            {
                key_value = 'j';
            }
        }
    }

    return key_value;
}
