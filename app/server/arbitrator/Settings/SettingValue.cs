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
        public int portnumber { get; set; }
        public int video_codec { get; set; }
        public int video_width { get; set; }
        public int video_height { get; set; }
        public int video_fps { get; set; }
        public int video_block_width { get; set; }
        public int video_block_height { get; set; }
        public int video_compression_level { get; set; }
        public int video_quantization_colors { get; set; }
        public bool enable_tls { get; set; }
        public bool enable_keepalive { get; set; }
        public bool enable_present { get; set; }
        public bool enable_auto_start { get; set; }
        public bool enable_caching { get; set; }
        public string cpu { get; set; }
        public string memory { get; set; }

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

            sirius_arbitrator.controller.update(
                (sbyte*)puuid, 
                (sbyte*)purl, 
                max_attendant_instance, 
                attendant_creation_delay, 
                portnumber, 
                video_codec, 
                video_width, 
                video_height, 
                video_fps, 
                video_block_width, 
                video_block_height, 
                video_compression_level, 
                video_quantization_colors, 
                enable_tls, 
                enable_keepalive, 
                enable_present, 
                enable_auto_start, 
                enable_caching);
        }
    }
}
