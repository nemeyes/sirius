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
            UseKeepalive.IsChecked = setting_value.enable_keepalive;
            TextKeepaliveTimeout.Text = setting_value.keepalive_timeout.ToString();
            UseStreamerKeepalive.IsChecked = setting_value.enable_streamer_keepalive;
            TextStreamerKeepaliveTimeout.Text = setting_value.streamer_keepalive_timeout.ToString();

            if (sirius_arbitrator.handle.get_status() == sirius_arbitrator.status_t.started ||
                sirius_arbitrator.handle.get_status() == sirius_arbitrator.status_t.starting)
                network_apply_button.IsEnabled = false;
            else if (sirius_arbitrator.handle.get_status() == sirius_arbitrator.status_t.stopped)
                network_apply_button.IsEnabled = true;

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

            if (UseKeepalive.IsChecked.Value)
                setting_value.enable_keepalive = true;
            else
                setting_value.enable_keepalive = false;
                       
            if (Convert.ToInt32(TextKeepaliveTimeout.Text) < 5000)
                TextKeepaliveTimeout.Text = "5000";

            setting_value.keepalive_timeout = Convert.ToInt32(TextKeepaliveTimeout.Text);
            
            if (UseStreamerKeepalive.IsChecked.Value)
                setting_value.enable_streamer_keepalive = true;
            else
                setting_value.enable_streamer_keepalive = false;

            if (Convert.ToInt32(TextStreamerKeepaliveTimeout.Text) < 5000)
                TextStreamerKeepaliveTimeout.Text = "5000";

            setting_value.streamer_keepalive_timeout = Convert.ToInt32(TextStreamerKeepaliveTimeout.Text);

            setting_value.update();
            System.Windows.Forms.Application.Restart();
            Environment.Exit(0);
            //sirius_arbitrator.controller.release();
            //sirius_arbitrator.controller.initailize();
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

        private void TextKeepaliveTimeout_PreviewTextInput(object sender, TextCompositionEventArgs e)
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

        private void TextStreamerKeepaliveTimeout_PreviewTextInput(object sender, TextCompositionEventArgs e)
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
