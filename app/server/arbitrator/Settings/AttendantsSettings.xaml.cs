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

    public static class attendatns_setting
    {
        public static AttendantsSettings handle;
    }
    public partial class AttendantsSettings : UserControl
    {
        public AttendantsSettings()
        {
            attendatns_setting.handle = this;
            InitializeComponent();
                     
            this.Loaded += OnLoaded;
        }

        void OnLoaded(object sender, RoutedEventArgs e)
        {
            // select first control on the form
            Keyboard.Focus(this.TextAttendantInstanceCount);

            SettingValue setting_value = SettingValue.Instance();

            TextAttendantUrl.Text = setting_value.url;
            TextAttendantInstanceCount.Text = setting_value.max_attendant_instance.ToString();
            TextAttendantCreationDelay.Text = setting_value.attendant_creation_delay.ToString();
            SliderImageCompressionLevel.Value = setting_value.video_compression_level;
            if (setting_value.enable_present)
            {
                DisaplyAttendantOn.IsChecked = true;
                DisaplyAttendantOff.IsChecked = false;
            }
            else
            {
                DisaplyAttendantOn.IsChecked = false;
                DisaplyAttendantOff.IsChecked = true;
            }
        }
        private void attendants_set_apply_click(object sender, RoutedEventArgs e)
        {
            SettingValue setting_value = SettingValue.Instance();

            setting_value.url = TextAttendantUrl.Text;
            setting_value.max_attendant_instance = Convert.ToInt32(TextAttendantInstanceCount.Text);
            setting_value.attendant_creation_delay = Convert.ToInt32(TextAttendantCreationDelay.Text);
            setting_value.video_compression_level = (int)SliderImageCompressionLevel.Value;

            if (DisaplyAttendantOn.IsChecked.Value)
                setting_value.enable_present = true;
            else
                setting_value.enable_present = false;
            
            setting_value.update();
        }
    }
}
