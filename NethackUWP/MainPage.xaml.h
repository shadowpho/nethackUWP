//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <deque>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <tuple>

struct NativeMainPage;

namespace NethackUWP
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
    [Windows::UI::Xaml::Data::Bindable]
	public ref class MainPage sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged
	{
	public:
		MainPage();

        property Platform::String^ OutStringBuf {
            Platform::String^ get() {
                std::lock_guard<std::mutex> lock(blocked_on_output);
                return ref new Platform::String(output_string.c_str());
            }
        }

        property Windows::Foundation::Collections::IVector<Platform::String^>^ Inventory_Strings;

        property Windows::Foundation::Collections::IVector<Platform::String^>^ Notifications;
        property Platform::String^ Status_Line_1;
        property Platform::String^ Status_Line_2;

        property Platform::String^ Modal_Question;
        property Platform::String^ Last_Notification;
        property Windows::Foundation::Collections::IVector<Platform::String^>^ Last_Notifications;
        property Windows::Foundation::Collections::IVector<Platform::String^>^ Modal_Answers;

        virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

	private:
		void NethackUWP::MainPage::OutputBox_Tapped2(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);

		void button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Send_butt_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Quick_Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void InputBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);

        std::deque<wchar_t> input_string;
		std::deque<std::tuple<unsigned short, unsigned short> > input_mouse;
        std::mutex blocked_on_input;
        std::condition_variable input_string_cv;

        std::mutex blocked_on_output;
        std::wstring output_string;

        friend struct ::NativeMainPage;
        void OutputBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
        void Button_Open_Inventory_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void listView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
        void SymbolIcon_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
        void Button_Open_History_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void Button_Close_History_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void ExpandNotifications(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
        void CollapseNotifications(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
    
		void OnSizeChanged(Platform::Object ^sender, Windows::UI::Xaml::SizeChangedEventArgs ^e);
		void OnKeyUp(Platform::Object ^sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^e);


        void clear_map();
	};
    static MainPage^ g_mainpage;
    static Platform::Agile<Windows::UI::Core::CoreWindow> g_corewindow;
}
