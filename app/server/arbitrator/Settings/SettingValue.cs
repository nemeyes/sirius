using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace sirius.app.server.arbitrator.Settings
{
    class SettingValue
    {
        public string uuid { get; set; }
        public string url { get; set; }
        public int max_attendant_instance { get; set; }
        public int attendant_creation_delay { get; set; }
        public int min_attendant_restart_threshold { get; set; }
        public int max_attendant_restart_threshold { get; set; }
        public int controller_portnumber { get; set; }
        public int streamer_portnumber { get; set; }
        public int video_codec { get; set; }
        public int video_width { get; set; }
        public int video_height { get; set; }
        public int video_fps { get; set; }
        public int video_buffer_count { get; set; }
        public int video_block_width { get; set; }
        public int video_block_height { get; set; }
        public int video_compression_level { get; set; }
        public bool video_quantization_posterization { get; set; }
        public bool video_quantization_dither_map { get; set; }
        public bool video_quantization_contrast_maps { get; set; }
        public int video_quantization_colors { get; set; }
        public float video_webp_quality { get; set; }
        public int video_webp_method { get; set; }
        public bool invalidate4client { get; set; }
        public bool indexed_mode { get; set; }
        public int nthread { get; set; }
        public bool double_reloading_on_creating { get; set; }
        public bool reloading_on_disconnecting { get; set; }
        public bool enable_tls { get; set; }
        public bool enable_keepalive { get; set; }
        public int keepalive_timeout { get; set; }
        public bool enable_streamer_keepalive { get; set; }
        public int streamer_keepalive_timeout { get; set; }
        public bool enable_present { get; set; }
        public bool enable_auto_start { get; set; }
        public bool enable_caching { get; set; }
        public bool clean_attendant { get; set; }
        public string cpu { get; set; }
        public string memory { get; set; }
        public int log_level { get; set; }
        public int idle_time { get; set; }
        public string log_root_path { get; set; }
        public string app_session_app { get; set; }
        public string caching_directory { get; set; }
        public int caching_expire_time { get; set; }


        private static SettingValue _instance;
        protected SettingValue()
        {
        }
        public static SettingValue Instance()
        {
            if (_instance == null)
            {
                _instance = new SettingValue();
            }
            return _instance;
        }
         unsafe public void update()
        {
            IntPtr puuid = Marshal.StringToHGlobalAnsi(uuid);
            IntPtr purl = Marshal.StringToHGlobalAnsi(url);
            IntPtr plog_root_path = Marshal.StringToHGlobalAnsi(log_root_path);
            IntPtr papp_sesion_app = Marshal.StringToHGlobalAnsi(app_session_app);
            IntPtr pcaching_directory = Marshal.StringToHGlobalAnsi(caching_directory);

            sirius_arbitrator.controller.update(
                (sbyte*)puuid, 
                (sbyte*)purl, 
                max_attendant_instance, 
                attendant_creation_delay,
                min_attendant_restart_threshold,
                max_attendant_restart_threshold,
                controller_portnumber,
                streamer_portnumber, 
                video_codec, 
                video_width, 
                video_height, 
                video_fps,
                video_buffer_count,
                video_block_width, 
                video_block_height, 
                video_compression_level,
                video_quantization_posterization,
                video_quantization_dither_map,
                video_quantization_contrast_maps,
                video_quantization_colors,
                video_webp_quality,
                video_webp_method,
                invalidate4client,
                indexed_mode,
                nthread,
                double_reloading_on_creating,
                reloading_on_disconnecting,                
                enable_tls, 
                enable_keepalive,
                keepalive_timeout,
                enable_streamer_keepalive,
                streamer_keepalive_timeout,
                enable_present, 
                enable_auto_start, 
                enable_caching,
                clean_attendant,
                (sbyte*)papp_sesion_app,
                (sbyte*)pcaching_directory,
                caching_expire_time
                );
        }
    }
}
