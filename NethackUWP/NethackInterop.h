#pragma once

struct NethackNative;

namespace NethackUWP
{
    public delegate void RedrawMapEventHandler();

    public ref class NethackInputRequest sealed
    {

    public:
        
        NethackInputRequest()
        {
            Handled = false;
        }

        property Platform::IBox<int>^ X;
        property Platform::IBox<int>^ Y;

        property Windows::System::VirtualKey Key;
        property Windows::System::VirtualKeyModifiers Modifiers;

        property Platform::IBox<bool>^ Handled;

    };

    public delegate void RequestForInputEventHandler(NethackInputRequest^ inputRequest);

    ref class NethackInterop sealed
    {

    public:

        static event RedrawMapEventHandler^ RedrawMap;
        static event RequestForInputEventHandler^ InputRequest;

    private:

        friend struct NethackNative;

        static void OnRedrawMap();
        static char OnInputRequest(int &x, int &y);

    };
}
