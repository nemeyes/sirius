using FirstFloor.ModernUI.Windows.Controls;
using FirstFloor.ModernUI.Presentation;
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
using System.Windows.Shapes;
using System.Runtime.InteropServices;
using sirius.app.server.arbitrator.Control;
using sirius.app.server.arbitrator.Settings;

namespace sirius.app.server.arbitrator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : ModernWindow
    {
        delegate_initialize_callback on_initalize_callback;
        delegate_system_monitor_info_callback on_system_monitor_info_callback;
        delegate_attendant_create_callback on_attendant_create_callback;
        delegate_start_callback on_start_callback;
        delegate_stop_callback on_stop_callback;
        delegate_release_callback on_release_callback;
        public static class arbitrator_proxy
        {
            public static sirius.app.server.arbitrator.wrapper.handler controller = null;
        }
        public MainWindow()
        {
            InitializeComponent();

            //AppearanceManager.Current.ThemeSource = AppearanceManager.DarkThemeSource;
            AppearanceManager.Current.AccentColor = Color.FromRgb(0xf0, 0x96, 0x09);

            arbitrator_proxy.controller = new sirius.app.server.arbitrator.wrapper.handler();
            unsafe
            {
                on_initalize_callback = new delegate_initialize_callback(on_initalize);
                arbitrator_proxy.controller.set_initialize_callback(on_initalize_callback);

                on_system_monitor_info_callback = new delegate_system_monitor_info_callback(on_system_monitor_info);
                arbitrator_proxy.controller.set_system_monitor_info_callback(on_system_monitor_info_callback);

                on_attendant_create_callback = new delegate_attendant_create_callback(on_attendant_create);
                arbitrator_proxy.controller.set_attendant_create_callback(on_attendant_create_callback);

                on_start_callback = new delegate_start_callback(on_start);
                arbitrator_proxy.controller.set_start_callback(on_start_callback);

                on_stop_callback = new delegate_stop_callback(on_stop);
                arbitrator_proxy.controller.set_stop_callback(on_stop_callback);

                on_release_callback = new delegate_release_callback(on_release);
                arbitrator_proxy.controller.set_release_callback(on_release_callback);
            }
            arbitrator_proxy.controller.initailize();             

        }               
        public unsafe void on_initalize(sbyte * uuid, sbyte * url, int max_attendant_instance, int attendant_creation_delay, int portnumber, int video_codec, int video_width, int video_height, int video_fps, int video_block_width, int video_block_height, int video_compression_level, int video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc, sbyte * cpu, sbyte * memory)
        {
            SettingValue.Instance().uuid = new string(uuid);
            SettingValue.Instance().url = new string(url);
            SettingValue.Instance().max_attendant_instance = max_attendant_instance;
            SettingValue.Instance().attendant_creation_delay = attendant_creation_delay;
            SettingValue.Instance().portnumber = portnumber;
            SettingValue.Instance().video_codec = video_codec;
            SettingValue.Instance().video_width = video_width;
            SettingValue.Instance().video_height = video_height;
            SettingValue.Instance().video_fps = video_fps;
            SettingValue.Instance().video_block_width = video_block_width;
            SettingValue.Instance().video_block_height = video_block_height;
            SettingValue.Instance().video_compression_level = video_compression_level;
            SettingValue.Instance().video_quantization_colors = video_quantization_colors;
            SettingValue.Instance().enable_tls = enable_tls;
            SettingValue.Instance().enable_gpu = enable_gpu;
            SettingValue.Instance().enable_present = enable_present;
            SettingValue.Instance().enable_auto_start = enable_auto_start;
            SettingValue.Instance().enable_quantization = enable_quantization;
            SettingValue.Instance().enable_caching = enable_caching;
            SettingValue.Instance().enable_crc = enable_crc;

            if (enable_auto_start)
                arbitrator_proxy.controller.start();
        }
        public unsafe void on_system_monitor_info(double cpu_usage, double memory_usage)
        {
            if(Status.status_page.handle != null)
                Status.status_page.handle.update_usage(cpu_usage, memory_usage);   
        }     
        public unsafe void on_attendant_create(double percent)
        {
            if(Splash.progress_bar.handle != null)
                Splash.progress_bar.handle.update_progress_bar(percent);

            if (percent >= 100)
                Status.status_page.handle.completed_attendant_load();
        }
        public unsafe void on_start()
        {
                
        }
        public unsafe void on_stop()
        {

        }
        public unsafe void on_release()
        {

        }

    }
}
