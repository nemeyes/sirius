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
using FirstFloor.ModernUI.Windows.Controls;

namespace sirius.app.server.arbitrator.Settings
{
    /// <summary>
    /// Interaction logic for Configuration.xaml
    /// </summary>
    /// 

    public static class log_setting
    {
        public static LogSettings handle;
    }
    public partial class LogSettings : UserControl
    {
        public LogSettings()
        {
            log_setting.handle = this;
            InitializeComponent();

            this.Loaded += OnLoaded;
        }

        void OnLoaded(object sender, RoutedEventArgs e)
        {
            SettingValue setting_value = SettingValue.Instance();

            TextLogRootPath.Text = setting_value.log_root_path;
            
            switch (setting_value.log_level)
            {
                case 0:
                    LogLevel.Text = "TRACE";
                    break;
                case 1:
                    LogLevel.Text = "DEBUF";
                    break;
                case 2:
                    LogLevel.Text = "INFO";
                    break;
                case 3:
                    LogLevel.Text = "WARN";
                    break;
                case 4:
                    LogLevel.Text = "ERROR";
                    break;
                case 5:
                    LogLevel.Text = "FATAL";
                    break;
                case 6:
                    LogLevel.Text = "OFF";
                    break;
                default:
                    break ;
            }   
        }
        private void log_apply_button_Click(object sender, RoutedEventArgs e)
        {
            SettingValue setting_value = SettingValue.Instance();
            setting_value.log_root_path = TextLogRootPath.Text;
            setting_value.log_level = LogLevel.SelectedIndex;

            setting_value.update();
        }
    }
}
