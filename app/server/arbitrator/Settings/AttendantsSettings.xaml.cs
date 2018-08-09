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
            TextFrameRate.Text = setting_value.video_fps.ToString();
            TextVideoBlockWidth.Text = setting_value.video_block_width.ToString();
            TextVideoBlockHeight.Text = setting_value.video_block_height.ToString();
            TextThreadCount.Text = setting_value.nthread.ToString();
            TextAppSessionApp.Text = setting_value.app_session_app.ToString();
            QuantizationColors.Text = setting_value.video_quantization_colors.ToString();
            //SliderImageCompressionLevel.Value = setting_value.video_compression_level;
            setting_value.video_compression_level = 1;
            setting_value.video_quantization_colors = Convert.ToInt32(QuantizationColors.Text);
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

            if (setting_value.indexed_mode)
            {
                IndexedModeOn.IsChecked = true;
                IndexedModeOff.IsChecked = false;
            }
            else
            {
                IndexedModeOn.IsChecked = false;
                IndexedModeOff.IsChecked = true;
            }

            if (setting_value.clean_attendant)
            {
                CleanAttendantOn.IsChecked = true;
                CleanAttendantOff.IsChecked = false;
            }
            else
            {
                CleanAttendantOn.IsChecked = false;
                CleanAttendantOff.IsChecked = true;
            }


            setting_value.enable_auto_start = false;

            if (sirius_arbitrator.handle.get_status() == sirius_arbitrator.status_t.started || 
                sirius_arbitrator.handle.get_status() == sirius_arbitrator.status_t.starting) 
                attendants_apply_button.IsEnabled = false;
            else
                attendants_apply_button.IsEnabled = true;

            //if (setting_value.enable_auto_start)
            //{
            //    AutostartOn.IsChecked = true;
            //    AutostartOff.IsChecked = false;
            //}
            //else
            //{
            //    AutostartOn.IsChecked = false;
            //    AutostartOff.IsChecked = true;
            //}
        }
        private void attendants_set_apply_click(object sender, RoutedEventArgs e)
        {
            Uri uriResult;
            if (!Uri.TryCreate(TextAttendantUrl.Text, UriKind.Absolute, out uriResult))
            {
                var v = new ModernDialog
                {
                    Title = "Error",
                    Content = "CODE: INVALID URL \n\n [SETTINGS] -> [ATTENDANT] -> [URL]"
                };
                v.ShowDialog();
                return;
            }

            if (uriResult.Scheme == Uri.UriSchemeFile)
            {
                System.IO.FileInfo fi = new System.IO.FileInfo(TextAttendantUrl.Text);
                if (!fi.Exists)
                {
                    var v = new ModernDialog
                    {
                        Title = "Error",
                        Content = "CODE: INVALID URL \n\n [SETTINGS] -> [ATTENDANT] -> [URL]"
                    };
                    v.ShowDialog();
                    return;
                }
            }       

            SettingValue setting_value = SettingValue.Instance();
            setting_value.url = TextAttendantUrl.Text.Trim();
            setting_value.max_attendant_instance = Convert.ToInt32(TextAttendantInstanceCount.Text);
            setting_value.attendant_creation_delay = Convert.ToInt32(TextAttendantCreationDelay.Text);
            //setting_value.video_compression_level = (int)SliderImageCompressionLevel.Value;
            setting_value.video_compression_level = 1;       
            setting_value.app_session_app = TextAppSessionApp.Text.Trim();

            setting_value.video_quantization_colors = Convert.ToInt32(QuantizationColors.Text);
            setting_value.video_fps = Convert.ToInt32(TextFrameRate.Text);
            setting_value.video_block_width = Convert.ToInt32(TextVideoBlockWidth.Text);
            setting_value.video_block_height = Convert.ToInt32(TextVideoBlockHeight.Text);
            setting_value.nthread = Convert.ToInt32(TextThreadCount.Text);

            if (DisaplyAttendantOn.IsChecked.Value)
                setting_value.enable_present = true;
            else
                setting_value.enable_present = false;

            if (IndexedModeOn.IsChecked.Value)
                setting_value.indexed_mode = true;
            else
                setting_value.indexed_mode = false;

            if (CleanAttendantOn.IsChecked.Value)
                setting_value.clean_attendant = true;
            else
                setting_value.clean_attendant = false;

            setting_value.enable_auto_start = false;
            //if (AutostartOn.IsChecked.Value)
            //    setting_value.enable_auto_start = true;
            //else
            //    setting_value.enable_auto_start = false;

            setting_value.update();
            sirius_arbitrator.controller.release();
            sirius_arbitrator.controller.initailize();
        }
        private void TextAttendantInstanceCount_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (TextAttendantInstanceCount.Text.Length > 0)
            {
                if (Convert.ToInt32(TextAttendantInstanceCount.Text) > 500)
                {
                    TextAttendantInstanceCount.Text = "500";
                }
            }
        }
        private void TextAttendantInstanceCount_PreviewTextInput(object sender, TextCompositionEventArgs e)
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
        private void TextAttendantCreationDelay_PreviewTextInput(object sender, TextCompositionEventArgs e)
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
        private void TextFraemRate_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            foreach (char c in e.Text)
            {
                if (!char.IsDigit(c))
                {
                    e.Handled = true;
                    break;
                }
            }
        }
        private void TextFrameRate_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (TextFrameRate.Text.Length > 0)
            {
                if (Convert.ToInt32(TextFrameRate.Text) > 30)
                {
                    TextFrameRate.Text = "30";
                }
            }
        }

        private void TextVideoBlockWidth_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            foreach (char c in e.Text)
            {
                if (!char.IsDigit(c))
                {
                    e.Handled = true;
                    break;
                }
            }
        }

        private void TextVideoBlockHeight_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            foreach (char c in e.Text)
            {
                if (!char.IsDigit(c))
                {
                    e.Handled = true;
                    break;
                }
            }
        }

        private void TextThreadCount_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
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
