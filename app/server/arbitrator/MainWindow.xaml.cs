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
        public static class proxy
        {
            public static sirius_arbitrator_proxy_wrapper.handler core;
        }
        public MainWindow()
        {
            InitializeComponent();

            //AppearanceManager.Current.ThemeSource = AppearanceManager.DarkThemeSource;
            AppearanceManager.Current.AccentColor = Color.FromRgb(0xf0, 0x96, 0x09);

            proxy.core = new sirius_arbitrator_proxy_wrapper.handler();
            unsafe
            {
                on_initalize_callback = new delegate_initialize_callback(on_initalize);
                proxy.core.set_initialize_callback(on_initalize_callback);

                on_system_monitor_info_callback = new delegate_system_monitor_info_callback(on_system_monitor_info);
                proxy.core.set_system_monitor_info_callback(on_system_monitor_info_callback);

                on_attendant_create_callback = new delegate_attendant_create_callback(on_attendant_create);
                proxy.core.set_attendant_create_callback(on_attendant_create_callback);

                on_start_callback = new delegate_start_callback(on_start);
                proxy.core.set_start_callback(on_start_callback);

                on_stop_callback = new delegate_stop_callback(on_stop);
                proxy.core.set_stop_callback(on_stop_callback);

                on_release_callback = new delegate_release_callback(on_release);
                proxy.core.set_release_callback(on_release_callback);
            }
            proxy.core.initailize();             

        }               
        public unsafe void on_initalize(sbyte* uuid, sbyte* url, int max_attendant_instance, int attendant_creation_delay, int portnumber, int video_codec, int video_width, int video_height, int video_fps, int video_block_width, int video_block_height, int video_compression_level, int video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc, sbyte* cpu, sbyte* memory, sbyte** gpu, int gpu_cnt)
        {

        }
        public unsafe void on_system_monitor_info(double cpu_usage, double memory_usage)
        {
            if(Status.status_page.handle != null)
                Status.status_page.handle.update_usage(cpu_usage, memory_usage);   
        }     
        public unsafe void on_attendant_create(double percent)
        {           
            Splash.progress_bar.handle.update_progress_bar(percent);

            if (percent >= 100)
                Status.status_page.handle.on_attendant_load();
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
