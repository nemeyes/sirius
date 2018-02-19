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
using System.Runtime.InteropServices;
using System.Windows.Threading;
using sirius.app.server.arbitrator.Control;
using sirius.app.server.arbitrator.Settings;

namespace sirius.app.server.arbitrator
{
    /// <summary>
    /// sirius_arbitrator.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class sirius_arbitrator : UserControl
    {
        public static sirius_arbitrator handle;
        public struct status_t
        {
            public const int initialized = 0;
            public const int starting = 1;
            public const int started = 2;
            public const int stopped = 3;
            public const int released = 4;
        };

        delegate_initialize_callback on_initalize_callback;
        delegate_system_monitor_info_callback on_system_monitor_info_callback;
        delegate_attendant_create_callback on_attendant_create_callback;
        delegate_start_callback on_start_callback;
        delegate_stop_callback on_stop_callback;
        delegate_release_callback on_release_callback;

        public static MainWindow front;
        public static sirius.app.server.arbitrator.wrapper.handler controller;
        public static int status = status_t.released;
                
        public sirius_arbitrator(MainWindow wnd)
        {
            sirius_arbitrator.handle = this;
            InitializeComponent();
            front = wnd;
            controller = new sirius.app.server.arbitrator.wrapper.handler();
            unsafe
            {
                on_initalize_callback = new delegate_initialize_callback(on_initalize);
                controller.set_initialize_callback(on_initalize_callback);

                on_system_monitor_info_callback = new delegate_system_monitor_info_callback(on_system_monitor_info);
                controller.set_system_monitor_info_callback(on_system_monitor_info_callback);

                on_attendant_create_callback = new delegate_attendant_create_callback(on_attendant_create);
                controller.set_attendant_create_callback(on_attendant_create_callback);

                on_start_callback = new delegate_start_callback(on_start);
                controller.set_start_callback(on_start_callback);

                on_stop_callback = new delegate_stop_callback(on_stop);
                controller.set_stop_callback(on_stop_callback);

                on_release_callback = new delegate_release_callback(on_release);
                controller.set_release_callback(on_release_callback);
            }
            controller.initailize();
            status = status_t.initialized;
        }
        public unsafe void on_initalize(sbyte* uuid, sbyte* url, int max_attendant_instance, int attendant_creation_delay, int controller_portnumber, int streamer_portnumber, int video_codec, int video_width, int video_height, int video_fps, int video_block_width, int video_block_height, int video_compression_level, int video_quantization_colors, bool enable_tls, bool enable_keepalive, bool enable_present, bool enable_auto_start, bool enable_caching, sbyte* cpu, sbyte* memory)
        {
            SettingValue.Instance().uuid = new string(uuid);
            SettingValue.Instance().url = new string(url);
            SettingValue.Instance().max_attendant_instance = max_attendant_instance;
            SettingValue.Instance().attendant_creation_delay = attendant_creation_delay;
            SettingValue.Instance().controller_portnumber = controller_portnumber;
            SettingValue.Instance().streamer_portnumber = streamer_portnumber;
            SettingValue.Instance().video_codec = video_codec;
            SettingValue.Instance().video_width = video_width;
            SettingValue.Instance().video_height = video_height;
            SettingValue.Instance().video_fps = video_fps;
            SettingValue.Instance().video_block_width = video_block_width;
            SettingValue.Instance().video_block_height = video_block_height;
            SettingValue.Instance().video_compression_level = video_compression_level;
            SettingValue.Instance().video_quantization_colors = video_quantization_colors;
            SettingValue.Instance().enable_tls = enable_tls;
            SettingValue.Instance().enable_keepalive = enable_keepalive;
            SettingValue.Instance().enable_present = enable_present;
            SettingValue.Instance().enable_auto_start = enable_auto_start;
            SettingValue.Instance().enable_caching = enable_caching;
            SettingValue.Instance().cpu = new string(cpu);
            SettingValue.Instance().memory = new string(memory);

            //if (enable_auto_start)
            //    controller.start();
        }
        public unsafe void on_system_monitor_info(double cpu_usage, double memory_usage)
        {
            if (Status.handle != null)
            {
                Status.handle.update_usage(cpu_usage, memory_usage);

                if (status == status_t.started)
                {
                    int max_attendant_instance = SettingValue.Instance().max_attendant_instance;
                    int connect_count = max_attendant_instance - controller.get_available_attendant_count();
                    Status.handle.update_connect_count(max_attendant_instance, connect_count);
                }
                else
                {
                    Status.handle.update_connect_count(0, 0);
                }
            }
        }
        public unsafe void on_attendant_create(double percent)
        {
            if (Splash.handle != null)
            {
                Splash.handle.update_progress_bar(percent);

                if (percent >= 100)
                {
                    System.Threading.Thread.Sleep(200);
                    Dispatcher.Invoke(DispatcherPriority.Normal,
                    new Action
                    (
                        delegate ()
                        {
                            popup_progressbar.IsOpen = false;
                            Status.handle.stop_button.IsEnabled = true;
                            front.IsEnabled = true;                           
                        }
                    ));

                    status = status_t.started;
                }
            }
        }
        public unsafe void on_start()
        {
            Dispatcher.Invoke(DispatcherPriority.Normal,
            new Action
            (
                delegate ()
                {
                    Splash.handle.update_progress_bar(0);
                    popup_progressbar.PlacementTarget = front;
                    popup_progressbar.IsOpen = true;
                    front.IsEnabled = false;                    
                }
            ));
            status = status_t.starting;
        }
        public unsafe void on_stop()
        {
            status = status_t.stopped;
        }
        public unsafe void on_release()
        {
            status = status_t.released;
        }
        public int get_status()
        {
            return status;
        }
    }
}
