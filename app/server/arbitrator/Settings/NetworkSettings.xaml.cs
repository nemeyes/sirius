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

            TextControllerPortnumber.Text = setting_value.portnumber.ToString();
            UseTLS.IsChecked = setting_value.enable_tls;
                 

        }
        private void network_set_apply_click(object sender, RoutedEventArgs e)
        {
            SettingValue setting_value = SettingValue.Instance();

            setting_value.portnumber = Convert.ToInt32(TextControllerPortnumber.Text);

            if (UseTLS.IsChecked.Value)
                setting_value.enable_tls = true;
            else
                setting_value.enable_tls = false;
            
            setting_value.update();
        }
    }
}
