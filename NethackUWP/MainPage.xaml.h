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

struct NativeMainPage {
    static constexpr int max_width = 100;
    static constexpr int max_width_offset = max_width + 1;
    static constexpr int max_height = 80;
    static int read_char();
    static void write_char(int x, int y, char ch);
 
    static void write_notification(const char*);
};

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

        property Windows::Foundation::Collections::IVector<Platform::String^>^ Notifications;

        virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

	private:
		void button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Send_butt_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void InputBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);

        std::deque<wchar_t> input_string;
        std::mutex blocked_on_input;
        std::condition_variable input_string_cv;

        std::mutex blocked_on_output;
        std::wstring output_string;

        friend struct ::NativeMainPage;
        void OutputBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
    };
    static MainPage^ g_mainpage;
    static Platform::Agile<Windows::UI::Core::CoreWindow> g_corewindow;
}
