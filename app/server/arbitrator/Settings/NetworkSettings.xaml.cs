using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace sirius.app.server.arbitrator.Settings
{
    /// <summary>
    /// Interaction logic for Configuration.xaml
    /// </summary>
    /// 
    public static class network_setting
    {
        public static NetworkSettings handle;
    }
    public partial class NetworkSettings : UserControl
    {
        public NetworkSettings()
        {
            network_setting.handle = this;
            InitializeComponent();

            this.Loaded += OnLoaded;
        }

        void OnLoaded(object sender, RoutedEventArgs e)
        {
            // select first control on the form
            Keyboard.Focus(this.TextControllerPortnumber);

            SettingValue setting_value = SettingValue.Instance();

            TextControllerPortnumber.Text = setting_value.controller_portnumber.ToString();
            TextStreamerPortnumber.Text = setting_value.streamer_portnumber.ToString();
            UseTLS.IsChecked = setting_value.enable_tls;
            UseKeepAlive.IsChecked = setting_value.enable_keepalive;
        }
        private void network_set_apply_click(object sender, RoutedEventArgs e)
        {
            SettingValue setting_value = SettingValue.Instance();

            setting_value.controller_portnumber = Convert.ToInt32(TextControllerPortnumber.Text);
            setting_value.streamer_portnumber = Convert.ToInt32(TextStreamerPortnumber.Text);

            if (UseTLS.IsChecked.Value)
                setting_value.enable_tls = true;
            else
                setting_value.enable_tls = false;

            if (UseKeepAlive.IsChecked.Value)
                setting_value.enable_keepalive = true;
            else
                setting_value.enable_keepalive = false;
            
            setting_value.update();
        }

        private void TextControllerPortnumber_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            //only number type
            foreach (char c in e.Text)
            {
                if (!char.IsDigit(c))
                {
                    e.Handled = true;
                    break;
                }
            }
        }

        private void TextStreamerPortnumber_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            //only number type
            foreach (char c in e.Text)
            {
                if (!char.IsDigit(c))
                {
                    e.Handled = true;
                    break;
                }
            }
        }
    }
}
